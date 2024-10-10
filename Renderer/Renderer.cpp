#include "Renderer.h"

Renderer::Renderer( MTL::Device* pDevice )
: _pDevice( pDevice->retain()) {
	buildShaders();
	buildMatrices();
	buildInstanceData();
}

void Renderer::init( MTL::CommandQueue* pCommandQueue, MTL::Buffer* pParticleBuffer ) {
	_pCommandQueue = pCommandQueue;
	_pParticleBuffer = pParticleBuffer;
	buildBuffers();
}

Renderer::~Renderer() {
	_pCirclePipeline->release();
	_pCommandQueue->release();
	_pDevice->release();
}

void Renderer::buildBuffers() {
	// circle data
	Circle = Primitives::buildCircle(_pDevice);
	// camera data
	_pCameraBuffer = _pDevice->newBuffer(sizeof(_CameraData), MTL::StorageModeShared);
	_pCameraBuffer->setLabel(NS::String::string("Camera Buffer", NS::UTF8StringEncoding));
	memcpy(_pCameraBuffer->contents(), &_CameraData, sizeof(_CameraData));
	// instance data
	_pInstanceBuffer = _pDevice->newBuffer(sizeof(InstanceData) * PARTICLE_COUNT, MTL::StorageModeShared);
	_pInstanceBuffer->setLabel(NS::String::string("Instance Buffer", NS::UTF8StringEncoding));
	memcpy(_pInstanceBuffer->contents(), &_InstanceData, sizeof(_InstanceData));
}

void Renderer::buildShaders() {
	_pCirclePipeline = buildShader("vertexShader", "fragmentShader");
}

MTL::RenderPipelineState* Renderer::buildShader(const char* vertName, const char* fragName) {

	NS::Error* error = nullptr;
	MTL::Library* library = _pDevice->newDefaultLibrary();
	
	if(!library) {
		std::cout << error->localizedDescription()->utf8String() << std::endl;
	}
	
	NS::String* vertexName = NS::String::string(vertName, NS::StringEncoding::UTF8StringEncoding);
	MTL::Function* vertexMain = library->newFunction(vertexName);
	
	NS::String* fragmentName = NS::String::string(fragName, NS::StringEncoding::UTF8StringEncoding);
	MTL::Function* fragmentMain = library->newFunction(fragmentName);
	
	MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	pipelineDescriptor->setVertexFunction(vertexMain);
	pipelineDescriptor->setFragmentFunction(fragmentMain);
	pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
	pipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
	pipelineDescriptor->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);
	pipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
	
	MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
	auto attributes = vertexDescriptor->attributes();
	// attributes: 0 => position
	auto positionDescriptor = attributes->object(0);
	positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
	positionDescriptor->setOffset(0);
	positionDescriptor->setBufferIndex(0);
	
	// attributes: 1 => color
	auto colorDescriptor = attributes->object(1);
	colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
	colorDescriptor->setOffset(4 * sizeof(float));
	colorDescriptor->setBufferIndex(0);
	
	auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
	layoutDescriptor->setStride(8 * sizeof(float));
	
	pipelineDescriptor->setDepthAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth16Unorm);
	pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
	
	MTL::RenderPipelineState* pipeline  = _pDevice->newRenderPipelineState(pipelineDescriptor, &error);
	if(!pipeline) std::cout << error->localizedDescription()->utf8String() << std::endl;
	
	vertexMain->release();
	fragmentMain->release();
	pipelineDescriptor->release();
	library->release();
	
	return pipeline;
}


void Renderer::draw( MTK::View* pView )
{
	buildInstanceData();
	NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

	MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
	
	MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
	MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
	
	pEnc->setRenderPipelineState(_pCirclePipeline);
	pEnc->setVertexBuffer(Circle.vertexBuffer, 0, 0);
	pEnc->setVertexBuffer(_pCameraBuffer, 0, 1);
	pEnc->setVertexBuffer(_pInstanceBuffer, 0, 2);
	pEnc->setVertexBuffer(_pParticleBuffer, 0, 3);
	
	pEnc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(3), MTL::IndexTypeUInt16, Circle.indexBuffer, NS::UInteger(0), PARTICLE_COUNT);
	
	pEnc->endEncoding();
	pCmd->presentDrawable( pView->currentDrawable() );
	pCmd->commit();
	
	pPool->release();
}

void Renderer::buildMatrices() {
	const float left = WINDOW_WIDTH / 2;
	const float right = -WINDOW_WIDTH / 2;
	const float bottom = -WINDOW_HEIGHT / 2;
	const float top = WINDOW_HEIGHT / 2;
	const float near =  5000.f;
	const float far =  -5000.f;
	_CameraData.orthographicProjection = matrix_ortho_left_hand(left, right, bottom, top, near, far);
	
	simd::float3 eye =    { 0.f, 0.f, 1.f };
	simd::float3 target = { 0.f, 0.f, 0.f };
	simd::float3 up =	   { 0.f, 1.f, 0.f };
	_CameraData.lookAt = matrix_look_at_left_hand(eye, target, up);
}

void Renderer::buildInstanceData() {
	for(int i = 0; i < PARTICLE_COUNT; i++) {
		// model matrix calculation
		simd::float3 scale = { 1.5f, 1.5f, 1.5f };
//		simd::float3 scale = { 10.f, 10.f, 10.f };
		simd::float4x4 scaleTranslation = matrix4x4_scale(scale);
		_InstanceData[ i ].modelTransform = scaleTranslation;
		if( i == PARTICLE_COUNT - 1 ) {
			_InstanceData[ i ].color = { 0.f, 0.f, 0.f };
		} else {
			_InstanceData[ i ].color = { 1.f, 1.f, 1.f };
		}
	}
}
