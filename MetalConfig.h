#pragma once

#include <cassert>
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <vector>

#include <iostream>

const uint PARTICLE_COUNT = 4096;

//############# TYPES #############//

struct Circle {
	MTL::Buffer* vertexBuffer, *indexBuffer;
};

struct Vertex {
	simd::float2 pos;
	simd::float3 color;
};

struct CameraData {
	simd::float4x4 orthographicProjection;
	simd::float4x4 lookAt;
};

struct InstanceData {
	simd::float4x4 modelTransform;
	simd::float3 color;
};
