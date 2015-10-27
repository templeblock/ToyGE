#ifndef RENDEREFFECTVARIABLE_H
#define RENDEREFFECTVARIABLE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\Kernel\Assert.h"

#include "rapidxml.hpp"

namespace ToyGE
{
	enum RenderEffectVariableType
	{
		RENDER_EFFECT_VARIABLE_TEXTURE,
		RENDER_EFFECT_VARIABLE_TEXTUREARRAY,
		RENDER_EFFECT_VARIABLE_TEXTURECUBE,
		RENDER_EFFECT_VARIABLE_TEXTURECUBEARRAY,
		RENDER_EFFECT_VARIABLE_RWTEXTURE,
		RENDER_EFFECT_VARIABLE_RWTEXTUREARRAY,
		RENDER_EFFECT_VARIABLE_BUFFER,
		RENDER_EFFECT_VARIABLE_SRUCTUREDBUFFER,
		RENDER_EFFECT_VARIABLE_RWSRUCTUREDBUFFER,
		RENDER_EFFECT_VARIABLE_RWBUFFER,
		RENDER_EFFECT_VARIABLE_APPEDSRUCTUREDBUFFER,
		RENDER_EFFECT_VARIABLE_CONSUMESRUCTUREDBUFFER,
		RENDER_EFFECT_VARIABLE_BYTEADDRESSBUFFER,
		RENDER_EFFECT_VARIABLE_RWBYTEADDRESSBUFFER,
		RENDER_EFFECT_VARIABLE_SAMPLERSTATE,
		RENDER_EFFECT_VARIABLE_CBUFFER,
		RENDER_EFFECT_VARIABLE_SCALAR,
		RENDER_EFFECT_VARIABLE_BLENDSTATE,
		RENDER_EFFECT_VARIABLE_DEPTHSTENCILSTATE,
		RENDER_EFFECT_VARIABLE_RASTERIZERSTATE
	};

	class RenderEffectVariable_ShaderResource;
	class RenderEffectVariable_UAV;
	class RenderEffectVariable_Sampler;
	class RenderEffectVariable_ConstantBuffer;
	class RenderEffectVariable_Scalar;
	class RenderEffectVariable_DepthStencilState;
	class RenderEffectVariable_BlendState;
	class RenderEffectVariable_RasterizerState;

	class TOYGE_CORE_API RenderEffectVariable : public std::enable_shared_from_this<RenderEffectVariable>
	{
	public:
		static Ptr<RenderEffectVariable> Load(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		const String & GetTypeStr() const
		{
			return _typeStr;
		}

		const String & Name() const
		{
			return _name;
		}

		//CLASS_GET(Name, String, _name);
		//CLASS_SET(Name, String, _name);

		virtual String GetCodeString() const
		{
			return String();
		}

		CLASS_GET(Register, String, _register);

		bool IsDirty() const
		{
			return _dirty;
		}

		void SetDirty(bool dirty)
		{
			_dirty = dirty;
		}

		/*virtual bool IsValid() const
		{
			return true;
		}*/

		virtual bool IsShaderResource() const
		{
			return false;
		}
		virtual bool IsUAV() const
		{
			return false;
		}
		virtual bool IsSampler() const
		{
			return false;
		}
		virtual bool IsConstantBuffer() const
		{
			return false;
		}
		virtual bool IsScalar() const
		{
			return false;
		}
		virtual bool IsBlendState() const
		{
			return false;
		}
		virtual bool IsDepthStencilState() const
		{
			return false;
		}
		virtual bool IsRasterizerState() const
		{
			return false;
		}

		Ptr<RenderEffectVariable_ShaderResource> AsShaderResource()
		{
			return std::static_pointer_cast<RenderEffectVariable_ShaderResource>(shared_from_this());
		}
		Ptr<RenderEffectVariable_UAV> AsUAV()
		{
			return std::static_pointer_cast<RenderEffectVariable_UAV>(shared_from_this());
		}
		Ptr<RenderEffectVariable_Sampler> AsSampler()
		{
			return std::static_pointer_cast<RenderEffectVariable_Sampler>(shared_from_this());
		}
		Ptr<RenderEffectVariable_ConstantBuffer> AsConstantBuffer()
		{
			return std::static_pointer_cast<RenderEffectVariable_ConstantBuffer>(shared_from_this());
		}
		Ptr<RenderEffectVariable_Scalar> AsScalar()
		{
			return std::static_pointer_cast<RenderEffectVariable_Scalar>(shared_from_this());
		}
		Ptr<RenderEffectVariable_BlendState> AsBlendState()
		{
			return std::static_pointer_cast<RenderEffectVariable_BlendState>(shared_from_this());
		}
		Ptr<RenderEffectVariable_DepthStencilState> AsDepthStencilState()
		{
			return std::static_pointer_cast<RenderEffectVariable_DepthStencilState>(shared_from_this());
		}
		Ptr<RenderEffectVariable_RasterizerState> AsRasterizerState()
		{
			return std::static_pointer_cast<RenderEffectVariable_RasterizerState>(shared_from_this());
		}

	protected:
		String _typeStr;
		String _name;
		RenderEffectVariableType _type;
		bool _dirty = false;
		String _register;
	};


