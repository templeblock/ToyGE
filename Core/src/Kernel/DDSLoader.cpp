#include "ToyGE\Kernel\ImageLoader.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\Assert.h"

namespace ToyGE
{
#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BUFFER_BINDitMask == b && ddpf.ABitMask == a )

#ifndef DXGI_FORMAT_DEFINED
	enum DXGI_FORMAT
	{
		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		DXGI_FORMAT_FORCE_UINT = 0xffffffff
	};
#endif

	enum D3D11_RESOURCE_DIMENSION
	{
		D3D11_RESOURCE_DIMENSION_UNKNOWN = 0,
		D3D11_RESOURCE_DIMENSION_BUFFER = 1,
		D3D11_RESOURCE_DIMENSION_TEXTURE1D = 2,
		D3D11_RESOURCE_DIMENSION_TEXTURE2D = 3,
		D3D11_RESOURCE_DIMENSION_TEXTURE3D = 4
	};

	enum D3D11_RESOURCE_MISC_FLAG
	{
		D3D11_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		D3D11_RESOURCE_MISC_SHARED = 0x2L,
		D3D11_RESOURCE_MISC_TEXTURECUBE = 0x4L,
		D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS = 0x10L,
		D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = 0x20L,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED = 0x40L,
		D3D11_RESOURCE_MISC_RESOURCE_CLAMP = 0x80L,
		D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x100L,
		D3D11_RESOURCE_MISC_GDI_COMPATIBLE = 0x200L,
		D3D11_RESOURCE_MISC_SHARED_NTHANDLE = 0x800L,
		D3D11_RESOURCE_MISC_RESHADERRICTED_CONTENT = 0x1000L,
		D3D11_RESOURCE_MISC_RESHADERRICT_SHARED_RESOURCE = 0x2000L,
		D3D11_RESOURCE_MISC_RESHADERRICT_SHARED_RESOURCE_DRIVER = 0x4000L,
		D3D11_RESOURCE_MISC_GUARDED = 0x8000L,
		D3D11_RESOURCE_MISC_TILE_POOL = 0x20000L,
		D3D11_RESOURCE_MISC_TILED = 0x40000L
	};

	const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

	struct DDS_PIXELFORMAT
	{
		uint32_t    size;
		uint32_t    flags;
		uint32_t    fourCC;
		uint32_t    RGBUFFER_BINDitCount;
		uint32_t    RBitMask;
		uint32_t    GBitMask;
		uint32_t    BUFFER_BINDitMask;
		uint32_t    ABitMask;
	};

	struct DDS_HEADER
	{
		uint32_t        size;
		uint32_t        flags;
		uint32_t        height;
		uint32_t        width;
		uint32_t        pitchOrLinearSize;
		uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
		uint32_t        mipMapCount;
		uint32_t        reserved1[11];
		DDS_PIXELFORMAT ddspf;
		uint32_t        caps;
		uint32_t        caps2;
		uint32_t        caps3;
		uint32_t        caps4;
		uint32_t        reserved2;
	};

	struct DDS_HEADER_DXT10
	{
		DXGI_FORMAT     dxgiFormat;
		uint32_t        resourceDimension;
		uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
		uint32_t        arraySize;
		uint32_t        miscFlags2;
	};

