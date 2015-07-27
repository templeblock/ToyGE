#pragma once
#ifndef RENDERCOMMONDEFINES_H
#define RENDERCOMMONDEFINES_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderFormat.h"

namespace ToyGE
{
	class RenderResource;

	enum MapType : uint32_t
	{
		MAP_READ = 1UL,
		MAP_WRITE = 2UL,
		MAP_READ_WRITE = 3UL,
		MAP_WRITE_DISCARD = 4UL,
		MAP_WRITE_NO_OVERWRITE = 5UL
	};

	enum RenderResourceType
	{
		RENDER_RESOURCE_TEXTURE,
		RENDER_RESOURCE_BUFFER
	};

	enum TextureType
	{
		TEXTURE_UNDEFINED,
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE
	};

	enum RenderInputType : uint32_t
	{
		RENDER_INPUT_PER_VERTEX = 0UL,
		RENDER_INPUT_PER_INSHADERANCE = 1UL
	};

	enum ShaderType : uint32_t
	{
		SHADER_VS = 0UL,
		SHADER_HS = 1UL,
		SHADER_DS = 2UL,
		SHADER_GS = 3UL,
		SHADER_PS = 4UL,
		SHADER_CS = 5UL
	};

	enum ShaderTypeNum
	{
		NUM_SHADER_TYPE = SHADER_CS + 1
	};

	enum TextureBindFlag : uint32_t
	{
		TEXTURE_BIND_SHADER_RESOURCE = 0x1UL << 0UL,
		TEXTURE_BIND_STREAM_OUTPUT = 0x1UL << 1UL,
		TEXTURE_BIND_RENDER_TARGET = 0x1UL << 2UL,
		TEXTURE_BIND_DEPTH_STENCIL = 0x1UL << 3UL,
		TEXTURE_BIND_UNORDERED_ACCESS = 0x1UL << 4UL,
		TEXTURE_BIND_IMMUTABLE = 0x1UL << 5UL,
		TEXTURE_BIND_GENERATE_MIPS = 0x1UL << 6UL
	};

	enum BufferBindFlag : uint32_t
	{
		BUFFER_BIND_CONSTANT = 0x1UL << 0UL,
		BUFFER_BIND_VERTEX = 0x1UL << 1UL,
		BUFFER_BIND_INDEX = 0x1UL << 2UL,
		BUFFER_BIND_UNORDERED_ACCESS = 0x1UL << 3UL,
		BUFFER_BIND_SHADER_RESOURCE = 0x1UL << 4UL,
		BUFFER_BIND_RENDER_TARGET = 0x1UL << 5UL,
		BUFFER_BIND_STRUCTURED = 0x1UL << 6UL,
		BUFFER_BIND_IMMUTABLE = 0x1UL << 7UL,
		BUFFER_BIND_INDIRECT_ARGS = 0x1UL << 8UL
	};

	enum BufferUAVFlag : uint32_t
	{
		BUFFER_UAV_RAW = 0x1UL << 0UL,
		BUFFER_UAV_APPEND = 0x1UL << 1UL,
		BUFFER_UAV_COUNTER = 0x1UL << 2UL,
	};

	enum CpuAccessFlag : uint32_t
	{
		CPU_ACCESS_READ = 0x1UL << 0UL,
		CPU_ACCESS_WRITE = 0x1UL << 1UL,
	};

	enum CubeFace : uint32_t
	{
		CUBE_FACE_X_POSITIVE = 0UL,
		CUBE_FACE_X_NEGATIVE = 1UL,
		CUBE_FACE_Y_POSITIVE = 2UL,
		CUBE_FACE_Y_NEGATIVE = 3UL,
		CUBE_FACE_Z_POSITIVE = 4UL,
		CUBE_FACE_Z_NEGATIVE = 5UL
	};

	enum ComparisonFuc : uint32_t
	{
		COMPARISON_NEVER = 1,
		COMPARISON_LESS = 2,
		COMPARISON_EQUAL = 3,
		COMPARISON_LESS_EQUAL = 4,
		COMPARISON_GREATER = 5,
		COMPARISON_NOT_EQUAL = 6,
		COMPARISON_GREATER_EQUAL = 7,
		COMPARISON_ALWAYS = 8
	};