	class TOYGE_CORE_API RenderEffectVariable_ShaderResource : public RenderEffectVariable
	{
	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		String GetCodeString() const override
		{
			std::stringstream ss;

			//type
			ss << GetTypeStr();

			//format
			if (FormatStr().size() > 0)
				ss << "<" << FormatStr() << ">";

			//name
			ss << " " << Name();

			//register
			if (GetRegister().size() > 0)
				ss << " : register(" << GetRegister() << ");";
			else
				ss << ";";

			return ss.str();
		}

		const ResourceView & Value() const
		{
			return _resource;
		}

		void SetValue(const ResourceView & resourceView)
		{
			if (_resource != resourceView)
			{
				_resource = resourceView;
				_dirty = true;
			}
		}

		bool IsShaderResource() const override
		{
			return true;
		}

		const String & FormatStr() const
		{
			return _formatStr;
		}

	protected:
		ResourceView _resource;
		String _formatStr;
	};

	class TOYGE_CORE_API RenderEffectVariable_UAV : public RenderEffectVariable_ShaderResource
	{
	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		//String GetCodeString() const override
		//{

		//}

		//const ResourceView & Value() const
		//{
		//	return _resource;
		//}

		//void SetValue(const ResourceView & resourceView)
		//{
		//	if (_resource != resourceView)
		//	{
		//		_resource = resourceView;
		//		_dirty = true;
		//	}
		//
		//}

		bool IsShaderResource() const override
		{
			return false;
		}

		bool IsUAV() const override
		{
			return true;
		}

	//	const String & FormatStr() const
	//	{
	//		return _formatStr;
	//	}

	//private:
	//	ResourceView _resource;
	//	String _formatStr;
	};

	class Sampler;

	class TOYGE_CORE_API RenderEffectVariable_Sampler : public RenderEffectVariable
	{
	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		String GetCodeString() const override
		{
			std::stringstream ss;
			ss << GetTypeStr() << " " << Name() << ";";
			return ss.str();
		}

		const Ptr<Sampler> & Value() const
		{
			return _sampler;
		}

		bool IsSampler() const override
		{
			return true;
		}

	private:
		Ptr<Sampler> _sampler;
	};


	class BlendState;

	class TOYGE_CORE_API RenderEffectVariable_BlendState : public RenderEffectVariable
	{
	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		RenderEffectVariable_BlendState()
			: _blendFactor({ 1.0f, 1.0f, 1.0f, 1.0f }),
			_blendSampleMask(0xffffffff)
		{

		}

		const Ptr<BlendState> & GetBlendState() const
		{
			return _blendState;
		}

		const std::vector<float> & BlendFactor() const
		{
			return _blendFactor;
		}

		uint32_t BlendSampleMask() const
		{
			return _blendSampleMask;
		}

		bool IsBlendState() const override
		{
			return true;
		}

	private:
		Ptr<BlendState> _blendState;
		std::vector<float> _blendFactor;
		uint32_t _blendSampleMask;
	};


	class DepthStencilState;

	class TOYGE_CORE_API RenderEffectVariable_DepthStencilState : public RenderEffectVariable
	{
	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);
		
		const Ptr<DepthStencilState> & GetDepthStencilState() const
		{
			return _depthStencilState;
		}

		uint32_t StencilRef() const
		{
			return _stencilRef;
		}

		bool IsDepthStencilState() const override
		{
			return true;
		}

	private:
		Ptr<DepthStencilState> _depthStencilState;
		uint32_t _stencilRef = 0;
	};


	class RasterizerState;

	class TOYGE_CORE_API RenderEffectVariable_RasterizerState : public RenderEffectVariable
	{
	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		const Ptr<RasterizerState> & GetRasterizerState() const
		{
			return _rasterizerState;
		}

		bool IsRasterizerState() const override
		{
			return true;
		}

	private:
		Ptr<RasterizerState> _rasterizerState;
	};


	class RenderBuffer;

	class TOYGE_CORE_API RenderEffectVariable_ConstantBuffer : public RenderEffectVariable
	{
		friend class RenderEffectVariable_Scalar;

	public:
		static Ptr<RenderEffectVariable> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		std::shared_ptr<RenderEffectVariable_ConstantBuffer> CreateCopy() const;

		String GetCodeString() const override;