	static size_t BitsPerPixel( DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return 32;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:

#ifdef DXGI_1_2_FORMATS
		case DXGI_FORMAT_B4G4R4A4_UNORM:
#endif
			return 16;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			return 0;
		}
	}

	static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
	{
		if (ddpf.flags & DDS_RGB)
		{
			// Note that sRGB formats are written using the "DX10" extended header

			switch (ddpf.RGBUFFER_BINDitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				{
					return DXGI_FORMAT_B8G8R8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				{
					return DXGI_FORMAT_B8G8R8X8_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assumme
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				{
					return DXGI_FORMAT_R10G10B10A2_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16G16_UNORM;
				}

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
				{
					// Only 32-bit color channel format in D3D9 was R32F
					return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
				}
				break;

			case 24:
				// No 24bpp DXGI formats aka D3DFMT_R8G8B8
				break;

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				{
					return DXGI_FORMAT_B5G5R5A1_UNORM;
				}
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
				{
					return DXGI_FORMAT_B5G6R5_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

#ifdef DXGI_1_2_FORMATS
				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				{
					return DXGI_FORMAT_B4G4R4A4_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4
#endif

				// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE)
		{
			if (8 == ddpf.RGBUFFER_BINDitCount)
			{
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
			}

			if (16 == ddpf.RGBUFFER_BINDitCount)
			{
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				{
					return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
			}
		}
		else if (ddpf.flags & DDS_ALPHA)
		{
			if (8 == ddpf.RGBUFFER_BINDitCount)
			{
				return DXGI_FORMAT_A8_UNORM;
			}
		}
		else if (ddpf.flags & DDS_FOURCC)
		{
			if (MakeFourCC<'D', 'X', 'T', '1'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC1_UNORM;
			}
			if (MakeFourCC<'D', 'X', 'T', '3'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MakeFourCC<'D', 'X', 'T', '5'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			// While pre-mulitplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			if (MakeFourCC<'D', 'X', 'T', '2'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MakeFourCC<'D', 'X', 'T', '4'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			if (MakeFourCC<'A', 'T', 'I', '1'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MakeFourCC<'B', 'C', '4', 'U'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MakeFourCC<'B', 'C', '4', 'S'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_SNORM;
			}

			if (MakeFourCC<'A', 'T', 'I', '2'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MakeFourCC<'B', 'C', '5', 'U'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MakeFourCC<'B', 'C', '5', 'S'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_SNORM;
			}

			// BC6H and BC7 are written using the "DX10" extended header

			if (MakeFourCC<'R', 'G', 'B', 'G'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			}
			if (MakeFourCC<'G', 'R', 'G', 'B'>::value == ddpf.fourCC)
			{
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
			}

			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				return DXGI_FORMAT_R16G16B16A16_UNORM;

			case 110: // D3DFMT_Q16W16V16U16
				return DXGI_FORMAT_R16G16B16A16_SNORM;

			case 111: // D3DFMT_R16F
				return DXGI_FORMAT_R16_FLOAT;

			case 112: // D3DFMT_G16R16F
				return DXGI_FORMAT_R16G16_FLOAT;

			case 113: // D3DFMT_A16B16G16R16F
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			case 114: // D3DFMT_R32F
				return DXGI_FORMAT_R32_FLOAT;

			case 115: // D3DFMT_G32R32F
				return DXGI_FORMAT_R32G32_FLOAT;

			case 116: // D3DFMT_A32B32G32R32F
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	template <class T>
	inline
		T * CastAndUpdatePos(const uint8_t *data, size_t & pos)
	{
		pos += sizeof(T);
		return reinterpret_cast<T*>( const_cast<uint8_t*>(data) + pos - sizeof(T) );
	}

	bool GetDDSHeader(const uint8_t * data, size_t dataSize, DDS_HEADER **outHeader, size_t & curDataPos, bool &hasDXT10)
	{
		uint32_t *magic = CastAndUpdatePos<uint32_t>(data, curDataPos);

		if (*magic != DDS_MAGIC)
			return false;

		DDS_HEADER *header = CastAndUpdatePos<DDS_HEADER>(data, curDataPos);
		if (header->size != sizeof(DDS_HEADER) ||
			header->ddspf.size != sizeof(DDS_PIXELFORMAT))
			return false;

		hasDXT10 = false;
		if ((header->ddspf.flags & DDS_FOURCC) &&
			(MakeFourCC<'D', 'X', '1', '0'>::value == header->ddspf.fourCC))
		{
			if (dataSize < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10)))
				return false;

			curDataPos += sizeof(DDS_HEADER_DXT10);
			hasDXT10 = true;
		}

		*outHeader = header;
		return true;
	}

	static void GetSurfaceInfo
		( size_t width,
		 size_t height,
		 DXGI_FORMAT fmt,
		 size_t* outNumBytes,
		 size_t* outRowBytes,
		 size_t* outNumRows)
	{
		size_t numBytes = 0;
		size_t rowBytes = 0;
		size_t numRows = 0;

		bool bc = false;
		bool packed = false;
		size_t bcnumBytesPerBlock = 0;
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			bc = true;
			bcnumBytesPerBlock = 8;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			bc = true;
			bcnumBytesPerBlock = 16;
			break;

		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			packed = true;
			break;
		}

		if (bc)
		{
			size_t numBlocksWide = 0;
			if (width > 0)
			{
				numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
			}
			size_t numBlocksHigh = 0;
			if (height > 0)
			{
				numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
			}
			rowBytes = numBlocksWide * bcnumBytesPerBlock;
			numRows = numBlocksHigh;
		}
		else if (packed)
		{
			rowBytes = ((width + 1) >> 1) * 4;
			numRows = height;
		}
		else
		{
			size_t bpp = BitsPerPixel(fmt);
			rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
			numRows = height;
		}

		numBytes = rowBytes * numRows;
		if (outNumBytes)
		{
			*outNumBytes = numBytes;
		}
		if (outRowBytes)
		{
			*outRowBytes = rowBytes;
		}
		if (outNumRows)
		{
			*outNumRows = numRows;
		}
	}

	bool LoadDDSImage(const Ptr<File> & file, const Ptr<Image> & image)
	{
		ToyGE_ASSERT(file);
		ToyGE_ASSERT(image);
		if (!file->CanRead())
			return false;

		std::shared_ptr<uint8_t> dataBuffer = std::shared_ptr<uint8_t>( new uint8_t[file->Size()], [](uint8_t *p){delete[] p; } );

		if (!dataBuffer || nullptr == dataBuffer)
			return false;

		size_t fileSize = file->Size();
		size_t bytesRead = file->Read(&dataBuffer.get()[0], file->Size());

		if (bytesRead != fileSize)
			return false;

		size_t curDataPos = 0;

		DDS_HEADER *header;
		bool hasDXT10;
		if (false == GetDDSHeader(dataBuffer.get(), fileSize, &header, curDataPos, hasDXT10) )
			return false;


		size_t width = header->width;
		size_t height = header->height;
		size_t depth = header->depth;

		uint32_t resDim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		size_t arraySize = 1;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		TextureType tType;
		bool isCubeMap = false;

		size_t mipCount = header->mipMapCount;
		if (0 == mipCount)
		{
			mipCount = 1;
		}

		if (hasDXT10)
		{
			DDS_HEADER_DXT10 *pDXT10 = reinterpret_cast<DDS_HEADER_DXT10*>(header + sizeof(DDS_HEADER));

			arraySize = pDXT10->arraySize;
			if (arraySize == 0)
				return false;

			if (BitsPerPixel(pDXT10->dxgiFormat) == 0)
				return false;

			format = pDXT10->dxgiFormat;

			switch (pDXT10->resourceDimension)
			{
			case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((header->flags & DDS_HEIGHT) && height != 1)
					return false;

				height = depth = 1;

				tType = TEXTURE_1D;
				break;

			case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
				if (pDXT10->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE)
				{
					arraySize *= 6;
					isCubeMap = true;
					tType = TEXTURE_CUBE;
				}
				else
					tType = TEXTURE_2D;
				depth = 1;
				break;

			case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
				if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
					return false;

				if (arraySize > 1)
					return false;
				tType = TEXTURE_3D;
				break;

			default:
				return false;
			}

			resDim = pDXT10->resourceDimension;
		}
		else
		{
			format = GetDXGIFormat(header->ddspf);

			if (format == DXGI_FORMAT_UNKNOWN)
				return false;

			if (header->flags & DDS_HEADER_FLAGS_VOLUME)
			{
				resDim = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
				tType = TEXTURE_3D;
			}
			else
			{
				if (header->caps2 & DDS_CUBEMAP)
				{
					// We require all six faces to be defined
					if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
						return false;

					arraySize = 6;
					isCubeMap = true;
					tType = TEXTURE_CUBE;
				}
				else
				{
					resDim = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
					tType = TEXTURE_2D;
				}
				depth = 1;
				// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
			}

			assert(BitsPerPixel(format) != 0);
		}

		std::vector<RenderDataDesc> imageDatas(arraySize * mipCount);

		size_t NumBytes = 0;
		size_t RowBytes = 0;
		size_t NumRows = 0;

		uint8_t *pImageData = dataBuffer.get() + curDataPos;
		//uint32_t bitsPerPixel = static_cast<uint32_t>(BitsPerPixel(format));

		size_t index = 0;
		uint32_t twidth = 0;
		uint32_t theight = 0;
		uint32_t tdepth = 0;
		for (size_t j = 0; j < arraySize; j++)
		{
			size_t w = width;
			size_t h = height;
			size_t d = depth;
			for (size_t i = 0; i < mipCount; i++)
			{
				GetSurfaceInfo(
					w,
					h,
					format,
					&NumBytes,
					&RowBytes,
					&NumRows
					);

				if (!twidth)
				{
					twidth = static_cast<uint32_t>(w);
					theight = static_cast<uint32_t>(h);
					tdepth = static_cast<uint32_t>(d);
				}

				assert(index < mipCount * arraySize);

				imageDatas[index].pData = (void*)pImageData;
				imageDatas[index].rowPitch = static_cast<UINT>(RowBytes);
				imageDatas[index].slicePitch = static_cast<UINT>(NumBytes);
				++index;

				pImageData += NumBytes * d;

				w = std::max<size_t>(1, w >> 1);
				h = std::max<size_t>(1, h >> 1);
				d = std::max<size_t>(1, d >> 1);
			}
		}

		image->_type = tType;
		image->_format = static_cast<RenderFormat>(format);
		if (IsCompress(image->_format))
		{ 
			image->_width = static_cast<int32_t>((twidth + 3) / 4 * 4);
			image->_height = static_cast<int32_t>((theight + 3) / 4 * 4);
		}
		else
		{
			image->_width = static_cast<int32_t>(twidth);
			image->_height = static_cast<int32_t>(theight);
		}
		image->_depth = static_cast<int32_t>(tdepth);
		image->_arraySize = static_cast<int32_t>(isCubeMap ? arraySize / 6 : arraySize);
		image->_mipLevels = static_cast<int32_t>(mipCount);
		image->_data = imageDatas;
		image->_rawImageData = dataBuffer;

		return true;
	}


}