	enum DepthWriteMask : uint32_t
	{
		DEPTH_WRITE_ZERO = 0,
		DEPTH_WRITE_ALL = 1
	};

	enum FillMode : uint32_t
	{
		FILL_WIREFRAME = 2,
		FILL_SOLID = 3
	};

	enum CullMode : uint32_t
	{
		CULL_NONE = 1,
		CULL_FRONT = 2,
		CULL_BACK = 3
	};

	enum StencilOperation : uint32_t
	{
		STENCIL_OP_KEEP = 1,
		STENCIL_OP_ZERO = 2,
		STENCIL_OP_REPLACE = 3,
		STENCIL_OP_INCR_SAT = 4,
		STENCIL_OP_DECR_SAT = 5,
		STENCIL_OP_INVERT = 6,
		STENCIL_OP_INCR = 7,
		STENCIL_OP_DECR = 8
	};

	enum TextureAddressMode : uint32_t
	{
		TEXTURE_ADDRESS_WRAP = 1,
		TEXTURE_ADDRESS_MIRROR = 2,
		TEXTURE_ADDRESS_CLAMP = 3,
		TEXTURE_ADDRESS_BORDER = 4,
		TEXTURE_ADDRESS_MIRROR_ONCE = 5
	};

	enum ColorWriteMask : uint32_t
	{
		COLOR_WRITE_R = 1UL << 0UL,
		COLOR_WRITE_G = 1UL << 1UL,
		COLOR_WRITE_B = 1UL << 2UL,
		COLOR_WRITE_A = 1UL << 3UL,
		COLOR_WRITE_ALL = (COLOR_WRITE_R | COLOR_WRITE_G | COLOR_WRITE_B | COLOR_WRITE_A)
	};

	enum BlendParam : uint32_t
	{
		BLEND_PARAM_ZERO = 1,
		BLEND_PARAM_ONE = 2,
		BLEND_PARAM_SRC_COLOR = 3,
		BLEND_PARAM_INV_SRC_COLOR = 4,
		BLEND_PARAM_SRC_ALPHA = 5,
		BLEND_PARAM_INV_SRC_ALPHA = 6,
		BLEND_PARAM_DEST_ALPHA = 7,
		BLEND_PARAM_INV_DEST_ALPHA = 8,
		BLEND_PARAM_DEST_COLOR = 9,
		BLEND_PARAM_INV_DEST_COLOR = 10,
		BLEND_PARAM_SRC_ALPHA_SAT = 11,
		BLEND_PARAM_BLEND_FACTOR = 14,
		BLEND_PARAM_INV_BLEND_FACTOR = 15,
		BLEND_PARAM_SRC1_COLOR = 16,
		BLEND_PARAM_INV_SRC1_COLOR = 17,
		BLEND_PARAM_SRC1_ALPHA = 18,
		BLEND_PARAM_INV_SRC1_ALPHA = 19
	};

	enum BlendOperation : uint32_t
	{
		BLEND_OP_ADD = 1,
		BLEND_OP_SUBTRACT = 2,
		BLEND_OP_REV_SUBTRACT = 3,
		BLEND_OP_MIN = 4,
		BLEND_OP_MAX = 5
	};

