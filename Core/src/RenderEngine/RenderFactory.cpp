#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "boost\functional\hash.hpp"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	RenderFactory::RenderFactory()
	{

	}

	Ptr<Texture> RenderFactory::CreateTexture(const Ptr<Image> & image, uint32_t bindFlag, uint32_t cpuAccess)
	{
		if (!image)
			return Ptr<Texture>();

		TextureDesc desc;
		desc.type = image->GetType();
		desc.format = image->GetFormat();
		desc.width = image->GetWidth();
		desc.height = image->GetHeight();
		desc.depth = image->GetDepth();
		desc.arraySize = image->GetArraySize();
		desc.mipLevels = image->GetMipLevels();
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		desc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_IMMUTABLE;
		desc.cpuAccess = 0;

		return CreateTexture(desc, image->DataDescs());
	}

	Ptr<Texture> RenderFactory::CreateTextureFromFile(const Ptr<File> & file, uint32_t bindFlag, uint32_t cpuAccess)
	{
		if (!file)
			return Ptr<Texture>();

		Ptr<Image> image = Image::Load(file);
		return CreateTexture(image, bindFlag, cpuAccess);
	}

	Ptr<Texture> RenderFactory::CreateTextureFromFile(const WString & filePath, uint32_t bindFlag, uint32_t cpuAccess)
	{
		Ptr<File> file = std::make_shared<File>(filePath, FILE_OPEN_READ);
		return CreateTextureFromFile(file, bindFlag, cpuAccess);
	}

	//Ptr<Texture> RenderFactory::CreateTextureAutoGenMips(const Ptr<Image> & image, uint32_t bindFlag, uint32_t cpuAccess)
	//{
	//	Ptr<Texture> src = CreateTexture(image, bindFlag, cpuAccess);
	//	return src->CreateMips();
	//}

	Ptr<BlendState> RenderFactory::GetBlendStatePooled(const BlendStateDesc & desc)
	{
		auto key = Hash(&desc, sizeof(desc));
		auto & bss = _blendStatePool[key];
		if (!bss)
			bss = CreateBlendState(desc);
		return bss;
	}

	Ptr<DepthStencilState> RenderFactory::GetDepthStencilStatePooled(const DepthStencilStateDesc & desc)
	{
		auto key = Hash(&desc, sizeof(desc));
		auto & dss = _depthStendilStatePool[key];
		if (!dss)
			dss = CreateDepthStencilState(desc);
		return dss;
	}

	Ptr<RasterizerState> RenderFactory::GetRasterizerStatePooled(const RasterizerStateDesc & desc)
	{
		auto key = Hash(&desc, sizeof(desc));
		auto & rs = _rasterizerStatePool[key];
		if (!rs)
			rs = CreateRasterizerState(desc);
		return rs;
	}

	Ptr<Sampler> RenderFactory::GetSamplerPooled(const SamplerDesc & desc)
	{
		auto key = Hash(&desc, sizeof(desc));
		auto & sampler = _samplersPool[key];
		if (!sampler)
			sampler = CreateSampler(desc);
		return sampler;
	}

	Ptr<Texture> RenderFactory::GetTexturePooled(const TextureDesc & desc)
	{
		auto texDesc = desc;
		if (texDesc.mipLevels == 0)
			texDesc.mipLevels = static_cast<int32_t>(log2(std::max<int32_t>(texDesc.width, texDesc.height))) + 1;
		else
			texDesc.mipLevels = std::min<int32_t>(texDesc.mipLevels, static_cast<int32_t>(log2(std::max<int32_t>(texDesc.width, texDesc.height))) + 1);
		//std::vector<std::tuple<int32_t, int32_t, int32_t>>
		//texDesc.mipLevels = Texture::ComputeMipLevels(desc.mipLevels, desc.width, desc.height, desc.depth, )

		auto key = Hash(&texDesc, sizeof(texDesc));
		auto texIdleFind = _texturePoolIdle.find(key);
		if (texIdleFind == _texturePoolIdle.end() || texIdleFind->second.size() == 0)
		{
			auto tex = CreateTexture(desc);
			return tex;
		}
		else
		{
			auto tex = texIdleFind->second.back();
			texIdleFind->second.pop_back();
			tex->_bActive = true;
			return tex;
		}
	}

	void RenderFactory::ReleaseTextureToPool(Ptr<Texture> & tex)
	{
		uint64_t key = Hash(&tex->Desc(), sizeof(tex->Desc()));
		_texturePoolIdle[key].push_back(tex);

		tex->_bActive = false;
	}

	void RenderFactory::ClearTexturePool()
	{
		_texturePoolIdle.clear();
	}
}