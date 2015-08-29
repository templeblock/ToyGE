#include "ToyGE\D3D11\D3D11TransientBuffer.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	bool D3D11RetiredFrame::IsFrameFinished()
	{
		auto d3dRE = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		if (!_query)
		{
			D3D11_QUERY_DESC queryDesc;
			queryDesc.Query = D3D11_QUERY_EVENT;
			queryDesc.MiscFlags = 0;

			ID3D11Query * query;
			d3dRE->RawD3DDevice()->CreateQuery(&queryDesc, &query);
			/*if (!query)
			{
				return false;
			}*/

			d3dRE->RawD3DDeviceContex()->End(query);

			_query = MakeComShared(query);
		}

		BOOL bFrameFinished = FALSE;
		d3dRE->RawD3DDeviceContex()->GetData(_query.get(), &bFrameFinished, sizeof(bFrameFinished), 0);

		if (bFrameFinished == TRUE)
			return true;
		else
			return false;
	}


	D3D11TransientBuffer::D3D11TransientBuffer(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags)
		: TransientBuffer(elementSize, initNumElements, bufferBindFlags)
	{

	}

	RetiredFramePtr D3D11TransientBuffer::CreateRetiredFrame() const
	{
		return std::make_shared<D3D11RetiredFrame>();
	}
}