#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	String GetShaderModelName(ShaderModel sm)
	{
		std::map<ShaderModel, String> smNameMap = 
		{
			{ SM_4, "SM_4" },
			{ SM_5, "SM_5" }
		};
		return smNameMap[sm];
	}
}