#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	RenderBuffer::RenderBuffer(const RenderBufferDesc & desc, const void *pInitData)
		: RenderResource(RENDER_RESOURCE_BUFFER),
		_desc(desc),
		_vertexBufferType(VERTEX_BUFFER_GEOMETRY),
		_bActive(false)
	{

	}

	void RenderBuffer::Release()
	{
		if (_bActive)
			Global::GetRenderEngine()->GetRenderFactory()->ReleaseBufferToPool(std::static_pointer_cast<RenderBuffer>(shared_from_this()));
	}

	//RenderBuffer::RenderBuffer(const RenderBufferDesc & desc, const void *pInitData,
	//	VertexBufferType vertexBufferType, const std::vector<VertexElementDesc> & vertexElementDesc)
	//	: RenderResource(RENDER_RESOURCE_BUFFER), _desc(desc), _vertexBufferType(vertexBufferType), _vertexElementsDesc(vertexElementDesc)
	//{
	//	
	//}
}