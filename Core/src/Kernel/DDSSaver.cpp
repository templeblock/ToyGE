#include "ToyGE\Kernel\ImageHelper.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"

//#include "nvtt\nvtt.h"

namespace ToyGE
{
	//void SaveDDSImage(const Ptr<File> & outFile, const Ptr<Image> & image)
	//{
	//	auto path = outFile->Path();
	//	outFile->Release();

	//	using namespace nvtt;

	//	InputOptions inputOpts;
	//	nvtt::TextureType texType = image->Type() == TEXTURE_2D ? nvtt::TextureType_2D : nvtt::TextureType_Cube;
	//	inputOpts.setTextureLayout(texType, image->Width(), image->Height());
	//	inputOpts.setMipmapData(image->_rawImageData.get(), image->Width(), image->Height(), 1, image->ArraySize(), image->MipLevels());

	//	OutputOptions outOpts;
	//	String outPath;
	//	ConvertStr_WToA(path, outPath);
	//	outOpts.setFileName(outPath.c_str());

	//	CompressionOptions compressOpts;
	//	compressOpts.setFormat(Format_RGB);

	//	Compressor compressor;
	//	compressor.process(inputOpts, compressOpts, outOpts);
	//}
}