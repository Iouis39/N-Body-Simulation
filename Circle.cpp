#include "Circle.h"


Circle Primitives::buildCircle(MTL::Device* pDevice) {
	Circle Circle;
	
	Vertex vertices[] {
		{{ -1.7321f,  -1 }, {1.f, 1.f, 1.f}},
		{{  1.7321f,  -1 }, {1.f, 1.f, 1.f}},
		{{      0.f, 2.f }, {1.f, 1.f, 1.f}}
	};
	
	ushort indices[] {
		0, 1, 2
	};
	
	const size_t vertexDataSize = sizeof(vertices);
	const size_t indexDataSize = sizeof(indices);
	
	Circle.vertexBuffer = pDevice->newBuffer(vertexDataSize, MTL::ResourceStorageModeShared);
	Circle.vertexBuffer->setLabel(NS::String::string("Vertex Buffer", NS::UTF8StringEncoding));
	memcpy(Circle.vertexBuffer->contents(), vertices, vertexDataSize);
	
	Circle.indexBuffer = pDevice->newBuffer(indexDataSize, MTL::ResourceStorageModeShared);
	Circle.indexBuffer->setLabel(NS::String::string("Index Buffer", NS::UTF8StringEncoding));
	memcpy(Circle.indexBuffer->contents(), indices, indexDataSize);
	
	return Circle;
}

