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

    void main() {
        vec3 camPos = rayParams.camPos.xyz;
        vec3 camDir = rayParams.camDir.xyz;
        vec3 camUp = rayParams.camUp.xyz;
        vec3 camSide = rayParams.camSide.xyz;

        const float fov = 45;
        const float aspect = float(gl_LaunchSizeEXT.x) / float(gl_LaunchSizeEXT.y);

        const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

        vec3 u = camSide.xyz;
        vec3 v = camUp.xyz;

        const float planeWidth = tan(fov * 0.5f);
        u *= (planeWidth * aspect);
        v *= planeWidth;

        const vec3 rayDir = normalize(camDir.xyz + (u * (uv.x*2-1)) - (v * (uv.y*2-1)));

        const vec3 origin = camPos;
        const vec3 direction = rayDir;

        const uint cullMask = 0xFF;
        const uint sbtRecordOffset = 0;
        const uint sbtRecordStride = 0;
        const uint missIndex = 0;
        const float tmin = 0.0f;
        const float tmax = 5000.0f;
        const int payloadLocation = 0;

        traceRayEXT(acc,
            gl_RayFlagsOpaqueEXT,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            origin,
            tmin,
            direction,
            tmax,
            payloadLocation);

        vec3 hitColor = payload.colorAndDist.xyz;
        float hitDistance = payload.colorAndDist.w;
        vec3 normal = payload.normalAndObjId.xyz;
        float objId = payload.normalAndObjId.w;
        vec3 shadowOrigin = origin + direction * hitDistance;

        float lighting = 1;
        float samples = 0;
        float ranges = 0.006;
        vec3 lightDir = rayParams.lightDir.xyz;
        float ndotl = max(dot(lightDir, normal), 0);

        // if (ndotl > 0)
        // {
        //     for (float i = -ranges; i <=ranges; i += 0.01)
        //     {
        //         for (float j = -ranges; j <= ranges; j += 0.01)
        //         {
        //             vec2 rnd = fract(uv.xy) + rayParams.frameNumber % 100;
        //             // vec2 shadowDirSpherical = vec2(1,0.5); // theta, phi
        //             // shadowDirSpherical += vec2(rand(uv.x+uv.y-i*20), rand(uv.x-uv.y+j*20))/30.0;            

        //             vec3 shadowDir = normalize(vec3(
        //                 lightDir.x + rand(rnd.x+rnd.y-i*20) * rayParams.lightSpread,
        //                 lightDir.y + rand(rnd.x+rnd.y+j*20) * rayParams.lightSpread,
        //                 lightDir.z + rand(rnd.x-rnd.y-i*20) * rayParams.lightSpread
        //             ));

        //             // vec3 shadowDir = normalize(vec3(
        //             //     sin(shadowDirSpherical.y) * cos(shadowDirSpherical.x),
        //             //     sin(shadowDirSpherical.y) * sin(shadowDirSpherical.x),
        //             //     cos(shadowDirSpherical.y)
        //             // ));

        //             traceRayEXT(acc,
        //                 gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
        //                 cullMask,
        //                 sbtRecordOffset,
        //                 sbtRecordStride,
        //                 missIndex,
        //                 shadowOrigin,
        //                 0.02,
        //                 shadowDir,
        //                 tmax,
        //             1);

        //             lighting += shadowPayload.colorAndDist.w != -1 ? 1 : 0;
        //             samples += 1;
        //         }
        //     }

        //     lighting /= samples;
        // }

        if (hitDistance > 0)
        {
            traceRayEXT(acc,
                gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
                cullMask,
                sbtRecordOffset,
                sbtRecordStride,
                missIndex,
                shadowOrigin,
                0.02,
                lightDir,
                tmax,
                1
            );

            if (shadowPayload.colorAndDist.w > 0) {
                lighting = 0.5;
            } else {
                lighting = max(0.5, ndotl);
            }
        }

        if (objId == 123)
        {
            vec3 reflectedDir = reflect(direction, normal);
            traceRayEXT(acc,
                gl_RayFlagsOpaqueEXT,
                cullMask,
                sbtRecordOffset,
                sbtRecordStride,
                missIndex,
                shadowOrigin,
                0.02,
                reflectedDir,
                tmax,
                0);
            hitColor = payload.colorAndDist.xyz;
            lighting = 1;
        }
        

        // lighting = 1 - lighting;
        hitColor *= lighting;

        imageStore(img, ivec2(gl_LaunchIDEXT), vec4(hitColor, 1));
        // imageStore(img, ivec2(gl_LaunchIDEXT), vec4(vec3(rand(uv.x+uv.y+rayParams.frameNumber % 50)),1));
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

    layout(binding = 0, set = 4) uniform sampler2D Textures[100];

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

        vec3 texel = textureLod(Textures[nonuniformEXT(texId)], uv, 0.0f).rgb;

        payload.colorAndDist = vec4(texel, gl_HitTEXT);
        payload.normalAndObjId = vec4(normal, gl_InstanceCustomIndexEXT);
    }
#endif
