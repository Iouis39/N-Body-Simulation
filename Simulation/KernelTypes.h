#pragma once
#include <simd/simd.h>

//#define RADIUS = 1.f;
//#define float MASS = 1.f;

struct Particle {
	simd::float3 position; // third component is mass
	simd::float3 velocity; // third component is radius
};

struct Node {
	simd::float2 topLeft;
	simd::float2 botRight; // bottom right
	simd::float2 centerMass;
	float totalMass;
	bool isLeaf;
	int start;
	int end;
};
