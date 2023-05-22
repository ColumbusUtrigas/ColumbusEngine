#version 460 core
#extension GL_EXT_ray_tracing : enable

struct RayPayload {
    vec4 colorAndDist;
    vec4 normalAndObjId;
};

#ifdef RAYGEN_SHADER
    layout(location = 0) rayPayloadEXT RayPayload payload;
    layout(location = 1) rayPayloadEXT RayPayload shadowPayload;

    layout(binding = 0, set = 0) uniform accelerationStructureEXT acc;
    layout(binding = 1, rgba32f) uniform image2D img;

    layout(push_constant) uniform params
    {
        vec4 camPos;
        vec4 camDir;
        vec4 camUp;
        vec4 camSide;
        vec4 lightDir;
        float lightSpread;
        int frameNumber;
        int reset;
    } rayParams;

    // A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
    uint hash( uint x ) {
        x += ( x << 10u );
        x ^= ( x >>  6u );
        x += ( x <<  3u );
        x ^= ( x >> 11u );
        x += ( x << 15u );
        return x;
    }

    // Construct a float with half-open range [0:1] using low 23 bits.
    // All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
    float floatConstruct( uint m ) {
        const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
        const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

        m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
        m |= ieeeOne;                          // Add fractional part to 1.0

        float  f = uintBitsToFloat( m );       // Range [1:2]
        return f - 1.0;                        // Range [0:1]
    }


    // Pseudo-random value in half-open range [0:1].
    float rand(float x) { return floatConstruct(hash(floatBitsToUint(x))); }

    // Steps the RNG and returns a floating-point value between 0 and 1 inclusive.
    float stepAndOutputRNGFloat(inout uint rngState)
    {
        // Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
        rngState  = rngState * 747796405 + 1;
        uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
        word      = (word >> 22) ^ word;
        return float(word) / 4294967295.0f;
    }

    vec3 calculateLight(inout vec3 origin, vec3 direction, out vec3 normal, out vec3 surfaceColor, out int hitSurface)
    {
        traceRayEXT(acc,
            gl_RayFlagsOpaqueEXT,
            0xFF,
            0,
            0,
            0,
            origin,
            0,
            direction,
            5000,
            0);

        // if we hit a surface
        if (payload.colorAndDist.w > 0)
        {
            hitSurface = 1;

            surfaceColor = payload.colorAndDist.rgb;
            vec3 hitPoint = payload.colorAndDist.w * direction + origin;
            normal = payload.normalAndObjId.xyz;
            origin = hitPoint + normal * 0.001;
            direction = rayParams.lightDir.xyz;

            // sample shadow
            traceRayEXT(acc,
                gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
                0xFF,
                0,
                0,
                0,
                origin,
                0,
                direction,
                5000,
                1);

            // float ambientColor = 0.5;
            float ambientColor = 0.0; // if there is no light, it should be black!

            // something is occluding the shadow ray
            if (shadowPayload.colorAndDist.w > 0)
            {
                return surfaceColor * ambientColor;
            }
            else
            {
                return surfaceColor * max(dot(normal, direction), ambientColor); // ndotl
            }
        }
        else // sky
        {
            hitSurface = 0;
            return payload.colorAndDist.rgb;
        }
    }

    void main() {
        vec3 camPos = rayParams.camPos.xyz;
        vec3 camDir = rayParams.camDir.xyz;
        vec3 camUp = rayParams.camUp.xyz;
        vec3 camSide = rayParams.camSide.xyz;

        const float fov = 45;
        const float aspect = float(gl_LaunchSizeEXT.x) / float(gl_LaunchSizeEXT.y);

        const uvec2 pixel = gl_LaunchIDEXT.xy;
        const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

        uint rngState = gl_LaunchSizeEXT.x * pixel.y + pixel.x * (rayParams.frameNumber + 1);  // Initial seed

        vec3 u = camSide.xyz;
        vec3 v = camUp.xyz;

        const float planeWidth = tan(fov * 0.5f);
        u *= (planeWidth * aspect);
        v *= planeWidth;

        const vec3 rayDir = normalize(camDir.xyz + (u * (uv.x*2-1)) - (v * (uv.y*2-1)));

        // The sum of the colors of all of the samples.
        vec3 summedPixelColor = vec3(1.0);

        vec3 origin = camPos;
        vec3 direction = rayDir;

        const uint cullMask = 0xFF;
        const uint sbtRecordOffset = 0;
        const uint sbtRecordStride = 0;
        const uint missIndex = 0;
        const float tmin = 0.0f;
        const float tmax = 5000.0f;
        const int payloadLocation = 0;

        const int NUM_SAMPLES = 1;

        // Define the field of view by the vertical slope of the topmost rays:
        const float fovVerticalSlope = 1.0 / 5.0;

        int hitSurface;
        vec3 normal = vec3(0);
        vec3 surfaceColor = vec3(0);
        vec3 finalColor = calculateLight(origin, direction, normal, surfaceColor, hitSurface);
        vec3 indirectColor = vec3(0);

        if (hitSurface == 1)
        {
            for (int sampleIdx = 0; sampleIdx < NUM_SAMPLES; sampleIdx++)
            {
                // For a random diffuse bounce direction, we follow the approach of
                // Ray Tracing in One Weekend, and generate a random point on a sphere
                // of radius 1 centered at the normal. This uses the random_unit_vector
                // function from chapter 8.5:
                float theta = 6.2831853 * stepAndOutputRNGFloat(rngState);  // Random in [0, 2pi]
                float u     = 2.0 * stepAndOutputRNGFloat(rngState) - 1.0;  // Random in [-1, 1]
                float r     = sqrt(1.0 - u * u);
                direction      = normal + vec3(r * cos(theta), r * sin(theta), u);
                direction = normalize(direction);

                vec3 normalBounce1 = vec3(0);
                vec3 colorBounce1 = vec3(0);
                vec3 accumulatedIndirect = calculateLight(origin, direction, normalBounce1, colorBounce1, hitSurface);

                // second bounce
                for (int sampleIdy = 0; sampleIdy < NUM_SAMPLES; sampleIdy++)
                {
                    theta = 6.2831853 * stepAndOutputRNGFloat(rngState);  // Random in [0, 2pi]
                    u     = 2.0 * stepAndOutputRNGFloat(rngState) - 1.0;  // Random in [-1, 1]
                    r     = sqrt(1.0 - u * u);
                    direction      = normalBounce1 + vec3(r * cos(theta), r * sin(theta), u);
                    direction = normalize(direction);

                    vec3 normalBounce2 = vec3(0);
                    vec3 colorBounce2 = vec3(0);
                    accumulatedIndirect += calculateLight(origin, direction, normalBounce2, colorBounce2, hitSurface);
                }

                indirectColor += accumulatedIndirect / float(NUM_SAMPLES);
            }
        }

        finalColor += indirectColor / float(NUM_SAMPLES) * surfaceColor;

        if (rayParams.reset == 1)
        {
            imageStore(img, ivec2(gl_LaunchIDEXT), vec4(0,0,0,1));
        }

        if (rayParams.frameNumber > 1)
        {
            vec4 prevValue = imageLoad(img, ivec2(gl_LaunchIDEXT));
            if (prevValue.rgb != vec3(0))
            {
                finalColor += prevValue.rgb;
            }
        }

        imageStore(img, ivec2(gl_LaunchIDEXT), vec4(finalColor, 1));
    }
