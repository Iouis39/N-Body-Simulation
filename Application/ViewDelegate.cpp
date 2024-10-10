#include "ViewDelegate.h"

MyMTKViewDelegate::MyMTKViewDelegate( MTL::Device* pDevice )
: MTK::ViewDelegate()
, _pRenderer( new Renderer( pDevice ))
, _pSimulation( new Simulation( pDevice ))
{
	_pParticleBuffer = pDevice->newBuffer(PARTICLE_COUNT * sizeof(Particle), MTL::ResourceStorageModeShared);
	_pParticleBuffer->setLabel(NS::String::string("Particle Buffer", NS::UTF8StringEncoding));
	
	_pCommandQueue = pDevice->newCommandQueue();
	_pSimulation->init( _pCommandQueue, _pParticleBuffer );
	_pRenderer->init( _pCommandQueue, _pParticleBuffer );

}

MyMTKViewDelegate::~MyMTKViewDelegate()
{
	delete _pRenderer;
	delete _pSimulation;
	_pCommandQueue->release();
	_pParticleBuffer->release();
}

void MyMTKViewDelegate::drawInMTKView( MTK::View* pView )
{
	_pSimulation->update();
	_pRenderer->draw( pView );
}
