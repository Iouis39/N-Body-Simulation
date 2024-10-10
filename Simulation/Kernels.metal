#include <metal_stdlib>
#include "KernelTypes.h"
using namespace metal;

//#define EPS2 0.5f
#define EPS2 1.2f

static void LoadLocalFromGLobal( thread Particle& value,
								 device Particle* ParticleInput,
								 const  ushort localID ) {
	
	value = ParticleInput[ localID ];
	threadgroup_barrier(mem_flags::mem_threadgroup);
}

//// radius 10.f ---> 1.f in shader
//static void checkConstraint( thread Particle& value ) {
//	int borderX = 10.f;
//	int borderY = 10.f;
//	
//	if( value.position.y <= -36.f + 1.f + borderY || value.position.y >= 36.f + 1.f - borderY ) {
//		value.velocity.y = - 0.9f * value.velocity.y;
//	} else if( value.position.x <= -64.f + 1.f + borderX || value.position.x >= 64.f + 1.f - borderX ) {
//		value.velocity.x = - 0.9f * value.velocity.x;
//	}
//}

static void integrate( thread Particle& Particle, float2 acc ) {
	float2 GRAVITY( 0.f, 0.f );
	const float DT = 1.f / 60.f;
	
	Particle.velocity.xy += ( acc + GRAVITY) * DT;
	Particle.position.xy += Particle.velocity.xy * DT + ( acc + GRAVITY ) * DT * DT;
}


static float2 particleParticleInteraction(Particle p1, Particle p2, thread float2& acceleration) {
	if( p1.position.x == p2.position.x || p1.position.y == p2.position.y) {
		return acceleration;
	} else {
		float2 r = p2.position.xy - p1.position.xy + EPS2;
		float distSqr = distance_squared(p1.position.xy, p2.position.xy) + pow(EPS2, 2);
		
		float invDist = rsqrt(distSqr);
		float invDistCubed = invDist * invDist * invDist;
		
		float s = p2.position.z * invDistCubed;
		acceleration = r * s;
		return acceleration;
	}
}

template<uint BLOCK_SIZE>
float2 tileCalculation( thread Particle& p1, threadgroup Particle* sharedParticle, thread float2& acceleration ) {
	for ( uint i = 0; i < BLOCK_SIZE; i++ ) {
		acceleration += particleParticleInteraction(p1, sharedParticle[i], acceleration);
	}
	return acceleration;
}

kernel void ParticleSimulation( device Particle* ParticleInput 		 [[ buffer(0) ]],
							    threadgroup Particle* sharedParticle [[ threadgroup(0) ]],
								const uint localID 					 [[ thread_index_in_threadgroup ]],
								const uint groupID 					 [[ threadgroup_position_in_grid ]],
							    const uint n 						 [[ threads_per_grid ]],
							    const uint BLOCK_SIZE 				 [[ threads_per_threadgroup ]]) {
	const uint baseID = groupID * BLOCK_SIZE;
	
	float2 acceleration = 0;
	Particle Particle;
	LoadLocalFromGLobal(Particle, &ParticleInput[ baseID ], localID);
	#pragma unroll(2)
	for( uint i = 0, tile = 0; i < n; i += BLOCK_SIZE, tile++ ) {
		uint tileID = tile * BLOCK_SIZE + localID;
		sharedParticle[ localID ] = ParticleInput[ tileID ];
		threadgroup_barrier(mem_flags::mem_threadgroup);
	    #pragma unroll(2)
		for( uint j = 0; j < BLOCK_SIZE; j++ ) {
			acceleration += particleParticleInteraction(Particle, sharedParticle[ j ], acceleration);
		}
		
	}
	
	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	integrate(Particle, acceleration);
	threadgroup_barrier(mem_flags::mem_threadgroup);
	ParticleInput[ baseID + localID ] = Particle;
}


///////////////////////////////////////////////////////////////////////////
//------------ KERNEL 1 : Compute Bounding Box of root node ----------- //
/////////////////////////////////////////////////////////////////////////

template<uint BLOCK_SIZE>
kernel void computeBBBoxKernel( device Particle* Particles [[ buffer(0) ]],
							    device Node* Nodes        [[ buffer(1) ]],
							    constant uint& nParticles,
							    uint localID [[ thread_index_in_threadgroup ]],
								uint groupID [[ threadgroup_position_in_grid ]] ) {
	
	threadgroup float topLeftX[BLOCK_SIZE];
	threadgroup float topLeftY[BLOCK_SIZE];
	threadgroup float botRightX[BLOCK_SIZE];
	threadgroup float botRightY[BLOCK_SIZE];
	
	uint baseID = groupID * BLOCK_SIZE + localID;
	
	topLeftX[ localID ]  =  INFINITY;
	topLeftY[ localID ]  = -INFINITY;
	botRightX[ localID ] = -INFINITY;
	botRightY[ localID]  =  INFINITY;
	
	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	if( baseID < nParticles ) {
		Particle particle = Particles[ baseID ];
		topLeftX[ localID ]  = particle.position.x;
		topLeftY[ localID ]  = particle.position.y;
		botRightX[ localID ] = particle.position.x;
		botRightY[ localID ] = particle.position.y;
	}
	
	// parallel reduction ---> balanced binary tree
	for( uint s = BLOCK_SIZE / 2; s > 0; s >>= 1 ) {
		threadgroup_barrier(mem_flags::mem_threadgroup);
		
		if( localID < s ) {
			topLeftX[ localID ]  = fmin(topLeftX[ localID ], topLeftX[ localID + s ]);
			topLeftY[ localID ]  = fmin(topLeftY[ localID ], topLeftY[ localID + s ]);
			botRightX[ localID ] = fmin(botRightX[ localID ], botRightX[ localID + s ]);
			botRightY[ localID ] = fmin(botRightY[ localID ], botRightY[ localID + s ]);
		}
	}
	
	if( localID == 0 ) {
//		device atomic_float* atomicTopLeftX = reinterpret_cast<device atomic_float*>(Nodes);
//		Nodes[ 0 ].topLeft.x  = atomic_min_explicit(Nodes[ 0 ].topLeft.x, topLeftX[ 0 ], memory_order_relaxed);
//		Nodes[ 0 ].topLeft.y  = atomic_max_explicit(Nodes[ 0 ].topLeft.y, topLeftY[ 0 ], memory_order_relaxed);
//		Nodes[ 0 ].botRight.x = atomic_min_explicit(Nodes[ 0 ].botRight.x, botRightX[ 0 ], memory_order_relaxed);
//		Nodes[ 0 ].botRight.y = atomic_max_explicit(Nodes[ 0 ].botRight.y, botRightY[ 0 ], memory_order_relaxed);
	}
}

template [[host_name("computeBBBoxKernel")]] kernel void computeBBBoxKernel<512>
	( device Particle*, device Node*, constant uint&, const uint, const uint);