	enum PrimitiveTopology : uint32_t
	{
		PRIMITIVE_TOPOLOGY_UNDEFINED = 0,
		PRIMITIVE_TOPOLOGY_POINTLIST = 1,
		PRIMITIVE_TOPOLOGY_LINELIST = 2,
		PRIMITIVE_TOPOLOGY_LINESTRIP = 3,
		PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4,
		PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5,
		PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10,
		PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ = 11,
		PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ = 12,
		PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ = 13,
		PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST = 33,
		PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST = 34,
		PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST = 35,
		PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST = 36,
		PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST = 37,
		PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST = 38,
		PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST = 39,
		PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST = 40,
		PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST = 41,
		PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST = 42,
		PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST = 43,
		PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST = 44,
		PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST = 45,
		PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST = 46,
		PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST = 47,
		PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST = 48,
		PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST = 49,
		PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST = 50,
		PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST = 51,
		PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST = 52,
		PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST = 53,
		PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST = 54,
		PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST = 55,
		PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST = 56,
		PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST = 57,
		PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST = 58,
		PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST = 59,
		PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST = 60,
		PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST = 61,
		PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST = 62,
		PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST = 63,
		PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST = 64
	};

	enum Filter : uint32_t
	{
		FILTER_MIN_MAG_MIP_POINT = 0,
		FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x1,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
		FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
		FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
		FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x14,
		FILTER_MIN_MAG_MIP_LINEAR = 0x15,
		FILTER_ANISOTROPIC = 0x55,
		FILTER_COMPARISON_MIN_MAG_MIP_POINT = 0x80,
		FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
		FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
		FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
		FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
		FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
		FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
		FILTER_COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
		FILTER_COMPARISON_ANISOTROPIC = 0xd5,
		FILTER_MINIMUM_MIN_MAG_MIP_POINT = 0x100,
		FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
		FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
		FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
		FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
		FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
		FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
		FILTER_MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
		FILTER_MINIMUM_ANISOTROPIC = 0x155,
		FILTER_MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
		FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
		FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
		FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
		FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
		FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
		FILTER_MAXIMUM_ANISOTROPIC = 0x1d5
	};

	struct MacroDesc
	{
		String name;
		String value;
	};

	struct RenderDataDesc
	{
		void * pData;
		size_t rowPitch;
		size_t slicePitch;
	};

	struct TextureDesc
	{
		TextureType type = TEXTURE_UNDEFINED;
		RenderFormat format = RENDER_FORMAT_UNDEFINED;
		int32_t width = 0;
		int32_t height = 0;
		int32_t depth = 0;
		int32_t arraySize = 1;
		int32_t mipLevels = 0;
		int32_t sampleCount = 1;
		int32_t sampleQuality = 0;
		uint32_t bindFlag = 0;
		uint32_t cpuAccess = 0;
	};

	enum VertexBufferType
	{
		VERTEX_BUFFER_GEOMETRY,
		VERTEX_BUFFER_INSHADERANCE
	};

	struct VertexElementDesc
	{
		String			name;
		int32_t		index;
		RenderFormat	format;
		int32_t		instanceDataRate;
		int32_t bytesOffset;
		int32_t bytesSize;
	};

	struct RenderBufferDesc
	{
		size_t elementSize;
		int32_t numElements;
		uint32_t bindFlag;
		uint32_t cpuAccess;
		size_t structedByteStride;
	};

	struct TextureSubDesc
	{
		bool bAsCube;
		int32_t firstMipLevel;
		int32_t mipLevels;
		union
		{
			struct
			{
				int32_t firstArray;
				int32_t arraySize;
			};

			//for cube
			struct
			{
				int32_t firstFaceOffset;
				int32_t numCubes;
			};
		};
		int32_t uavInitalCounts;
	};

	struct Box
	{
		int32_t left;
		int32_t right;
		int32_t top;
		int32_t bottom;
		int32_t front;
		int32_t back;
	};

	struct BufferSubDesc
	{
		int32_t firstElement;
		int32_t numElements;
		uint32_t uavFlags;
		int32_t uavInitalCounts;

		bool operator==(const BufferSubDesc & subDesc) const
		{
			return firstElement == subDesc.firstElement
				&& numElements == subDesc.numElements
				&& uavFlags == subDesc.uavFlags;
		}
	};

	struct RenderResourceSubDesc
	{
		union
		{
			TextureSubDesc textureDesc;
			BufferSubDesc bufferDesc;
		};

	};

	struct TOYGE_CORE_API ResourceView
	{
		Ptr<RenderResource> resource;
		RenderResourceSubDesc subDesc;
		RenderFormat formatHint;

