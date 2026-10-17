#pragma once

#ifndef LIGHTWEIGHT_SHADOW_HITS
#define LIGHTWEIGHT_SHADOW_HITS 0
#endif

bool IsOpaqueShadowRay()
{
	// Shared opaque visibility rays carry both flags. Surface rays and the
	// alpha-aware closest-hit retry loop must retain full surface shading.
	const uint ShadowFlags = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
	return (RayFlags() & ShadowFlags) == ShadowFlags;
}