#endif

#ifdef MISS_SHADER
    layout(location = 0) rayPayloadInEXT RayPayload payload;

    void main() {
        payload.colorAndDist = vec4(0.412f, 0.796f, 1.0f, -1);
        payload.normalAndObjId = vec4(0,0,0,0);
    }
#endif

#ifdef CLOSEST_HIT_SHADER
    #extension GL_EXT_nonuniform_qualifier : require

    layout(location = 0) rayPayloadInEXT RayPayload payload;

    vec2 BaryLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
        return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
    }

    vec3 BaryLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
        return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
    }

    layout(binding = 0, set = 1) readonly buffer IndexBuffer {
        uint indices[];
    } IndexBuffers[1000];

    layout(binding = 0, set = 2) readonly buffer UvsBuffer {
        vec2 uvs[];
    } UvsBuffers[1000];

    layout(binding = 0, set = 3) readonly buffer NormalsBuffer {
        float normals[];
    } NormalsBuffers[1000];

    layout(binding = 0, set = 4) uniform sampler2D Textures[1000];

    layout(binding = 0, set = 5) readonly buffer MaterialsBuffer {
        int id;
    } MaterialsBuffers[1000];

    #define NORMALBUF NormalsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].normals

    hitAttributeEXT vec2 HitAttribs;

    void main() {
        vec3 barycentrics = vec3(1.0f - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);

        uint index0 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 0];
        uint index1 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 1];
        uint index2 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 2];

        vec2 uv0 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index0];
        vec2 uv1 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index1];
        vec2 uv2 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index2];
        vec2 uv = BaryLerp(uv0, uv1, uv2, barycentrics);

        vec3 normal0 = vec3(NORMALBUF[index0*3+0], NORMALBUF[index0*3+1], NORMALBUF[index0*3+2]);
        vec3 normal1 = vec3(NORMALBUF[index1*3+0], NORMALBUF[index1*3+1], NORMALBUF[index1*3+2]);
        vec3 normal2 = vec3(NORMALBUF[index2*3+0], NORMALBUF[index2*3+1], NORMALBUF[index2*3+2]);
        vec3 normal = BaryLerp(normal0, normal1, normal2, barycentrics);

        int texId = MaterialsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].id;
        vec3 texel = vec3(1);

        if (texId > -1)
        {
            texel = textureLod(Textures[nonuniformEXT(texId)], uv, 0.0f).rgb;
        }

        payload.colorAndDist = vec4(texel, gl_HitTEXT);
        payload.normalAndObjId = vec4(normal, gl_InstanceCustomIndexEXT);
    }
#endif
