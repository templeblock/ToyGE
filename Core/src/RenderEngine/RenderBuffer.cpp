#include "ToyGE\RenderEngine\RenderBuffer.h"

namespace ToyGE
{
	RenderBuffer::RenderBuffer(const RenderBufferDesc & desc, const void *pInitData)
		: RenderResource(RENDER_RESOURCE_BUFFER),
		_desc(desc),
		_vertexBufferType(VERTEX_BUFFER_GEOMETRY)
	{

	}

	//RenderBuffer::RenderBuffer(const RenderBufferDesc & desc, const void *pInitData,
	//	VertexBufferType vertexBufferType, const std::vector<VertexElementDesc> & vertexElementDesc)
	//	: RenderResource(RENDER_RESOURCE_BUFFER), _desc(desc), _vertexBufferType(vertexBufferType), _vertexElementsDesc(vertexElementDesc)
	//{
	//	
	//}
}