		ResourceView() : formatHint(RENDER_FORMAT_UNDEFINED)
		{
		}

		bool operator==(const ResourceView & view) const;
		bool operator!=(const ResourceView & view) const
		{
			return !this->operator==(view);
		}
	};

	struct BlendRenderTargetDesc
	{
		bool     blendEnable = false;
		BlendParam    srcBlend = BLEND_PARAM_ONE;
		BlendParam    dstBlend = BLEND_PARAM_ZERO;
		BlendOperation blendOP = BLEND_OP_ADD;
		BlendParam    srcBlendAlpha = BLEND_PARAM_ONE;
		BlendParam    dstBlendAlpha = BLEND_PARAM_ONE;
		BlendOperation blendOPAlpha = BLEND_OP_ADD;
		uint8_t  renderTargetWriteMask = COLOR_WRITE_ALL;
	};

	struct BlendStateDesc
	{
		bool			alphaToCoverageEnable = false;
		bool			independentBlendEnable = false;
		BlendRenderTargetDesc	blendRTDesc[8];
	};

	struct StencilOperationDesc
	{
		StencilOperation      stencilFailOp = STENCIL_OP_KEEP;
		StencilOperation      stencilDepthFailOp = STENCIL_OP_KEEP;
		StencilOperation      stencilPassOp = STENCIL_OP_KEEP;
		ComparisonFuc stencilFunc = COMPARISON_ALWAYS;
	};

	struct DepthStencilStateDesc
	{
		bool                depthEnable = true;
		DepthWriteMask    depthWriteMask = DEPTH_WRITE_ALL;
		ComparisonFuc     depthFunc = COMPARISON_LESS;
		bool                stencilEnable = false;
		uint8_t             stencilReadMask = 0xff;
		uint8_t             stencilWriteMask = 0xff;
		StencilOperationDesc		frontFace;
		StencilOperationDesc		backFace;
	};

	struct RasterizerStateDesc
	{
		FillMode fillMode = FILL_SOLID;
		CullMode cullMode = CULL_BACK;
		bool      bFrontCounterClockwise = false;
		int32_t       depthBias = 0;
		float     depthBiasClamp = 0.0f;
		float     slopeScaledDepthBias = 0.0f;
		bool      depthClipEnable = true;
		bool      scissorEnable = false;
		bool      multisampleEnable = false;
		bool      antialiasedLineEnable = false;
	};

	struct SamplerDesc
	{
		Filter					filter = FILTER_MIN_MAG_MIP_LINEAR;
		TextureAddressMode	addressU = TEXTURE_ADDRESS_CLAMP;
		TextureAddressMode	addressV = TEXTURE_ADDRESS_CLAMP;
		TextureAddressMode	addressW = TEXTURE_ADDRESS_CLAMP;
		float                   mipLODBias = 0.0f;
		uint8_t                 maxAnisotropy = 0;
		ComparisonFuc			comparisonFunc = COMPARISON_NEVER;
		float                   borderColor[4];
		float                   minLOD = -FLT_MAX;
		float                   maxLOD = FLT_MAX;
	};

	struct BufferVarReflectionInfo
	{
		String name;
		size_t bytesOffset;
		size_t bytesSize;
	};

	struct BufferReflectionInfo
	{
		String name;
		size_t bytesSize;
		int32_t bindPoint;
		std::vector<BufferVarReflectionInfo> variables;
	};

	struct BoundResourceReflectionInfo
	{
		String name;
		int32_t bindPoint;
	};

	struct ShaderProgramReflectionInfo
	{
		std::vector<BufferReflectionInfo> buffers;
		std::vector<BoundResourceReflectionInfo> shaderResources;
		std::vector<BoundResourceReflectionInfo> uavs;
		std::vector<BoundResourceReflectionInfo> samplers;
	};

	struct Blob
	{
		void *bufferPointer;
		size_t bufferSize;
	};

	struct RenderDeviceAdapter
	{
		WString description;
	};

}

#endif