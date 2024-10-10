#include <metal_stdlib>
#include "../Simulation/KernelTypes.h"
using namespace metal;

struct VertexInput {
	float2 position [[ attribute(0) ]];
	float3 color 	[[ attribute(1) ]];
};

struct VertexOutput {
	float4 position [[ position ]];
	half3 color;
	float2 localSpace;
	float2 velocity;
};

struct CameraData {
	float4x4 orthographicProjection;
	float4x4 lookAt;
};

struct InstanceData {
	float4x4 modelTransform;
	float3 color;
};

VertexOutput vertex vertexShader( VertexInput input 		  [[ stage_in ]],
					 device const CameraData& CameraData 	  [[ buffer(1) ]],
					 device const InstanceData* InstanceData  [[ buffer(2) ]],
					 device const Particle* ParticleData	  [[ buffer(3) ]],
								  uint instanceId 			  [[ instance_id ]] ) {
	
	VertexOutput output;
	// calculate initial position + updatet position
	half2 pos = half2(input.position + ParticleData[ instanceId ].position.xy);
	output.position = float4(half4x4(InstanceData[ instanceId ].modelTransform) * half4(pos, 0.f, 1.f));
	output.position = 
		float4(half4x4(CameraData.orthographicProjection) * half4x4(CameraData.lookAt) * half4(output.position));
	output.color = half3(InstanceData[ instanceId ].color);
	output.localSpace = input.position;
	output.velocity = ParticleData[ instanceId ].velocity.xy;
	
	return output;
}



half4 fragment fragmentShader( VertexOutput frag [[ stage_in ]] ) {
	float distSquared = dot(frag.localSpace, frag.localSpace);
	
	if(distSquared > 1) {
		discard_fragment();
	}
	
	// Apply a smoothstep to create a soft edge glow effect (adjust thresholds for glow)
//	float glowFactor = smoothstep(0.0, 1.0, 0.8 - distSquared);
//	float glowFactor = pow(smoothstep(0.0, 1.0, 1.0 - distSquared), 2.f);  // Exponent controls glow sharpness
	
	// Base color for the star (could be white for stars)
//	half3 baseColor = half3(clamp(frag.velocity.x, 0.f, 1.f),clamp(frag.velocity.y, 0.f, 1.f), 0.1);  // White color
//	half3 baseColor = half3( 1.f, 1.f, 1.f);
	
	// Optionally add some velocity-based color variation (for dynamic star colors)
//	half3 velocityColor = half3(smoothstep(-10.f, frag.velocity.x, frag.velocity.y));
	
	// Combine base color and velocity color, and modulate by glow factor
//	half3 finalColor = baseColor * glowFactor;
	
	// Return the final color with glow and full alpha
	half length = metal::length(frag.velocity);
	half normalizedVelocity = clamp(length / 30.f, 0.f, 1.f);
	half3 slowColor = { 0.f, 0.f, 1.f };
	half3 fastColor = { 1.f, 0.f, 0.f };
	half3 color = mix(slowColor, fastColor, normalizedVelocity);
	return half4(color * frag.color, 1.f);
}
