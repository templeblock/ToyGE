#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

#include "rapidxml.hpp"

namespace ToyGE
{
	class BlendState;
	class DepthStencilState;
	class RasterizerState;
	class Shader;
	class RenderTechnique;

	class TOYGE_CORE_API RenderPass
	{
		friend class RenderEffect;

	public:
		static Ptr<RenderPass> Load(const rapidxml::xml_node<> * node, const Ptr<RenderTechnique> & technique);

		const std::weak_ptr<RenderTechnique> & Technique() const
		{
			return _technique;
		}

		void SetName(const String & name)
		{
			_name = name;
		}

		const String & Name() const
		{
			return _name;
		}

		/*void SetShader(const Ptr<Shader> & shader);
		void ResetShader(ShaderType shaderType)
		{
			_shaders[shaderType] = Ptr<Shader>();
		}
		Ptr<Shader> GetShader(ShaderType shaderType) const
		{
			auto find = _shaders.find(shaderType);
			if (find != _shaders.end())
				return find->second;
			else
				return Ptr<Shader>();
		}*/

		//init shader contents from its reflect desc
		//void InitShaderContents();

		void SetBlendState(const Ptr<BlendState> & blendState)
		{
			_blendState = blendState;
		}

		Ptr<BlendState> GetBlendState() const
		{
			return _blendState;
		}

		void SetDepthStencilState(const Ptr<DepthStencilState> & depthStencilState)
		{
			_depthStencilState = depthStencilState;
		}

		Ptr<DepthStencilState> GetDepthStencilState() const
		{
			return _depthStencilState;
		}

		void SetRasterizerState(const Ptr<RasterizerState> & rasterizerState)
		{
			_rasterizerState = rasterizerState;
		}

		Ptr<RasterizerState> GetRasterizerState() const
		{
			return _rasterizerState;
		}

		CLASS_SET(StencilRef, uint32_t, _stencilRef);
		CLASS_GET(StencilRef, uint32_t, _stencilRef);

		void Bind();

		void UnBind();

	private:
		std::weak_ptr<RenderTechnique> _technique;

		String _name;
		//std::vector<MacroDesc> _macros;
		std::map<ShaderType, String> _shadersEntries;
		std::map<ShaderType, Ptr<Shader>> _shaders;
		Ptr<BlendState> _blendState;
		std::vector<float> _blendFactor;
		uint32_t _blendSampleMask;
		Ptr<DepthStencilState> _depthStencilState;
		uint32_t _stencilRef;
		Ptr<RasterizerState> _rasterizerState;

		void UpdateVariables();
	};
}

#endif