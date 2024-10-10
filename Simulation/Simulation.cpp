#include "Simulation.h"

Simulation::Simulation( MTL::Device* pDevice ) :
_pDevice(pDevice->retain()) {
	_pComputePipeline = buildComputePipeline("ParticleSimulation");
}

Simulation::~Simulation() {
	_pDevice->release();
	_pCommandQueue->release();
	_pComputePipeline->release();
	delete _pParticle;
}


// set command queue from view delegate
void Simulation::init( MTL::CommandQueue* pCommandQueue, MTL::Buffer* pParticleBuffer ) {
	_pCommandQueue = pCommandQueue;
	_pParticleBuffer = pParticleBuffer;
	fillParticleList();
}

void Simulation::fillParticleList() {
	_pParticle = new Particle[PARTICLE_COUNT];

	for(uint i = 0; i < PARTICLE_COUNT - 1; i++) {
		
		float angle = 2 * M_PI * random_float(0.f, 10.f);
		float radius = random_float(200.f, 300.f);
		
		float x = radius * cosf(angle);
		float y = radius * sinf(angle);
		Particle& p = _pParticle[ i ];
		p.position.xy = { x, y };
		p.position.z = 1;
		
		float distance = sqrt(pow(x, 2) + pow(y, 2));
		simd::float2 a = p.position.xy / distance;
		float esc = sqrt((random_float(10.f, 10000.f) * p.position.z) / distance);
		p.velocity.xy = { -a.y * esc, a.x * esc };
	}
	
	Particle &p1 = _pParticle[ PARTICLE_COUNT - 1 ];
	p1.position = { 0.f, 0.f, 5000.f };
	p1.velocity = { 0.f, 0.f, 0.f };
	
//	Particle &p2 = _pParticle[ PARTICLE_COUNT - 2 ];
//	p2.position = { 100.f, -100.f, 4000.f };
//	p2.velocity = { 0.f, 0.f, 0.f };
	
	memcpy(_pParticleBuffer->contents(), _pParticle, PARTICLE_COUNT * sizeof(Particle));
}

void Simulation::galaxyColl() {
	_pParticle = new Particle[PARTICLE_COUNT];
	
	for(uint i = 0; i < PARTICLE_COUNT / 2 ; i++) {
		
		float angle = 2 * M_PI * random_float(0.f, 10.f);
		float radius = random_float(5.f, 50.f);
		
		float x = radius * cosf(angle) - 150.f;
		float y = radius * sinf(angle);
		Particle& p = _pParticle[ i ];
		p.position.xy = { x, y };
		p.position.z = 10;
		
		float distance = sqrt(pow(x - 150.f, 2) + pow(y, 2));
		simd::float2 a = p.position.xy / distance;
		float esc = sqrt((1000 * p.position.z) / distance);
		p.velocity.xy = { -a.y * esc, a.x * esc };
	}
	
	for(uint j = PARTICLE_COUNT / 2; j < PARTICLE_COUNT - 1; j++) {
		
		float angle = 2 * M_PI * random_float(0.f, 10.f);
		float radius = random_float(5.f, 50.f);
		
		float x = radius * cosf(angle) + 150.f;
		float y = radius * sinf(angle);
		Particle& p = _pParticle[ j ];
		p.position.xy = { x, y };
		p.position.z = 10;
		
		float distance = sqrt(pow(x + 150.f, 2) + pow(y, 2));
		simd::float2 a = p.position.xy / distance;
		float esc = sqrt((1000 * p.position.z) / distance);
		p.velocity.xy = { -a.y * esc, a.x * esc };
	}
	
	Particle &p1 = _pParticle[ PARTICLE_COUNT - 1 ];
	p1.position = { 0.f, 0.f, 1000.f };
	p1.velocity = { 0.f, 0.f, 0.f };
	
	memcpy(_pParticleBuffer->contents(), _pParticle, PARTICLE_COUNT * sizeof(Particle));
}



// dispatch kernel
void Simulation::update() {
	NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
	
	MTL::CommandBuffer* pCommandBuffer = _pCommandQueue->commandBuffer();
	assert(pCommandBuffer);
	
	MTL::ComputeCommandEncoder* pComputeEnc = pCommandBuffer->computeCommandEncoder();
	pComputeEnc->setComputePipelineState(_pComputePipeline);
	
	MTL::Size threadgroupsPerGrid(THREADGROUPS_PER_GRID, 1, 1);
	MTL::Size threadsPerThreadgroup(THREADS_PER_THREADGROUP, 1, 1);
	pComputeEnc->setBuffer(_pParticleBuffer, 0, 0);
	pComputeEnc->setThreadgroupMemoryLength(THREADS_PER_THREADGROUP * sizeof(Particle), 0);
	pComputeEnc->dispatchThreadgroups(threadgroupsPerGrid, threadsPerThreadgroup);
	pComputeEnc->endEncoding();
	pCommandBuffer->commit();
	
	pPool->release();

}

MTL::ComputePipelineState* Simulation::buildComputePipeline(const char *funcName) {
	NS::Error* error = nullptr;
	MTL::Library* pComputeLibrary = _pDevice->newDefaultLibrary();
	
	if(!pComputeLibrary) std::cout << error->localizedDescription()->utf8String() << std::endl;
	
	NS::String* functionName = NS::String::string(funcName, NS::StringEncoding::UTF8StringEncoding);
	MTL::Function* pFunction = pComputeLibrary->newFunction(functionName);
	_pComputePipeline = _pDevice->newComputePipelineState(pFunction, &error);
	
	MTL::ComputePipelineDescriptor* pDesc = MTL::ComputePipelineDescriptor::alloc()->init();
	pDesc->setComputeFunction(pFunction);
	
	if(!_pComputePipeline) std::cout << error->localizedDescription()->utf8String() << std::endl;
	
	pFunction->release();
	pComputeLibrary->release();
	
	return _pComputePipeline;
}

void Simulation::computeBBBoxKernel() {
	size_t THREADGROUPS_PER_GRID = 1;
	size_t THREADS_PER_THREADGROUP = 512;
	
	_pComputeBBBoxPipeline = buildComputePipeline("computeBBBoxKernel");
	NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
	
	MTL::CommandBuffer* pCommandBuffer = _pCommandQueue->commandBuffer();
	assert(pCommandBuffer);
	
	MTL::ComputeCommandEncoder* pEncoder = pCommandBuffer->computeCommandEncoder();
	pEncoder->setComputePipelineState(_pComputeBBBoxPipeline);
	
	MTL::Size threadgroupsPerGrid(THREADGROUPS_PER_GRID, 1, 1);
	MTL::Size threadsPerThreadgroup(THREADS_PER_THREADGROUP, 1, 1);
	
	pEncoder->setBuffer(_pParticleBuffer, 0, 0);
	pEncoder->setBuffer(_pNodeBuffer, 0, 1);
	pEncoder->setBytes(&PARTICLE_COUNT, sizeof(const uint), 2);
	pEncoder->dispatchThreadgroups(threadgroupsPerGrid, threadsPerThreadgroup);
	
	pPool->release();
}
