#pragma once
#include "../Application/MetalConfig.h"
#include "../Simulation/Simulation.h"
#include "Circle.h"
#include "MathUtilities.h"

// ############ VARIABLES ############## //
const float WINDOW_WIDTH = 1280;
const float WINDOW_HEIGHT = 720;
const float ASPECT_RATIO = 16.f / 9.f;

class Renderer {
public:
	Renderer( MTL::Device* pDevice );
	~Renderer();
	void draw( MTK::View* pView );
	void init( MTL::CommandQueue* pCommandQueue, MTL::Buffer* pParticleBuffer );
	
private:
	void buildShaders();
	void buildPrimitives();
	void buildBuffers();
	void buildMatrices();
	void buildInstanceData();
	void updatePosition();
	
	MTL::RenderPipelineState* buildShader(const char* vertName, const char* fragName);
	
	MTL::Device* _pDevice;
	MTL::CommandQueue* _pCommandQueue;
	MTL::RenderPipelineState* _pCirclePipeline;
	
	MTL::Buffer* _pParticleBuffer;
	
	// circle
	Circle Circle;
	
	// matrices
	CameraData _CameraData;
	MTL::Buffer* _pCameraBuffer;
	
	// instancing
	MTL::Buffer* _pInstanceBuffer;
	InstanceData _InstanceData[ PARTICLE_COUNT ];
};

