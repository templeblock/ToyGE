#include "ToyGE\RenderEngine\Panel.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	Panel::Panel()
		: _size(0.0f),
		_backgroundColor(-1.0f)
	{

	}

	bool Panel::Intersect(const float2 & parentSpacePos) const
	{
		return parentSpacePos.x >= (GetPos().x) && parentSpacePos.x <= (GetPos().x + GetSize().x)
			&& parentSpacePos.y >= (GetPos().y) && parentSpacePos.y <= (GetPos().y + GetSize().y);
	}

	void Panel::RenderSelf(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		if (_backgroundColor.w == 0.0f || _size.x <= 0.0f || _size.y <= 0.0f)
			return;

		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"Widget.xml");
		if (_backgroundTex)
			fx->VariableByName("renderTex")->AsShaderResource()->SetValue(_backgroundTex->CreateTextureView());

		if (_backgroundColor.w > 0.0f)
		{
			fx->VariableByName("color")->AsScalar()->SetValue(&_backgroundColor);
		}
		else if (_backgroundColor.w < 0.0f)
		{
			float4 defaultColor = 1.0f;
			fx->VariableByName("color")->AsScalar()->SetValue(&_backgroundColor);
		}

		float2 screenPos = LocalPosToScreen(0.0f);

		RenderQuad(fx->TechniqueByName(_backgroundTex ? "RenderPanelTex" : "RenderPanel"),
			static_cast<int32_t>(screenPos.x), static_cast<int32_t>(screenPos.y),
			static_cast<int32_t>(_size.x), static_cast<int32_t>(_size.y));
	}

	
}