		bool IsConstantBuffer() const override
		{
			return true;
		}

		int32_t NumVariables() const
		{
			return static_cast<int32_t>(_variables.size());
		}

		Ptr<RenderEffectVariable_Scalar> VariableByIndex(int32_t index)
		{
			return _variables[index];
		}

		Ptr<RenderEffectVariable_Scalar> VariableByName(const String & name);

		/*const Ptr<RenderBuffer> & Value() const
		{
			return _renderBuffer;
		}*/

		CLASS_GET(Value, Ptr<RenderBuffer>, _renderBuffer);

		void SetValue(const Ptr<RenderBuffer> & buffer)
		{
			_renderBuffer = buffer;
			_dirty = true;
		}

		CLASS_GET(Size, size_t, _size);
		CLASS_SET(Size, size_t, _size);

		/*size_t Size() const
		{
			return _size;
		}*/

		void InitFromReflectionInfo(const BufferReflectionInfo & desc);

		bool IsExtraMacrosRelevant() const
		{
			return _bExtraMacrosRelevant;
		}

		bool IsShared() const
		{
			return _bShared;
		}

	private:
		size_t _size;
		std::vector<Ptr<RenderEffectVariable_Scalar>> _variables;
		Ptr<RenderBuffer> _renderBuffer;
		bool _bExtraMacrosRelevant;
		bool _bShared;

		RenderEffectVariable_ConstantBuffer()
			: _size(0),
			_bExtraMacrosRelevant(false)
		{

		};
	};

	enum SCALAR_TYPE
	{
		SCALAR_float,
		SCALAR_float2,
		SCALAR_float2x2,
		SCALAR_float2x3,
		SCALAR_float2x4,
		SCALAR_float3,
		SCALAR_float3x2,
		SCALAR_float3x3,
		SCALAR_float3x4,
		SCALAR_float4,
		SCALAR_float4x2,
		SCALAR_float4x3,
		SCALAR_float4x4,
		SCALAR_int,
		SCALAR_int2,
		SCALAR_int3,
		SCALAR_int4,
		SCALAR_uint,
		SCALAR_uint2,
		SCALAR_uint3,
		SCALAR_uint4
	};


	class TOYGE_CORE_API RenderEffectVariable_Scalar : public RenderEffectVariable
	{
		friend class RenderEffectVariable_ConstantBuffer;

	public:
		static Ptr<RenderEffectVariable_Scalar> Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

		std::shared_ptr<RenderEffectVariable_Scalar> CreateCopy() const;

		String GetCodeString() const override
		{
			std::stringstream ss;
			ss << GetTypeStr() << " " << Name();
			if (ArraySize() > 1)
				ss << "[" << ArraySize() << "]";
			else if (ArraySize() == 0)
				ss << "[" << _arraySizeStr << "]";

			ss << ";";

			return ss.str();
		}

		bool IsScalar() const override
		{
			return true;
		}

		SCALAR_TYPE ScalarType() const
		{
			return _scalarType;
		}

		void SetValue(const void * val, size_t size = 0, size_t offset = 0)
		{
			ToyGE_ASSERT(offset + size <= _size);
			ToyGE_ASSERT(val);

			size_t cpySize = std::min<size_t>(size, _size);
			if (cpySize == 0)
				cpySize = _size;

			if (memcmp(_data.get() + offset, val, cpySize) != 0)
			{
				memcpy(_data.get() + offset, val, cpySize);
				_dirty = true;
				if (!_cbuffer.expired())
					_cbuffer.lock()->SetDirty(true);
			}
		}

		const void * Value() const
		{
			return _data.get();
		}

		void SetConstantBuffer(const std::weak_ptr<RenderEffectVariable_ConstantBuffer> & cbuffer)
		{
			_cbuffer = cbuffer;
		}

		const std::weak_ptr<RenderEffectVariable_ConstantBuffer> & ConstantBuffer() const
		{
			return _cbuffer;
		}

		size_t BytesOffset() const
		{
			return _bytesOffset;
		}

		size_t Size() const
		{
			return _size;
		}

		int32_t ArraySize() const
		{
			return _arraySize;
		}

		bool IsExtraMacrosRelevant() const
		{
			return _bExtraMacrosRelevant;
		}

	private:
		std::weak_ptr<RenderEffectVariable_ConstantBuffer> _cbuffer;
		std::shared_ptr<uint8_t> _data;
		SCALAR_TYPE _scalarType;
		size_t _bytesOffset;
		size_t _size;
		int32_t _arraySize;
		String _arraySizeStr;
		bool _bExtraMacrosRelevant;

		RenderEffectVariable_Scalar()
			: _bytesOffset(0),
			_size(0),
			_arraySize(0),
			_bExtraMacrosRelevant(false)
		{

		}
	};

}

#endif