#ifdef MISS_SHADER
	layout(location = 0) rayPayloadInEXT RayPayload payload;

	void main() {
		payload.colorAndDist = vec4(0.412f, 0.796f, 1.0f, -1);
		payload.normalAndObjId = vec4(0,0,0,0);
		payload.RoughnessMetallic = vec2(1,0);
	}
#endif
