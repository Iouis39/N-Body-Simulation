#pragma once
#include "MetalConfig.h"
#include "../Renderer/Renderer.h"
#include "../Simulation/Simulation.h"

class MyMTKViewDelegate : public MTK::ViewDelegate
{
	public:
		MyMTKViewDelegate( MTL::Device* pDevice );
		virtual ~MyMTKViewDelegate() override;
		virtual void drawInMTKView( MTK::View* pView ) override;
		void beginSimulation( MTL::Device* pDevice );

	private:
		MTL::CommandQueue* _pCommandQueue;
		Renderer* _pRenderer;
		Simulation* _pSimulation;
		
		MTL::Buffer* _pParticleBuffer;
};

