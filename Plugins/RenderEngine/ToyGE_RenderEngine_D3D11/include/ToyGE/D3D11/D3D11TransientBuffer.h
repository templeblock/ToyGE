#pragma once
#ifndef D3D11TRANSIENTBUFFER_H
#define D3D11TRANSIENTBUFFER_H

#include "ToyGE\D3D11\D3D11PreInclude.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"

namespace ToyGE
{
	class D3D11RetiredFrame : public RetiredFrame
	{
	public:
		bool IsFrameFinished() override;

	protected:
		Ptr<ID3D11Query> _query;
	};


	class D3D11TransientBuffer : public TransientBuffer
	{
	public:

	protected:
		RetiredFramePtr CreateRetiredFrame() const override;
	};
}

#endif