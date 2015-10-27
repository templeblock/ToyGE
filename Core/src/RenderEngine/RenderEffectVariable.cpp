#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	typedef Ptr<RenderEffectVariable>(*VariableCreateFunc)(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros);

	static const std::map<String, RenderEffectVariableType> _variableTypeMap = 
	{
		{ "Texture1D",          RENDER_EFFECT_VARIABLE_TEXTURE },
		{ "Texture1DArray",     RENDER_EFFECT_VARIABLE_TEXTUREARRAY },
		{ "Texture2D",          RENDER_EFFECT_VARIABLE_TEXTURE },
		{ "Texture2DArray",     RENDER_EFFECT_VARIABLE_TEXTUREARRAY },
		{ "Texture3D",			RENDER_EFFECT_VARIABLE_TEXTURE },
		{ "TextureCube",        RENDER_EFFECT_VARIABLE_TEXTURECUBE },
		{ "TextureCubeArray",   RENDER_EFFECT_VARIABLE_TEXTURECUBEARRAY },
		{ "RWTexture1D",        RENDER_EFFECT_VARIABLE_RWTEXTURE },
		{ "RWTexture1DArray",   RENDER_EFFECT_VARIABLE_RWTEXTUREARRAY },
		{ "RWTexture2D",        RENDER_EFFECT_VARIABLE_RWTEXTURE },
		{ "RWTexture2DArray",   RENDER_EFFECT_VARIABLE_RWTEXTUREARRAY },
		{ "RWTexture3D",        RENDER_EFFECT_VARIABLE_RWTEXTURE },
		{ "Buffer", RENDER_EFFECT_VARIABLE_BUFFER },
		{ "StructuredBuffer", RENDER_EFFECT_VARIABLE_SRUCTUREDBUFFER },
		{ "RWStructuredBuffer", RENDER_EFFECT_VARIABLE_RWSRUCTUREDBUFFER },
		{ "RWBuffer", RENDER_EFFECT_VARIABLE_RWBUFFER },
		{ "AppendStructuredBuffer", RENDER_EFFECT_VARIABLE_APPEDSRUCTUREDBUFFER },
		{ "ConsumeStructuredBuffer", RENDER_EFFECT_VARIABLE_CONSUMESRUCTUREDBUFFER },
		{ "ByteAddressBuffer", RENDER_EFFECT_VARIABLE_BYTEADDRESSBUFFER},
		{ "RWByteAddressBuffer", RENDER_EFFECT_VARIABLE_RWBYTEADDRESSBUFFER },
		{ "SamplerState",       RENDER_EFFECT_VARIABLE_SAMPLERSTATE },
		{ "SamplerComparisonState", RENDER_EFFECT_VARIABLE_SAMPLERSTATE },
		{ "cbuffer",            RENDER_EFFECT_VARIABLE_CBUFFER },
		{ "BlendState",         RENDER_EFFECT_VARIABLE_BLENDSTATE },
		{ "DepthStencilState",  RENDER_EFFECT_VARIABLE_DEPTHSTENCILSTATE },
		{ "RasterizerState",    RENDER_EFFECT_VARIABLE_RASTERIZERSTATE }
	};

	static const std::map<RenderEffectVariableType, VariableCreateFunc> _variableCreateMap =
	{
		{ RENDER_EFFECT_VARIABLE_TEXTURE,           &RenderEffectVariable_ShaderResource::Create },
		{ RENDER_EFFECT_VARIABLE_TEXTUREARRAY,      &RenderEffectVariable_ShaderResource::Create },
		{ RENDER_EFFECT_VARIABLE_TEXTURECUBE,         &RenderEffectVariable_ShaderResource::Create },
		{ RENDER_EFFECT_VARIABLE_TEXTURECUBEARRAY, &RenderEffectVariable_ShaderResource::Create },
		{ RENDER_EFFECT_VARIABLE_RWTEXTURE,         &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_RWTEXTUREARRAY,    &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_BUFFER, &RenderEffectVariable_ShaderResource::Create },
		{ RENDER_EFFECT_VARIABLE_SRUCTUREDBUFFER, &RenderEffectVariable_ShaderResource::Create },
		{ RENDER_EFFECT_VARIABLE_RWSRUCTUREDBUFFER, &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_RWBUFFER, &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_APPEDSRUCTUREDBUFFER, &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_CONSUMESRUCTUREDBUFFER, &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_BYTEADDRESSBUFFER, &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_RWBYTEADDRESSBUFFER, &RenderEffectVariable_UAV::Create },
		{ RENDER_EFFECT_VARIABLE_SAMPLERSTATE,        &RenderEffectVariable_Sampler::Create },
		{ RENDER_EFFECT_VARIABLE_CBUFFER,             &RenderEffectVariable_ConstantBuffer::Create },
		{ RENDER_EFFECT_VARIABLE_BLENDSTATE,          &RenderEffectVariable_BlendState::Create },
		{ RENDER_EFFECT_VARIABLE_DEPTHSTENCILSTATE,   &RenderEffectVariable_DepthStencilState::Create },
		{ RENDER_EFFECT_VARIABLE_RASTERIZERSTATE,     &RenderEffectVariable_RasterizerState::Create }
	};

	Ptr<RenderEffectVariable> RenderEffectVariable::Load(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end())
		{
			Logger::LogLine("effect error> invalid variable type [type:%s]", typeStr.c_str());
			return Ptr<RenderEffectVariable>();
		}

		auto var = _variableCreateMap.find(typeFind->second)->second(node, macros);
		return var;
	}

	Ptr<RenderEffectVariable> RenderEffectVariable_ShaderResource::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		if (nullptr == node)
			return Ptr<RenderEffectVariable>();

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end())
			return Ptr<RenderEffectVariable>();

		//set varibale type
		auto var = std::make_shared<RenderEffectVariable_ShaderResource>();

		RenderEffectVariableType varType = typeFind->second;
		var->_type = varType;
		var->_typeStr = typeStr;
		var->_name = node->first_attribute("name")->value();
		auto formatAttr = node->first_attribute("format");
		if (formatAttr)
			var->_formatStr = formatAttr->value();
		else
			var->_formatStr = "float4";

		auto registerAttr = node->first_attribute("register");
		if (registerAttr)
			var->_register = registerAttr->value();

		return var;
	}


	Ptr<RenderEffectVariable> RenderEffectVariable_UAV::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		if (nullptr == node)
			return Ptr<RenderEffectVariable>();

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end())
			return Ptr<RenderEffectVariable>();

		//set varibale type
		auto var = std::make_shared<RenderEffectVariable_UAV>();

		RenderEffectVariableType varType = typeFind->second;
		var->_type = varType;
		var->_typeStr = typeStr;
		var->_name = node->first_attribute("name")->value();
		auto formatAttri = node->first_attribute("format");
		if(formatAttri)
			var->_formatStr = formatAttri->value();
		auto registerAttr = node->first_attribute("register");
		if (registerAttr)
			var->_register = registerAttr->value();

		return var;
	}


	static const std::map<String, Filter> _filterMap =
	{
		{ "MIN_MAG_MIP_POINT",                          FILTER_MIN_MAG_MIP_POINT },
		{ "MIN_MAG_POINT_MIP_LINEAR",                   FILTER_MIN_MAG_POINT_MIP_LINEAR },
		{ "MIN_POINT_MAG_LINEAR_MIP_POINT",	            FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT },
		{ "MIN_POINT_MAG_MIP_LINEAR",                   FILTER_MIN_POINT_MAG_MIP_LINEAR },
		{ "MIN_LINEAR_MAG_MIP_POINT",                   FILTER_MIN_LINEAR_MAG_MIP_POINT },
		{ "MIN_LINEAR_MAG_POINT_MIP_LINEAR",			FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
		{ "MIN_MAG_LINEAR_MIP_POINT",					FILTER_MIN_MAG_LINEAR_MIP_POINT },
		{ "MIN_MAG_MIP_LINEAR",							FILTER_MIN_MAG_MIP_LINEAR },
		{ "ANISOTROPIC",								FILTER_ANISOTROPIC },
		{ "COMPARISON_MIN_MAG_MIP_POINT",				FILTER_COMPARISON_MIN_MAG_MIP_POINT },
		{ "COMPARISON_MIN_MAG_POINT_MIP_LINEAR",        FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR },
		{ "COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT",  FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT },
		{ "COMPARISON_MIN_POINT_MAG_MIP_LINEAR",        FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR },
		{ "COMPARISON_MIN_LINEAR_MAG_MIP_POINT",        FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT },
		{ "COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR", FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
		{ "COMPARISON_MIN_MAG_LINEAR_MIP_POINT",        FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT },
		{ "COMPARISON_MIN_MAG_MIP_LINEAR",              FILTER_COMPARISON_MIN_MAG_MIP_LINEAR },
		{ "COMPARISON_ANISOTROPIC",                     FILTER_COMPARISON_ANISOTROPIC },
		{ "MINIMUM_MIN_MAG_MIP_POINT",					FILTER_MINIMUM_MIN_MAG_MIP_POINT },
		{ "MINIMUM_MIN_MAG_POINT_MIP_LINEAR",			FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR },
		{ "MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT",		FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT },
		{ "MINIMUM_MIN_POINT_MAG_MIP_LINEAR",			FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR },
		{ "MINIMUM_MIN_LINEAR_MAG_MIP_POINT",			FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT },
		{ "MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR",	FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
		{ "MINIMUM_MIN_MAG_LINEAR_MIP_POINT",			FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT },
		{ "MINIMUM_MIN_MAG_MIP_LINEAR",					FILTER_MINIMUM_MIN_MAG_MIP_LINEAR },
		{ "MINIMUM_ANISOTROPIC",						FILTER_MINIMUM_ANISOTROPIC },
		{ "MAXIMUM_MIN_MAG_MIP_POINT",					FILTER_MAXIMUM_MIN_MAG_MIP_POINT },
		{ "MAXIMUM_MIN_MAG_POINT_MIP_LINEAR",			FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR },
		{ "MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT",		FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT },
		{ "MAXIMUM_MIN_POINT_MAG_MIP_LINEAR",			FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR },
		{ "MAXIMUM_MIN_LINEAR_MAG_MIP_POINT",			FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT },
		{ "MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR",	FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
		{ "MAXIMUM_MIN_MAG_LINEAR_MIP_POINT",			FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT },
		{ "MAXIMUM_MIN_MAG_MIP_LINEAR",					FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR },
		{ "MAXIMUM_ANISOTROPIC",						FILTER_MAXIMUM_ANISOTROPIC },
	};

	static const std::map<String, TextureAddressMode> _textureAddressModeMap = 
	{
		{ "WRAP",			TEXTURE_ADDRESS_WRAP },
		{ "MIRROR",			TEXTURE_ADDRESS_MIRROR },
		{ "CLAMP",			TEXTURE_ADDRESS_CLAMP },
		{ "BORDER",			TEXTURE_ADDRESS_BORDER },
		{ "MIRROR_ONCE",	TEXTURE_ADDRESS_MIRROR_ONCE }
	};

	static const std::map<String, ComparisonFuc> _comparisonFuncMap = 
	{
		{ "NEVER",			COMPARISON_NEVER },
		{ "LESS",			COMPARISON_LESS },
		{ "EQUAL",			COMPARISON_EQUAL },
		{ "LESS_EQUAL",		COMPARISON_LESS_EQUAL },
		{ "GREATER",		COMPARISON_GREATER },
		{ "NOT_EQUAL",		COMPARISON_NOT_EQUAL },
		{ "GREATER_EUQAL",	COMPARISON_GREATER_EQUAL },
		{ "ALWAYS",			COMPARISON_ALWAYS }
	};

	Ptr<RenderEffectVariable> RenderEffectVariable_Sampler::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		if (nullptr == node)
			return Ptr<RenderEffectVariable>();

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end())
			return Ptr<RenderEffectVariable>();
		if (RENDER_EFFECT_VARIABLE_SAMPLERSTATE != typeFind->second)
			return Ptr<RenderEffectVariable>();

		auto samplerVar = std::make_shared<RenderEffectVariable_Sampler>();
		samplerVar->_type = RENDER_EFFECT_VARIABLE_SAMPLERSTATE;
		samplerVar->_typeStr = typeStr;
		samplerVar->_name = node->first_attribute("name")->value();

		//set sampler state
		SamplerDesc samDesc;
		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String stateNameStr = stateNode->first_attribute("name")->value();
			String stateValueStr = stateNode->first_attribute("value")->value();
			if ("filter" == stateNameStr)
			{
				auto filterFind = _filterMap.find(stateValueStr);
				if (filterFind == _filterMap.end())
					return Ptr<RenderEffectVariable>();

				samDesc.filter = filterFind->second;
			}
			else if ("addressU" == stateNameStr)
			{
				auto addressFind = _textureAddressModeMap.find(stateValueStr);
				if (addressFind == _textureAddressModeMap.end())
					return Ptr<RenderEffectVariable>();

				samDesc.addressU = addressFind->second;
			}
			else if ("addressV" == stateNameStr)
			{
				auto addressFind = _textureAddressModeMap.find(stateValueStr);
				if (addressFind == _textureAddressModeMap.end())
					return Ptr<RenderEffectVariable>();

				samDesc.addressV = addressFind->second;
			}
			else if ("addressW" == stateNameStr)
			{
				auto addressFind = _textureAddressModeMap.find(stateValueStr);
				if (addressFind == _textureAddressModeMap.end())
					return Ptr<RenderEffectVariable>();

				samDesc.addressW = addressFind->second;
			}
			else if ("mipLODBias" == stateNameStr)
			{
				samDesc.mipLODBias = std::stof(stateValueStr);
			}
			else if ("maxAnisotropy" == stateNameStr)
			{
				samDesc.maxAnisotropy = static_cast<uint8_t>( std::stoi(stateValueStr) );
			}
			else if ("comparisonFunc" == stateNameStr)
			{
				auto compareFuncFind = _comparisonFuncMap.find(stateValueStr);
				if (compareFuncFind == _comparisonFuncMap.end())
					return Ptr<RenderEffectVariable>();

				samDesc.comparisonFunc = compareFuncFind->second;
			}
			else if ("borderColor" == stateNameStr)
			{
				std::stringstream ss(stateValueStr);
				ss >> samDesc.borderColor[0];
				ss >> samDesc.borderColor[1];
				ss >> samDesc.borderColor[2];
				ss >> samDesc.borderColor[3];
			}
			else if ("minLOD" == stateNameStr)
			{
				samDesc.minLOD = std::stof(stateValueStr);
			}
			else if ("maxLOD" == stateNameStr)
			{
				samDesc.maxLOD = std::stof(stateValueStr);
			}
			else
			{
				Logger::LogLine("effect error> invalid sampler state [type:%s]", stateNameStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}
		samplerVar->_sampler = Global::GetRenderEngine()->GetRenderFactory()->GetSamplerPooled(samDesc);

		return samplerVar;
	}


	Ptr<RenderEffectVariable> RenderEffectVariable_ConstantBuffer::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		if (nullptr == node)
			return Ptr<RenderEffectVariable>();

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end() || RENDER_EFFECT_VARIABLE_CBUFFER != typeFind->second)
			return Ptr<RenderEffectVariable>();

		auto cbufferVar = std::shared_ptr<RenderEffectVariable_ConstantBuffer>(new RenderEffectVariable_ConstantBuffer());
		cbufferVar->_type = RENDER_EFFECT_VARIABLE_CBUFFER;
		cbufferVar->_typeStr = typeStr;
		cbufferVar->_name = node->first_attribute("name")->value();

		cbufferVar->_bExtraMacrosRelevant = false;
		auto bufVarNode = node->first_node("variable");
		while (bufVarNode)
		{
			auto var = RenderEffectVariable_Scalar::Create(bufVarNode, macros);
			if (var)
			{
				var->SetConstantBuffer(cbufferVar);
				cbufferVar->_variables.push_back(var);

				cbufferVar->_bExtraMacrosRelevant |= var->IsExtraMacrosRelevant();
			}

			bufVarNode = bufVarNode->next_sibling("variable");
		}

		cbufferVar->_bShared = false;
		auto bufPropertyNode = node->first_node("property");
		while (bufPropertyNode)
		{
			String propertyName = bufPropertyNode->first_attribute("name")->value();
			if (propertyName == "shared")
				cbufferVar->_bShared = true;

			bufPropertyNode = bufPropertyNode->next_sibling("property");
		}

		return cbufferVar;
	}

	String RenderEffectVariable_ConstantBuffer::GetCodeString() const
	{
		std::stringstream ss;
		//auto cbufVar = cbuf->AsConstantBuffer();
		ss << "cbuffer " << Name() << "{";
		for (auto & var : _variables)
		{
			//auto scalarVar = cbufVar->VariableByIndex(bufVarIndex)->AsScalar();
			/*ss << scalarVar->GetTypeStr() << " " << scalarVar->Name();
			if (scalarVar->ArraySize() > 0)
			{
			ss << "[" << scalarVar->ArraySize() << "]";
			}
			ss << ";";*/
			ss << var->GetCodeString();
		}
		ss << "}";

		return ss.str();
	}

	std::shared_ptr<RenderEffectVariable_ConstantBuffer> RenderEffectVariable_ConstantBuffer::CreateCopy() const
	{
		auto cb = std::shared_ptr<RenderEffectVariable_ConstantBuffer>(new RenderEffectVariable_ConstantBuffer());

		*static_cast<RenderEffectVariable*>(cb.get()) = *(static_cast<const RenderEffectVariable*>(this));
		/*cb->_typeStr = _typeStr;
		cb->_type = _type;
		cb->_name = _name;
		cb->_dirty = _dirty;
		cb->_register = _register;*/

		cb->_size = _size;
		cb->_bExtraMacrosRelevant = _bExtraMacrosRelevant;
		cb->_bShared = _bShared;
		cb->_renderBuffer = _renderBuffer;
		for (auto & scalar : _variables)
		{
			auto scalarCpy = scalar->CreateCopy();
			scalarCpy->SetConstantBuffer(cb);
			cb->_variables.push_back(scalarCpy);
		}

		return cb;
	}

	Ptr<RenderEffectVariable_Scalar> RenderEffectVariable_ConstantBuffer::VariableByName(const String & name)
	{
		for (auto & var : _variables)
		{
			if (var->Name() == name)
				return var;
		}
		return Ptr<RenderEffectVariable_Scalar>();
	}

	void RenderEffectVariable_ConstantBuffer::InitFromReflectionInfo(const BufferReflectionInfo & desc)
	{
		_size = desc.bytesSize;

		//std::vector<Ptr<RenderEffectVariable_Scalar>> vars;
		for (auto & varDesc : desc.variables)
		{
			auto scalarVar = VariableByName(varDesc.name);
			if (scalarVar)
			{
				scalarVar->_bytesOffset = varDesc.bytesOffset;
				scalarVar->_size = varDesc.bytesSize;
				if (!_bShared)
					scalarVar->_data = MakeBufferedDataShared(varDesc.bytesSize);
				if (scalarVar->_arraySizeStr.size() > 0)
					scalarVar->_arraySize = static_cast<int32_t>(scalarVar->_size / sizeof(float4));

				_variables.push_back(scalarVar);
			}
		}

		if (!_bShared)
		{
			RenderBufferDesc bufDesc;
			bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
			bufDesc.cpuAccess = CPU_ACCESS_WRITE;
			bufDesc.numElements = 1;
			bufDesc.elementSize = _size;

			_renderBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(bufDesc, 0);
		}
		//_bExtraMacrosRelevant = true;
		//_isInitReflectionInfo = true;
	}

	static const std::map<String, BlendParam> _blendParamMap = 
	{
		{ "ZERO",                BLEND_PARAM_ZERO },
		{ "ONE",                 BLEND_PARAM_ONE },
		{ "SRC_COLOR",           BLEND_PARAM_SRC_COLOR },
		{ "INV_SRC_COLOR",       BLEND_PARAM_INV_SRC_COLOR },
		{ "SRC_ALPHA",           BLEND_PARAM_SRC_ALPHA },
		{ "INV_SRC_ALPHA",       BLEND_PARAM_INV_SRC_ALPHA },
		{ "DEST_ALPHA",          BLEND_PARAM_DEST_ALPHA },
		{ "INV_DEST_ALPHA",      BLEND_PARAM_INV_DEST_ALPHA },
		{ "DEST_COLOR",          BLEND_PARAM_DEST_COLOR },
		{ "INV_DEST_COLOR",      BLEND_PARAM_INV_DEST_COLOR },
		{ "SRC_ALPHA_SAT",       BLEND_PARAM_SRC_ALPHA_SAT },
		{ "BLEND_FACTOR",        BLEND_PARAM_BLEND_FACTOR },
		{ "INV_BLEND_FACTOR",    BLEND_PARAM_INV_BLEND_FACTOR },
		{ "SRC1_COLOR",          BLEND_PARAM_SRC1_COLOR },
		{ "INV_SRC1_COLOR",      BLEND_PARAM_INV_SRC1_COLOR },
		{ "SRC1_ALPHA",          BLEND_PARAM_SRC1_ALPHA },
		{ "INV_SRC1_ALPHA",      BLEND_PARAM_INV_SRC1_ALPHA }
	};

	static const std::map<String, BlendOperation> _blendOperationMap =
	{
		{ "ADD",           BLEND_OP_ADD },
		{ "SUBTRACT",      BLEND_OP_SUBTRACT },
		{ "REV_SUBTRACT",  BLEND_OP_REV_SUBTRACT },
		{ "MIN",           BLEND_OP_MIN },
		{ "MAX",           BLEND_OP_MAX }
	};

	static void _LoadBlendRenderTargetDesc(const rapidxml::xml_node<> * node, BlendRenderTargetDesc & outDesc)
	{
		ToyGE_ASSERT(node);

		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String stateNameStr = stateNode->first_attribute("name")->value();
			String stateValueStr = stateNode->first_attribute("value")->value();
			if ("blendEnable" == stateNameStr)
			{
				outDesc.blendEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("srcBlend" == stateNameStr)
			{
				auto blendParamFind = _blendParamMap.find(stateValueStr);
				if (blendParamFind != _blendParamMap.end())
				{
					outDesc.srcBlend = blendParamFind->second;
				}
				else
				{
					Logger::LogLine("effect error> blend param invalid [value:%s]", stateValueStr);
				}
			}
			else if ("dstBlend" == stateNameStr)
			{
				auto blendParamFind = _blendParamMap.find(stateValueStr);
				if (blendParamFind != _blendParamMap.end())
				{
					outDesc.dstBlend = blendParamFind->second;
				}
				else
				{
					Logger::LogLine("effect error> blend param invalid [value:%s]", stateValueStr);
				}
			}
			else if ("blendOP" == stateNameStr)
			{
				auto blendOPFind = _blendOperationMap.find(stateValueStr);
				if (blendOPFind != _blendOperationMap.end())
				{
					outDesc.blendOP = blendOPFind->second;
				}
				else
				{
					Logger::LogLine("effect error> blend operation invalid [value:%s]", stateValueStr);
				}
			}
			else if ("srcBlendAlpha" == stateNameStr)
			{
				auto blendParamFind = _blendParamMap.find(stateValueStr);
				if (blendParamFind != _blendParamMap.end())
				{
					outDesc.srcBlendAlpha = blendParamFind->second;
				}
				else
				{
					Logger::LogLine("effect error> blend param invalid [value:%s]", stateValueStr);
				}
			}
			else if ("dstBlendAlpha" == stateNameStr)
			{
				auto blendParamFind = _blendParamMap.find(stateValueStr);
				if (blendParamFind != _blendParamMap.end())
				{
					outDesc.dstBlendAlpha = blendParamFind->second;
				}
				else
				{
					Logger::LogLine("effect error> blend param invalid [value:%s]", stateValueStr);
				}
			}
			else if ("blendOPAlpha" == stateNameStr)
			{
				auto blendOPFind = _blendOperationMap.find(stateValueStr);
				if (blendOPFind != _blendOperationMap.end())
				{
					outDesc.blendOPAlpha = blendOPFind->second;
				}
				else
				{
					Logger::LogLine("effect error> blend operation invalid [value:%s]", stateValueStr);
				}
			}
			else if ("renderTargetWriteMask" == stateNameStr)
			{
				outDesc.renderTargetWriteMask = static_cast<uint8_t>(std::stoi(stateValueStr, 0, 16));
			}
			else
			{
				Logger::LogLine("effect error> invalid blend render target state [type:%s]", stateNameStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}
	}

	Ptr<RenderEffectVariable> RenderEffectVariable_BlendState::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		ToyGE_ASSERT(node);

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end() || RENDER_EFFECT_VARIABLE_BLENDSTATE != typeFind->second)
		{
			ToyGE_ASSERT_FAIL("");
			//return Ptr<RenderEffectVariable>();
		}

		auto blendStateVar = std::make_shared<RenderEffectVariable_BlendState>();
		blendStateVar->_typeStr = typeStr;
		blendStateVar->_type = RENDER_EFFECT_VARIABLE_BLENDSTATE;
		blendStateVar->_name = node->first_attribute("name")->value();

		BlendStateDesc blendDesc;
		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String stateNameStr = stateNode->first_attribute("name")->value();
			String stateValueStr;
			auto stateValueAttr = stateNode->first_attribute("value");
			if (stateValueAttr)
				stateValueStr = stateNode->first_attribute("value")->value();
			if ("alphaToCoverageEnable" == stateNameStr)
			{
				blendDesc.alphaToCoverageEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("independentBlendEnable" == stateNameStr)
			{
				blendDesc.independentBlendEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("blendRTDesc" == stateNameStr)
			{
				int32_t index = std::stoi(stateNode->first_attribute("index")->value());
				if (index >= 0 && index < 8)
				{
					auto & blendRTDesc = blendDesc.blendRTDesc[index];
					_LoadBlendRenderTargetDesc(stateNode, blendRTDesc);
				}
				else
				{
					Logger::LogLine("effect error> blend render target index invalid(0-7) [index:%d]", index);
				}
			}
			else if ("blendFactor" == stateNameStr)
			{
				std::stringstream ss(stateValueStr);
				float fac;
				for (int i = 0; i < 4; ++i)
				{
					ss >> fac;
					blendStateVar->_blendFactor.push_back(fac);
				}
			}
			else if ("blendSampleMask" == stateNameStr)
			{
				blendStateVar->_blendSampleMask = std::stoul(stateValueStr, 0, 16);
			}
			else
			{
				Logger::LogLine("effect error> invalid blend state [type:%s]", stateNameStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}
		blendStateVar->_blendState = Global::GetRenderEngine()->GetRenderFactory()->GetBlendStatePooled(blendDesc);
		ToyGE_ASSERT(blendStateVar->_blendState);

		return blendStateVar;
	}


	static const std::map<String, StencilOperation> _stencilOperationMap = 
	{
		{ "KEEP",     STENCIL_OP_KEEP },
		{ "ZERO",     STENCIL_OP_ZERO },
		{ "REPLACE",  STENCIL_OP_REPLACE },
		{ "INCR_SAT", STENCIL_OP_INCR_SAT },
		{ "DECR_SAT", STENCIL_OP_DECR_SAT },
		{ "INVERT",   STENCIL_OP_INVERT },
		{ "INCR",     STENCIL_OP_INCR },
		{ "DECR",     STENCIL_OP_DECR }
	};

	//static const std::map<String, DepthWriteMask> _depthWriteMaskMap = 
	//{

	//};

	static void _LoadStencilOperationDesc(const rapidxml::xml_node<> * node, StencilOperationDesc & outDesc)
	{
		ToyGE_ASSERT(node);

		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String stateNameStr = stateNode->first_attribute("name")->value();
			String stateValueStr = stateNode->first_attribute("value")->value();
			if ("stencilFailOp" == stateNameStr)
			{
				auto opFind = _stencilOperationMap.find(stateValueStr);
				if (opFind != _stencilOperationMap.end())
					outDesc.stencilFailOp = opFind->second;
				else
					Logger::LogLine("effect error> invalid stencil operation(stencilFailOp) [value:%s]", stateValueStr.c_str());
			}
			else if ("stencilDepthFailOp" == stateNameStr)
			{
				auto opFind = _stencilOperationMap.find(stateValueStr);
				if (opFind != _stencilOperationMap.end())
					outDesc.stencilDepthFailOp = opFind->second;
				else
					Logger::LogLine("effect error> invalid stencil operation(stencilFailOp) [value:%s]", stateValueStr.c_str());
			}
			else if ("stencilPassOp" == stateNameStr)
			{
				auto opFind = _stencilOperationMap.find(stateValueStr);
				if (opFind != _stencilOperationMap.end())
					outDesc.stencilPassOp = opFind->second;
				else
					Logger::LogLine("effect error> invalid stencil operation(stencilFailOp) [value:%s]", stateValueStr.c_str());
			}
			else if ("stencilFunc" == stateNameStr)
			{
				auto funcFind = _comparisonFuncMap.find(stateValueStr);
				if (funcFind != _comparisonFuncMap.end())
					outDesc.stencilFunc = funcFind->second;
				else
					Logger::LogLine("effect error> invalid stencil func [value:%s]", stateValueStr.c_str());
			}
			else
			{
				Logger::LogLine("effect error> invalid stencil operation desc state [name:%s]", stateNameStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}
	}

	Ptr<RenderEffectVariable> RenderEffectVariable_DepthStencilState::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		ToyGE_ASSERT(node);

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end() || RENDER_EFFECT_VARIABLE_DEPTHSTENCILSTATE != typeFind->second)
		{
			ToyGE_ASSERT_FAIL("");
		}

		auto depthStencilStateVar = std::make_shared<RenderEffectVariable_DepthStencilState>();
		depthStencilStateVar->_typeStr = typeStr;
		depthStencilStateVar->_type = RENDER_EFFECT_VARIABLE_DEPTHSTENCILSTATE;
		depthStencilStateVar->_name = node->first_attribute("name")->value();

		DepthStencilStateDesc dssDesc;
		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String stateNameStr = stateNode->first_attribute("name")->value();
			String stateValueStr;
			auto stateValueAttr = stateNode->first_attribute("value");
			if (stateValueAttr)
				stateValueStr = stateValueAttr->value();
			if ("depthEnable" == stateNameStr)
			{
				dssDesc.depthEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("depthWriteMask" == stateNameStr)
			{
				if ("ZERO" == stateValueStr)
					dssDesc.depthWriteMask = DEPTH_WRITE_ZERO;
				else if ("ALL" == stateValueStr)
					dssDesc.depthWriteMask = DEPTH_WRITE_ALL;
				else
					Logger::LogLine("effect error> invalid depth write value [value:%s]", stateValueStr.c_str());
			}
			else if ("depthFunc" == stateNameStr)
			{
				auto funcFind = _comparisonFuncMap.find(stateValueStr);
				if (funcFind != _comparisonFuncMap.end())
				{
					dssDesc.depthFunc = funcFind->second;
				}
				else
				{
					Logger::LogLine("effect error> invalid depth func value [value:%s]", stateValueStr.c_str());
				}
			}
			else if ("stencilEnable" == stateNameStr)
			{
				dssDesc.stencilEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("stencilReadMask" == stateNameStr)
			{
				dssDesc.stencilReadMask = static_cast<uint8_t>(std::stoi(stateValueStr, 0, 16));
			}
			else if ("stencilWriteMask" == stateNameStr)
			{
				dssDesc.stencilWriteMask = static_cast<uint8_t>(std::stoi(stateValueStr, 0, 16));
			}
			else if ("frontFace" == stateNameStr)
			{
				_LoadStencilOperationDesc(stateNode, dssDesc.frontFace);
			}
			else if ("backFace" == stateNameStr)
			{
				_LoadStencilOperationDesc(stateNode, dssDesc.backFace);
			}
			else if ("stencilRef" == stateNameStr)
			{
				depthStencilStateVar->_stencilRef = std::stoul(stateValueStr);
			}
			else
			{
				Logger::LogLine("effect error> invalid depth stencil state [name:%s]", stateNameStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}

		depthStencilStateVar->_depthStencilState = Global::GetRenderEngine()->GetRenderFactory()->GetDepthStencilStatePooled(dssDesc);
		ToyGE_ASSERT(depthStencilStateVar->_depthStencilState);

		return depthStencilStateVar;
	}

	Ptr<RenderEffectVariable> RenderEffectVariable_RasterizerState::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		ToyGE_ASSERT(node);

		String typeStr = node->first_attribute("type")->value();
		auto typeFind = _variableTypeMap.find(typeStr);
		if (typeFind == _variableTypeMap.end() || RENDER_EFFECT_VARIABLE_RASTERIZERSTATE != typeFind->second)
		{
			ToyGE_ASSERT_FAIL("");
		}

		auto rasterizerStateVar = std::make_shared<RenderEffectVariable_RasterizerState>();
		rasterizerStateVar->_typeStr = typeStr;
		rasterizerStateVar->_type = RENDER_EFFECT_VARIABLE_RASTERIZERSTATE;
		rasterizerStateVar->_name = node->first_attribute("name")->value();

		RasterizerStateDesc rsDesc;
		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String stateNameStr = stateNode->first_attribute("name")->value();
			String stateValueStr = stateNode->first_attribute("value")->value();
			if ("fillMode" == stateNameStr)
			{
				if ("WIREFRAME" == stateValueStr)
					rsDesc.fillMode = FILL_WIREFRAME;
				else if ("SOLID" == stateValueStr)
					rsDesc.fillMode = FILL_SOLID;
				else
					Logger::LogLine("effect error> invalid fill mode [value:%s]", stateValueStr.c_str());
			}
			else if ("cullMode" == stateNameStr)
			{
				if ("NONE" == stateValueStr)
					rsDesc.cullMode = CULL_NONE;
				else if ("FRONT" == stateValueStr)
					rsDesc.cullMode = CULL_FRONT;
				else if ("BACK" == stateValueStr)
					rsDesc.cullMode = CULL_BACK;
				else
					Logger::LogLine("effect error> invalid cull mode [value:%s]", stateValueStr.c_str());
			}
			else if ("bFrontCounterClockwise" == stateNameStr)
			{
				rsDesc.bFrontCounterClockwise = ("true" == stateValueStr) ? true : false;
			}
			else if ("depthBias" == stateNameStr)
			{
				rsDesc.depthBias = std::stoi(stateValueStr);
			}
			else if ("depthBiasClamp" == stateNameStr)
			{
				rsDesc.depthBiasClamp = std::stof(stateValueStr);
			}
			else if ("slopeScaledDepthBias" == stateNameStr)
			{
				rsDesc.slopeScaledDepthBias = std::stof(stateValueStr);
			}
			else if ("depthClipEnable" == stateNameStr)
			{
				rsDesc.depthClipEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("scissorEnable" == stateNameStr)
			{
				rsDesc.scissorEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("multisampleEnable" == stateNameStr)
			{
				rsDesc.multisampleEnable = ("true" == stateValueStr) ? true : false;
			}
			else if ("antialiasedLineEnable" == stateNameStr)
			{
				rsDesc.antialiasedLineEnable = ("true" == stateValueStr) ? true : false;
			}
			else
			{
				Logger::LogLine("effect error> invalid rasterizer state [value:%s]", stateNameStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}
		rasterizerStateVar->_rasterizerState = Global::GetRenderEngine()->GetRenderFactory()->GetRasterizerStatePooled(rsDesc);
		ToyGE_ASSERT(rasterizerStateVar->_rasterizerState);

		return rasterizerStateVar;
	}

	static const std::map<String, SCALAR_TYPE> _scalarTypeMap = 
	{
		{ "float",		SCALAR_float },
		{ "float2",		SCALAR_float2 },
		{ "float2x2",	SCALAR_float2x2 },
		{ "float2x3",	SCALAR_float2x3 },
		{ "float2x4",	SCALAR_float2x4 },
		{ "float3",		SCALAR_float3 },
		{ "float3x2",	SCALAR_float3x2 },
		{ "float3x3",	SCALAR_float3x3 },
		{ "float3x4",	SCALAR_float3x4 },
		{ "float4",		SCALAR_float4 },
		{ "float4x2",	SCALAR_float4x2 },
		{ "float4x3",	SCALAR_float4x3 },
		{ "float4x4",	SCALAR_float4x4 },
		{ "int",		SCALAR_int },
		{ "int2",		SCALAR_int2 },
		{ "int3",		SCALAR_int3 },
		{ "int4",		SCALAR_int4 },
		{ "uint",		SCALAR_uint },
		{ "uint2",		SCALAR_uint2 },
		{ "uint3",		SCALAR_uint3 },
		{ "uint4",		SCALAR_uint4 }
	};

	//static const std::map<SCALAR_TYPE, size_t> _scalarSizeMap =
	//{
	//	{ SCALAR_float,	(sizeof(float))				},
	//	{ SCALAR_float2, (sizeof(float)) * 2 },
	//	{ SCALAR_float2x2, (sizeof(float)) * 4 },
	//	{ SCALAR_float2x3, (sizeof(float)) * 6 },
	//	{ SCALAR_float2x4, (sizeof(float)) * 8 },
	//	{ SCALAR_float3, (sizeof(float)) * 3 },
	//	{ SCALAR_float3x2, (sizeof(float)) * 6 },
	//	{ SCALAR_float3x3, (sizeof(float)) * 9 },
	//	{ SCALAR_float3x4, (sizeof(float)) * 12 },
	//	{ SCALAR_float4, (sizeof(float)) * 4 },
	//	{ SCALAR_float4x2, (sizeof(float)) * 8 },
	//	{ SCALAR_float4x3, (sizeof(float)) * 12 },
	//	{ SCALAR_float4x4, (sizeof(float)) * 16 },
	//	{ SCALAR_int, (sizeof(int32_t)) },
	//	{ SCALAR_int2, (sizeof(int32_t)) * 2 },
	//	{ SCALAR_int3, (sizeof(int32_t)) * 3 },
	//	{ SCALAR_int4, (sizeof(int32_t)) * 4 },
	//	{ SCALAR_uint, (sizeof(uint32_t)) },
	//	{ SCALAR_uint2, (sizeof(uint32_t)) * 2 },
	//	{ SCALAR_uint3, (sizeof(uint32_t)) * 3 },
	//	{ SCALAR_uint4, (sizeof(uint32_t)) * 4 }
	//};

	Ptr<RenderEffectVariable_Scalar> RenderEffectVariable_Scalar::Create(const rapidxml::xml_node<> * node, const std::vector<MacroDesc> & macros)
	{
		if (nullptr == node)
			return Ptr<RenderEffectVariable_Scalar>();

		String typeStr = node->first_attribute("type")->value();
		auto scalarTypeFind = _scalarTypeMap.find(typeStr);
		if (scalarTypeFind == _scalarTypeMap.end())
			return Ptr<RenderEffectVariable_Scalar>();

		auto scalar = std::shared_ptr<RenderEffectVariable_Scalar>(new RenderEffectVariable_Scalar());
		scalar->_type = RENDER_EFFECT_VARIABLE_SCALAR;
		scalar->_typeStr = typeStr;
		scalar->_name = node->first_attribute("name")->value();

		scalar->_scalarType = scalarTypeFind->second;

		auto arraySizeAttr = node->first_attribute("arraySize");
		if (arraySizeAttr)
		{
			auto sizeStr = String(arraySizeAttr->value());
			bool macroReplaceSuccess = true;
			size_t macroRefBegin = 0;
			while ( (macroRefBegin = sizeStr.find_first_of('$', macroRefBegin)) != std::string::npos )
			{
				++macroRefBegin;
				auto end = sizeStr.find_first_of(' ', macroRefBegin);
				auto macro = sizeStr.substr(macroRefBegin, end - macroRefBegin);
				bool macroFind = false;
				String macroValue;
				for (auto & i : macros)
				{
					if (i.name == macro)
					{
						macroValue = i.value;
						macroFind = true;
						break;
					}
				}

				if (macroFind)
				{
					sizeStr.replace(macroRefBegin - 1, end - macroRefBegin + 1, macroValue);
				}
				else //No MaroFound
				{
					scalar->_arraySize = 0;
					macroReplaceSuccess = false;
					break;
				}
			}
			//if (macroReplaceSuccess)
			//	scalar->_arraySize = Math::Calculate<int>(sizeStr);

			scalar->_arraySize = 0;
			scalar->_bExtraMacrosRelevant = !macroReplaceSuccess;
			scalar->_arraySizeStr = arraySizeAttr->value();
			for (auto & c : scalar->_arraySizeStr)
				if (c == '$')
					c = ' ';
		}
		else
		{
			scalar->_arraySize = 1;
			scalar->_bExtraMacrosRelevant = false;
		}

		return scalar;
	}

	std::shared_ptr<RenderEffectVariable_Scalar> RenderEffectVariable_Scalar::CreateCopy() const
	{
		auto var = std::shared_ptr<RenderEffectVariable_Scalar>(new RenderEffectVariable_Scalar());

		*static_cast<RenderEffectVariable*>(var.get()) = *(static_cast<const RenderEffectVariable*>(this));

		var->_scalarType = _scalarType;
		var->_bytesOffset = _bytesOffset;
		var->_size = _size;
		var->_arraySize = _arraySize;
		var->_arraySizeStr = _arraySizeStr;
		var->_bExtraMacrosRelevant = _bExtraMacrosRelevant;

		if (var->_size > 0)
		{
			var->_data = MakeBufferedDataShared(var->_size);
			memcpy(var->_data.get(), _data.get(), _size);
		}

		return var;
	}
}