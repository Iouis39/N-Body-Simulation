## N-Body Simulation in C++ and Metal.
This simulation uses the all-pairs approach, which has O(n<sup>2</sup>) time complexity.

## Simulation Kernel
```Metal
kernel void ParticleSimulation( device Particle* ParticleInput 	        [[ buffer(0) ]],
				threadgroup Particle* sharedParticle    [[ threadgroup(0) ]],
				const uint localID 			[[ thread_index_in_threadgroup ]],
				const uint groupID 		 	[[ threadgroup_position_in_grid ]],
				const uint n 			        [[ threads_per_grid ]],
                                const uint BLOCK_SIZE 			[[ threads_per_threadgroup ]]) {
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
```




<img width="991" alt="Bildschirmfoto 2024-10-09 um 18 49 48" src="https://github.com/user-attachments/assets/9648b5e5-c703-4a79-a186-bd8765b53d4a">

<img width="1280" alt="Bildschirmfoto 2024-10-10 um 05 35 55" src="https://github.com/user-attachments/assets/1ee44513-a34a-4e64-9e0b-21e034bc144d">
