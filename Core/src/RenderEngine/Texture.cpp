#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	void Texture::Init(const std::vector<RenderDataDesc> & initDataList)
	{
		RenderResource::Init();

		InitMipsSize();
	}

	void Texture::Release()
	{
		RenderResource::Release();

		for (auto & view : _srvCache)
			view->Release();
		_srvCache.clear();
		for (auto & view : _uavCache)
			view->Release();
		_uavCache.clear();
		for (auto & view : _rtvCache)
			view->Release();
		_rtvCache.clear();
		for (auto & view : _dsvCache)
			view->Release();
		_dsvCache.clear();
	}

	Ptr<Texture> Texture::CreateMips() const
	{
		auto mipsTexDesc = _desc;
		mipsTexDesc.mipLevels = 0;
		mipsTexDesc.bindFlag |= TEXTURE_BIND_GENERATE_MIPS;
		mipsTexDesc.bindFlag &= ~TEXTURE_BIND_IMMUTABLE;
		auto mipsTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(GetType());
		mipsTex->SetDesc(mipsTexDesc);
		mipsTex->Init({});

		int32_t adjustArraySize = _desc.arraySize;
		if(_desc.bCube)
			adjustArraySize *= 6;

		for (uint32_t arrayIndex = 0; arrayIndex != adjustArraySize; ++arrayIndex)
		{
			this->CopyTo(
				mipsTex, //dst
				0, //dstMipLevel
				arrayIndex, //dstArrayIndex
				0, //xOffset
				0, //yOffset
				0, //zOffset
				0, //srcMipLevel
				arrayIndex //srcArrayIndex
				);
		}

		mipsTex->GenerateMips();

		return mipsTex;
	}

	int32_t Texture::GetDataSize() const
	{
		int32_t pixelBits = GetRenderFormatNumBits(_desc.format);
		bool bCompression = IsCompress(_desc.format);

		int32_t dataSize = 0;
		for (int32_t mipLevel = 0; mipLevel < _desc.mipLevels; ++mipLevel)
		{
			auto mipSize = GetMipSize(mipLevel);
			int32_t w = mipSize.x();
			int32_t h = mipSize.y();
			int32_t d = mipSize.z();

			if (bCompression)
			{
				w = (w + 3) / 4 * 4;
				h = (h + 3) / 4 * 4;

				auto pitch = (16 * pixelBits / 8) * w / 4;
				auto slice = pitch * h / 4;
				dataSize += slice * d;
			}
			else
			{
				auto pitch = pixelBits / 8 * w;
				auto slice = pitch * h;
				dataSize += slice * d;
			}
		}
		dataSize *= _desc.bCube ? _desc.arraySize * 6 : _desc.arraySize;

		return dataSize;
	}

	bool Texture::Dump(void * outDumpBuffer, std::vector<RenderDataDesc> & outDataDescs)
	{
		outDataDescs.clear();

		int32_t adjustArraySize = _desc.bCube ? _desc.arraySize * 6 : _desc.arraySize;

		// Dump texture
		auto dumpTexDesc = _desc;
		dumpTexDesc.bindFlag = 0;
		dumpTexDesc.cpuAccess = CPU_ACCESS_READ;
		auto dumpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(_type);
		dumpTex->SetDesc(dumpTexDesc);
		dumpTex->Init();

		for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				if (!CopyTo(dumpTex, mipLevel, arrayIndex, 0, 0, 0, mipLevel, arrayIndex))
					return false;
			}
		}

		// Creat buffer
		int32_t pixelBits = GetRenderFormatNumBits(dumpTexDesc.format);
		bool bCompression = IsCompress(dumpTexDesc.format);

		// Copy from mapped data
		uint8_t * pDst = static_cast<uint8_t*>(outDumpBuffer);

		for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				auto mipSize = dumpTex->GetMipSize(mipLevel);
				int32_t w = mipSize.x();
				int32_t h = mipSize.y();
				int32_t d = mipSize.z();

				int32_t cpyPitch = 0;
				int32_t numCpys = 0;

				if (bCompression)
				{
					w = (w + 3) / 4 * 4;
					h = (h + 3) / 4 * 4;

					cpyPitch = (16 * pixelBits / 8) * (w / 4);
					numCpys = h / 4;
				}
				else
				{
					cpyPitch = pixelBits / 8 * w;
					numCpys = h;
				}

				auto mappedData = dumpTex->Map(MAP_READ, mipLevel, arrayIndex);

				RenderDataDesc dataDesc;
				dataDesc.pData = pDst;
				dataDesc.rowPitch = cpyPitch;
				dataDesc.slicePitch = dataDesc.rowPitch * numCpys;
				outDataDescs.push_back(dataDesc);

				for (int32_t slice = 0; slice < d; ++slice)
				{
					uint8_t * pSrc = static_cast<uint8_t*>(mappedData.pData) + slice * mappedData.slicePitch;
					for (int32_t row = 0; row < numCpys; ++row)
					{
						memcpy(pDst, pSrc, cpyPitch);
						pSrc += mappedData.rowPitch;
						pDst += cpyPitch;
					}
				}

				dumpTex->UnMap();
			}
		}

		return true;
	}

	Ptr<TextureShaderResourceView> Texture::GetShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat)
	{
		if (numMips <= 0)
			numMips = _desc.mipLevels;
		if (numArrays <= 0)
		{
			if (GetType() == TEXTURE_3D)
				numArrays = _desc.depth;
			else
				numArrays = _desc.arraySize;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			viewFormat = _desc.format;

		for (auto & view : _srvCache)
		{
			if (view->firstMip == firstMip &&
				view->numMips == numMips &&
				view->firstArray == firstArray &&
				view->numArrays == numArrays &&
				view->bCube == bCube &&
				view->viewFormat == viewFormat)
				return view;
		}

		auto newView = CreateShaderResourceView(firstMip, numMips, firstArray, numArrays, bCube, viewFormat);
		newView->firstMip = firstMip;
		newView->numMips = numMips;
		newView->firstArray = firstArray;
		newView->numArrays = numArrays;
		newView->bCube = bCube;
		newView->viewFormat = viewFormat;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_srvCache.push_back(newView);

		return newView;
	}

	Ptr<TextureUnorderedAccessView> Texture::GetUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		if (numArrays <= 0)
		{
			if (GetType() == TEXTURE_3D)
				numArrays = _desc.depth;
			else
				numArrays = _desc.arraySize;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			viewFormat = _desc.format;

		for (auto & view : _uavCache)
		{
			if (view->mipLevel == mipLevel &&
				view->firstArray == firstArray &&
				view->numArrays == numArrays &&
				view->viewFormat == viewFormat)
				return view;
		}

		auto newView = CreateUnorderedAccessView(mipLevel, firstArray, numArrays, viewFormat);
		newView->mipLevel = mipLevel;
		newView->firstArray = firstArray;
		newView->numArrays = numArrays;
		newView->viewFormat = viewFormat;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_uavCache.push_back(newView);

		return newView;
	}

	Ptr<TextureRenderTargetView> Texture::GetRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		if (numArrays <= 0)
		{
			if (GetType() == TEXTURE_3D)
				numArrays = _desc.depth;
			else
				numArrays = _desc.arraySize;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			viewFormat = _desc.format;

		for (auto & view : _rtvCache)
		{
			if (view->mipLevel == mipLevel &&
				view->firstArray == firstArray &&
				view->numArrays == numArrays &&
				view->viewFormat == viewFormat)
				return view;
		}

		auto newView = CreateRenderTargetView(mipLevel, firstArray, numArrays, viewFormat);
		newView->mipLevel = mipLevel;
		newView->firstArray = firstArray;
		newView->numArrays = numArrays;
		newView->viewFormat = viewFormat;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_rtvCache.push_back(newView);

		return newView;
	}

	Ptr<TextureDepthStencilView> Texture::GetDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		if (numArrays <= 0)
		{
			if (GetType() == TEXTURE_3D)
				numArrays = _desc.depth;
			else
				numArrays = _desc.arraySize;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			viewFormat = _desc.format;

		for (auto & view : _dsvCache)
		{
			if (view->mipLevel == mipLevel &&
				view->firstArray == firstArray &&
				view->numArrays == numArrays &&
				view->viewFormat == viewFormat)
				return view;
		}

		auto newView = CreateDepthStencilView(mipLevel, firstArray, numArrays, viewFormat);
		newView->mipLevel = mipLevel;
		newView->firstArray = firstArray;
		newView->numArrays = numArrays;
		newView->viewFormat = viewFormat;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_dsvCache.push_back(newView);

		return newView;
	}


	void Texture::InitMipsSize()
	{
		ComputeMipsSize(_desc.width, _desc.height, _desc.depth, _mipsSize);

		if (_desc.mipLevels == 0)
			_desc.mipLevels = (int32_t)_mipsSize.size();

		if (_desc.mipLevels < (int32_t)_mipsSize.size())
			_mipsSize.resize(_desc.mipLevels);
		else if(_desc.mipLevels > (int32_t)_mipsSize.size())
			_desc.mipLevels = (int32_t)_mipsSize.size();
	}

}