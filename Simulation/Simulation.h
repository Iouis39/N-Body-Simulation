#pragma once
#include "KernelTypes.h"
#include "../Renderer/Renderer.h"
#include "../Application/MetalConfig.h"
#include "random"

// ############ CONFIGURATION ############# //
const uint THREADGROUPS_PER_GRID = 32;
const uint THREADS_PER_THREADGROUP = 256;

#define GRAVITY 6.67E-11


class Simulation {
public:
	Simulation( MTL::Device* pDevice );
	~Simulation();
	
	void init( MTL::CommandQueue* pCommandQueue, MTL::Buffer* _pParticleBuffer );
	void update();
	Particle dispatchParticles();
	
private:
	void fillParticleList();
	void galaxyColl();
	void setup1();
	void buildBuffer();
	MTL::ComputePipelineState* buildComputePipeline( const char* funcName );
	
	void computeBBBoxKernel();
	
	MTL::Device* _pDevice;
	MTL::ComputePipelineState* _pComputePipeline;
	MTL::CommandQueue* _pCommandQueue;
	
	// Kernel Pipelines
	MTL::ComputePipelineState* _pComputeBBBoxPipeline;

	Particle* _pParticle = nullptr;
//	Node* _pNodes = new Node;
	MTL::Buffer* _pParticleBuffer;
	MTL::Buffer* _pNodeBuffer;
};
