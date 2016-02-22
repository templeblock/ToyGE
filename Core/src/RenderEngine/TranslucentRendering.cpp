#include "ToyGE\RenderEngine\TranslucentRendering.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\SceneRenderer.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\RenderComponent.h"

namespace ToyGE
{
	struct TranslucentRenderObj
	{
		Ptr<class Material> mat;
		Ptr<class RenderComponent> renderComponent;
	};

	class TranslucentDepthDrawingPolicy : public DrawingPolicy
	{
	public:
		Ptr<RenderView> _view;
		Ptr<Material> _mat;

		virtual void BindView(const Ptr<class RenderView> & view) override
		{
			_view = view;
		}

		virtual void BindMaterial(const Ptr<class Material> & mat) override
		{
			_mat = mat;
		}

		virtual void Draw(const Ptr<class RenderComponent> & renderComponent) override
		{
			if (!_mat || !_mat->IsTranslucent())
				return;

			std::map<String, String> macros;

			bool bWithOpacityMask = false;

			_mat->BindDepthMacros(macros);
			bWithOpacityMask = _mat->GetTexture(MAT_TEX_OPACITYMASK).size() > 0;

			auto meshElement = renderComponent->GetMeshElement();
			if (meshElement)
				meshElement->BindDepthMacros(bWithOpacityMask, macros);

			auto vs = Shader::FindOrCreate<DepthOnlyVS>(macros);
			auto ps = Shader::FindOrCreate<DepthOnlyPS>(macros);

			_view->BindShaderParams(vs);
			renderComponent->BindShaderParams(vs);

			_mat->BindDepthShaderParams(ps);

			ps->SetSampler("bilinearSampler",
				SamplerTemplate<
				FILTER_MIN_MAG_MIP_LINEAR,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP >::Get());

			vs->Flush();
			ps->Flush();

			meshElement->DrawDepth(bWithOpacityMask);
		}
	};

	void TranslucentRendering::Render(
		const Ptr<class RenderView> & view,
		const Ptr<class RenderTargetView> & target)
	{
		if (bOIT)
			OITRender(view, target);
		else
			ForwardRender(view, target);

		// Render depth
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
		//rc->ClearDepthStencil(1.0f, 0);

		rc->SetDepthStencilState(nullptr);
		rc->SetBlendState(nullptr);
		rc->SetRasterizerState(nullptr);

		rc->SetViewport(view->GetViewport());

		rc->SetRenderTargets({});

		auto drawingPolicy = std::make_shared<TranslucentDepthDrawingPolicy>();

		view->GetViewRenderContext()->primitiveDrawList->Draw(drawingPolicy, view);
	}

	void ForwardRenderObj(
		const Ptr<class RenderView> & view,
		const Ptr<class RenderTargetView> & target,
		const TranslucentRenderObj & obj)
	{
		auto targetTex = target->GetResource()->Cast<Texture>();
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto velocity = view->GetViewRenderContext()->GetSharedTexture("Velocity");

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		// Lighting
		auto lightingTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, targetTex->GetDesc() });
		auto lightingTex = lightingTexRef->Get()->Cast<Texture>();
		auto lightingTexBackRef = TexturePool::Instance().FindFree({ TEXTURE_2D, targetTex->GetDesc() });
		auto lightingTexBack = lightingTexBackRef->Get()->Cast<Texture>();

		if (view->GetViewRenderContext()->lights.size() > 0)
		{
			int32_t index = 0;
			for (auto & light : view->GetViewRenderContext()->lights)
			{
				std::map<String, String> macros;

				if (index == 0)
					macros["INIT"] = "";
				if (index == (int32_t)view->GetViewRenderContext()->lights.size() - 1)
					macros["AMBIENT"] = "";

				obj.mat->BindMacros(macros);
				obj.renderComponent->GetMeshElement()->BindMacros(macros);
				light->BindMacros(true, view, macros);

				auto vs = Shader::FindOrCreate<ForwardTranslucentRenderingVS>(macros);
				auto ps = Shader::FindOrCreate<ForwardTranslucentLightingPS>(macros);

				obj.mat->BindShaderParams(ps);

				obj.renderComponent->BindShaderParams(vs);
				obj.renderComponent->BindShaderParams(ps);

				view->BindShaderParams(vs);
				view->BindShaderParams(ps);

				light->BindShaderParams(ps, true, view);

				if (index == (int32_t)view->GetViewRenderContext()->lights.size() - 1)
					ps->SetScalar("ambientColor", Global::GetScene()->GetAmbientColor());

				if (index != 0)
					ps->SetSRV("lightingTex", lightingTex->GetShaderResourceView());

				ps->SetSampler("bilinearSampler", SamplerTemplate<>::Get());
				ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

				vs->Flush();
				ps->Flush();

				rc->SetViewport(GetTextureQuadViewport(targetTex));

				rc->SetRenderTargets({ lightingTexBack->GetRenderTargetView(0, 0, 1) });

				rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

				rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO, COMPARISON_LESS>::Get());

				obj.renderComponent->GetMeshElement()->Draw();

				rc->SetDepthStencilState(nullptr);

				std::swap(lightingTex, lightingTexBack);
				++index;
			}
		}
		else
		{
			std::map<String, String> macros;

			obj.mat->BindMacros(macros);
			obj.renderComponent->GetMeshElement()->BindMacros(macros);

			auto vs = Shader::FindOrCreate<ForwardTranslucentRenderingVS>(macros);
			auto ps = Shader::FindOrCreate<ForwardTranslucentAmbientPS>(macros);

			obj.mat->BindShaderParams(ps);

			obj.renderComponent->BindShaderParams(vs);
			obj.renderComponent->BindShaderParams(ps);

			view->BindShaderParams(vs);
			view->BindShaderParams(ps);

			ps->SetScalar("ambientColor", Global::GetScene()->GetAmbientColor());

			//ps->SetSRV("lightingTex", sceneTex->GetShaderResourceView());

			ps->SetSampler("bilinearSampler", SamplerTemplate<>::Get());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			vs->Flush();
			ps->Flush();

			rc->SetViewport(GetTextureQuadViewport(targetTex));

			rc->SetRenderTargets({ lightingTexBack->GetRenderTargetView(0, 0, 1) });

			rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

			rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO, COMPARISON_LESS>::Get());

			obj.renderComponent->GetMeshElement()->Draw();

			rc->SetDepthStencilState(nullptr);

			std::swap(lightingTex, lightingTexBack);
		}

		{
			std::map<String, String> macros;

			if (obj.mat->IsRefraction())
				macros["TRASLUCENT_REFRACTION"] = "";
			if (Global::GetRenderEngine()->GetSceneRenderer()->bGenVelocityMap)
				macros["GEN_VELOCITY"] = "";

			obj.mat->BindMacros(macros);
			obj.renderComponent->GetMeshElement()->BindMacros(macros);

			auto vs = Shader::FindOrCreate<ForwardTranslucentRenderingVS>(macros);
			auto ps = Shader::FindOrCreate<ForwardTranslucentRenderingPS>(macros);

			obj.mat->BindShaderParams(ps);

			obj.renderComponent->BindShaderParams(vs);
			obj.renderComponent->BindShaderParams(ps);

			view->BindShaderParams(vs);
			view->BindShaderParams(ps);

			ps->SetSRV("lightingTex", lightingTex->GetShaderResourceView());

			if (obj.mat->IsRefraction())
			{
				auto tmpSceneTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, targetTex->GetDesc() });
				auto tmpSceneTex = tmpSceneTexRef->Get()->Cast<Texture>();
				auto tmpDepthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneClipDepth->GetDesc() });
				auto tmpDepthTex = tmpDepthTexRef->Get()->Cast<Texture>();
				targetTex->CopyTo(tmpSceneTex, 0, 0, 0, 0, 0, 0, 0);
				sceneClipDepth->CopyTo(tmpDepthTex, 0, 0, 0, 0, 0, 0, 0);

				ps->SetSRV("sceneTex", tmpSceneTex->GetShaderResourceView());
				ps->SetSRV("depthTex", tmpDepthTex->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
				ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

				rc->SetBlendState(nullptr);
			}
			else
			{
				rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_SRC1_COLOR>::Get());
			}

			ps->SetSampler("bilinearSampler", SamplerTemplate<>::Get());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			vs->Flush();
			ps->Flush();

			rc->SetViewport(GetTextureQuadViewport(targetTex));

			if (obj.mat->IsRefraction())
			{
				if (Global::GetRenderEngine()->GetSceneRenderer()->bGenVelocityMap)
					rc->SetRenderTargets({ target, velocity->GetRenderTargetView(0, 0, 1) });
				else
					rc->SetRenderTargets({ target });
			}
			else
			{
				if (Global::GetRenderEngine()->GetSceneRenderer()->bGenVelocityMap)
					rc->SetRenderTargets({ target, nullptr, velocity->GetRenderTargetView(0, 0, 1) });
				else
					rc->SetRenderTargets({ target });
			}

			rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
			rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO>::Get());

			obj.renderComponent->GetMeshElement()->Draw();

			rc->SetDepthStencilState(nullptr);
			rc->SetBlendState(nullptr);
		}
		
	}

	void TranslucentRendering::ForwardRender(
		const Ptr<class RenderView> & view,
		const Ptr<class RenderTargetView> & target)
	{
		std::vector<TranslucentRenderObj> objs;

		for (auto & drawBatchPair : view->GetViewRenderContext()->primitiveDrawList->drawBatches)
		{
			TranslucentRenderObj obj;

			auto & mat = drawBatchPair.first;
			if (!mat->IsTranslucent())
				continue;

			obj.mat = mat;

			for (auto & drawComponent : drawBatchPair.second)
			{
				obj.renderComponent = drawComponent;
				objs.push_back(obj);
			}

		}

		auto & objsSorted = objs;
		std::sort(objsSorted.begin(), objsSorted.end(),
			[&](const TranslucentRenderObj & obj0, const TranslucentRenderObj & obj1) -> bool
		{
			auto viewDir = view->GetCamera()->GetZAxis();
			auto viewPos = view->GetCamera()->GetPos();

			auto pos0 = obj0.renderComponent->GetPos();
			auto vec0 = pos0 - viewPos;// XMFLOAT3(pos0.x - viewPos.x, pos0.y - viewPos.y, pos0.z - viewPos.z);
			auto dist0 = dot(vec0, viewDir);// vec0.x * viewDir.x + vec0.y * viewDir.y + vec0.z * viewDir.z;

			auto pos1 = obj0.renderComponent->GetPos();
			auto vec1 = pos1 - viewPos; // XMFLOAT3(pos1.x - viewPos.x, pos1.y - viewPos.y, pos1.z - viewPos.z);
			auto dist1 = dot(vec1, viewDir);//vec1.x * viewDir.x + vec1.y * viewDir.y + vec1.z * viewDir.z;

			return dist0 > dist1;
		});

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto targetTex = target->GetResource()->Cast<Texture>();
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		for (auto & obj : objsSorted)
		{
			ForwardRenderObj(view, target, obj);
		}
		rc->SetBlendState(nullptr);
		rc->SetDepthStencilState(nullptr);
	}


	void OITRenderObjs(
		const Ptr<class RenderView> & view,
		const Ptr<class RenderTargetView> & target,
		const std::vector<TranslucentRenderObj> & objs,
		Ptr<RenderBuffer>	oitCounter,
		Ptr<Texture>		oitListHead,
		Ptr<RenderBuffer>	oitListLink,
		Ptr<RenderBuffer>	oitTransmittanceList,
		Ptr<RenderBuffer>	oitLightingList,
		Ptr<RenderBuffer>	oitDepthList)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto targetTex = target->GetResource()->Cast<Texture>();
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto velocity = view->GetViewRenderContext()->GetSharedTexture("Velocity");

		/*auto lightingTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, targetTex->GetDesc() });
		auto lightingTex = lightingTexRef->Get()->Cast<Texture>();
		auto lightingTexBackRef = TexturePool::Instance().FindFree({ TEXTURE_2D, targetTex->GetDesc() });
		auto lightingTexBack = lightingTexBackRef->Get()->Cast<Texture>();*/

		int objIndex = 0;
		for (auto & obj : objs)
		{
			//// Lighting
			//if (view->GetViewRenderContext()->lights.size() > 0)
			//{
			//	int32_t index = 0;
			//	for (auto & light : view->GetViewRenderContext()->lights)
			//	{
			//		std::map<String, String> macros;

			//		if (index == 0)
			//			macros["INIT"] = "";
			//		if (index == (int32_t)view->GetViewRenderContext()->lights.size())
			//			macros["AMBIENT"] = "";

			//		obj.mat->BindMacros(macros);
			//		obj.renderComponent->GetMeshElement()->BindMacros(macros);
			//		light->BindMacros(true, view, macros);

			//		auto vs = Shader::FindOrCreate<OITRenderingVS>(macros);
			//		auto ps = Shader::FindOrCreate<OITLightingPS>(macros);

			//		obj.mat->BindShaderParams(ps);

			//		obj.renderComponent->BindShaderParams(vs);
			//		obj.renderComponent->BindShaderParams(ps);

			//		view->BindShaderParams(vs);
			//		view->BindShaderParams(ps);

			//		light->BindShaderParams(ps, true, view);

			//		if (index != 0)
			//			ps->SetSRV("lightingTex", lightingTex->GetShaderResourceView());
			//		if(index == (int32_t)view->GetViewRenderContext()->lights.size())
			//			ps->SetScalar("ambientColor", Global::GetScene()->GetAmbientColor());

			//		ps->SetSampler("bilinearSampler", 
			//			SamplerTemplate<
			//			FILTER_MIN_MAG_MIP_LINEAR,
			//			TEXTURE_ADDRESS_WRAP,
			//			TEXTURE_ADDRESS_WRAP,
			//			TEXTURE_ADDRESS_WRAP>::Get());
			//		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			//		vs->Flush();
			//		ps->Flush();

			//		rc->SetViewport(GetTextureQuadViewport(targetTex));

			//		rc->SetRenderTargets({ lightingTex->GetRenderTargetView(0, 0, 1) });
			//		rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

			//		rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO>::Get());
			//		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE>::Get());

			//		obj.renderComponent->GetMeshElement()->Draw();

			//		rc->SetDepthStencilState(nullptr);
			//		rc->SetBlendState(nullptr);

			//		//std::swap(lightingTex, lightingTexBack);
			//		++index;
			//	}
			//}
			//else
			//{
			//	std::map<String, String> macros;

			//	obj.mat->BindMacros(macros);
			//	obj.renderComponent->GetMeshElement()->BindMacros(macros);

			//	auto vs = Shader::FindOrCreate<OITRenderingVS>(macros);
			//	auto ps = Shader::FindOrCreate<OITAmbientPS>(macros);

			//	obj.mat->BindShaderParams(ps);

			//	obj.renderComponent->BindShaderParams(vs);
			//	obj.renderComponent->BindShaderParams(ps);

			//	view->BindShaderParams(vs);
			//	view->BindShaderParams(ps);

			//	ps->SetScalar("ambientColor", Global::GetScene()->GetAmbientColor());

			//	ps->SetSampler("bilinearSampler", 
			//		SamplerTemplate<
			//		FILTER_MIN_MAG_MIP_LINEAR,
			//		TEXTURE_ADDRESS_WRAP,
			//		TEXTURE_ADDRESS_WRAP,
			//		TEXTURE_ADDRESS_WRAP>::Get());

			//	vs->Flush();
			//	ps->Flush();

			//	rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE>::Get());

			//	rc->SetViewport(GetTextureQuadViewport(targetTex));

			//	rc->SetRenderTargets({ lightingTex->GetRenderTargetView(0, 0, 1) });
			//	rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

			//	rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO>::Get());

			//	obj.renderComponent->GetMeshElement()->Draw();

			//	rc->SetDepthStencilState(nullptr);
			//	rc->SetBlendState(nullptr);

			//	//std::swap(lightingTex, lightingTexBack);
			//}

			{
				Ptr<LightComponent> light;
				if (view->GetViewRenderContext()->lights.size() > 0)
					light = view->GetViewRenderContext()->lights[0];

				std::map<String, String> macros;

				if (Global::GetRenderEngine()->GetSceneRenderer()->bGenVelocityMap)
					macros["GEN_VELOCITY"] = "";

				if(light)
					light->BindMacros(true, view, macros);

				obj.mat->BindMacros(macros);
				obj.renderComponent->GetMeshElement()->BindMacros(macros);

				auto vs = Shader::FindOrCreate<OITRenderingVS>(macros);
				auto ps = Shader::FindOrCreate<OITRenderingPS>(macros);

				ps->SetScalar("ambientColor", Global::GetScene()->GetAmbientColor());

				if (light)
					light->BindShaderParams(ps, true, view);

				obj.mat->BindShaderParams(ps);

				obj.renderComponent->BindShaderParams(vs);
				obj.renderComponent->BindShaderParams(ps);

				view->BindShaderParams(vs);
				view->BindShaderParams(ps);

				//ps->SetSRV("lightingTex", lightingTex->GetShaderResourceView());
				if (objIndex == 0)
					ps->SetUAV("oitCounter", oitCounter->GetUnorderedAccessView(0, 0, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_COUNTER), 0);
				else
					ps->SetUAV("oitCounter", oitCounter->GetUnorderedAccessView(0, 0, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_COUNTER), -1);
				ps->SetUAV("oitListHead", oitListHead->GetUnorderedAccessView(0, 0, 0, RENDER_FORMAT_R32_SINT));
				ps->SetUAV("oitListLink", oitListLink->GetUnorderedAccessView(0, 0, RENDER_FORMAT_R32_SINT, 0));
				ps->SetUAV("oitTransmittanceList", oitTransmittanceList->GetUnorderedAccessView(0, 0, RENDER_FORMAT_R11G11B10_FLOAT, 0));
				ps->SetUAV("oitLightingList", oitLightingList->GetUnorderedAccessView(0, 0, RENDER_FORMAT_R11G11B10_FLOAT, 0));
				ps->SetUAV("oitDepthList", oitDepthList->GetUnorderedAccessView(0, 0, RENDER_FORMAT_R32_FLOAT, 0));

				ps->SetSampler("bilinearSampler", 
					SamplerTemplate<
					FILTER_MIN_MAG_MIP_LINEAR,
					TEXTURE_ADDRESS_WRAP,
					TEXTURE_ADDRESS_WRAP,
					TEXTURE_ADDRESS_WRAP>::Get());

				vs->Flush();
				ps->Flush();

				rc->SetViewport(GetTextureQuadViewport(targetTex));

				if (Global::GetRenderEngine()->GetSceneRenderer()->bGenVelocityMap)
					rc->SetRenderTargets({ velocity->GetRenderTargetView(0, 0, 1) });
				else
					rc->SetRenderTargets({});

				rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
				rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO>::Get());

				obj.renderComponent->GetMeshElement()->Draw();

				rc->SetDepthStencilState(nullptr);
			}

			++objIndex;
		}

		auto texDesc = targetTex->GetDesc();
		auto backgroundTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto backgroundTex = backgroundTexRef->Get()->Cast<Texture>();
		targetTex->CopyTo(backgroundTex, 0, 0, 0, 0, 0, 0, 0);

		// Merge
		auto cs = Shader::FindOrCreate<OITRenderingMergeCS>();

		cs->SetSRV("backgroundTex", backgroundTex->GetShaderResourceView());
		cs->SetSRV("oitListHead", oitListHead->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R32_SINT));
		cs->SetSRV("oitListLink", oitListLink->GetShaderResourceView(0, 0, RENDER_FORMAT_R32_SINT));
		cs->SetSRV("oitTransmittanceList", oitTransmittanceList->GetShaderResourceView(0, 0, RENDER_FORMAT_R11G11B10_FLOAT));
		cs->SetSRV("oitLightingList", oitLightingList->GetShaderResourceView(0, 0, RENDER_FORMAT_R11G11B10_FLOAT));
		cs->SetSRV("oitDepthList", oitDepthList->GetShaderResourceView(0, 0, RENDER_FORMAT_R32_FLOAT));

		cs->SetUAV("resultTex", targetTex->GetUnorderedAccessView(0, 0, 0));

		cs->Flush();

		int32_t groupX = (targetTex->GetDesc().width + 15) / 16;
		int32_t groupY = (targetTex->GetDesc().height + 15) / 16;
		int32_t groupZ = 1;
		rc->Compute(groupX, groupY, groupZ);

		rc->ResetShader(SHADER_CS);
	}

	void TranslucentRendering::OITRender(
		const Ptr<class RenderView> & view,
		const Ptr<class RenderTargetView> & target)
	{
		std::vector<TranslucentRenderObj> objs;

		for (auto & drawBatchPair : view->GetViewRenderContext()->primitiveDrawList->drawBatches)
		{
			TranslucentRenderObj obj;

			auto & mat = drawBatchPair.first;
			if (!mat->IsTranslucent())
				continue;

			obj.mat = mat;

			for (auto & drawComponent : drawBatchPair.second)
			{
				obj.renderComponent = drawComponent;
				objs.push_back(obj);
			}
		}

		auto & objsSorted = objs;
		/*std::sort(objsSorted.begin(), objsSorted.end(),
			[&](const TranslucentRenderObj & obj0, const TranslucentRenderObj & obj1) -> bool
		{
			auto viewDir = view->GetCamera()->GetZAxis();
			auto viewPos = view->GetCamera()->GetPos();

			auto pos0 = obj0.renderComponent->GetPos();
			auto vec0 = XMFLOAT3(pos0.x - viewPos.x, pos0.y - viewPos.y, pos0.z - viewPos.z);
			auto dist0 = vec0.x * viewDir.x + vec0.y * viewDir.y + vec0.z * viewDir.z;

			auto pos1 = obj0.renderComponent->GetPos();
			auto vec1 = XMFLOAT3(pos1.x - viewPos.x, pos1.y - viewPos.y, pos1.z - viewPos.z);
			auto dist1 = vec1.x * viewDir.x + vec1.y * viewDir.y + vec1.z * viewDir.z;

			return dist0 > dist1;
		});*/


		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto targetTex = target->GetResource()->Cast<Texture>();
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		// Init resources
		Ptr<RenderBuffer>	oitCounter;
		Ptr<Texture>		oitListHead;
		Ptr<RenderBuffer>	oitListLink;
		Ptr<RenderBuffer>	oitTransmittanceList;
		Ptr<RenderBuffer>	oitLightingList;
		Ptr<RenderBuffer>	oitDepthList;

		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_UNORDERED_ACCESS;
		bufDesc.bStructured = true;
		bufDesc.cpuAccess = 0;
		bufDesc.elementSize = (int32_t)sizeof(float);
		bufDesc.numElements = 1;

		auto oitCounterRef = BufferPool::Instance().FindFree(bufDesc);
		oitCounter = oitCounterRef->Get()->Cast<RenderBuffer>();

		int32_t maxLayers = 3;
		int32_t bufferSize = targetTex->GetDesc().width * targetTex->GetDesc().height * maxLayers;

		bufDesc.bStructured = false;
		bufDesc.bindFlag = BUFFER_BIND_UNORDERED_ACCESS | BUFFER_BIND_SHADER_RESOURCE;
		bufDesc.numElements = bufferSize;
		bufDesc.elementSize = (int32_t)sizeof(int32_t);
		auto oitListLinkRef = BufferPool::Instance().FindFree(bufDesc);
		oitListLink = oitListLinkRef->Get()->Cast<RenderBuffer>();

		bufDesc.elementSize = 4; //r11g11b10
		auto oitTransmittanceListRef = BufferPool::Instance().FindFree(bufDesc);
		oitTransmittanceList = oitTransmittanceListRef->Get()->Cast<RenderBuffer>();

		bufDesc.elementSize = 4; //r11g11b10
		auto oitLightingListRef = BufferPool::Instance().FindFree(bufDesc);
		oitLightingList = oitLightingListRef->Get()->Cast<RenderBuffer>();

		bufDesc.elementSize = (int32_t)sizeof(float);
		auto oitDepthListRef = BufferPool::Instance().FindFree(bufDesc);
		oitDepthList = oitDepthListRef->Get()->Cast<RenderBuffer>();

		TextureDesc texDesc = targetTex->GetDesc();
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.format = RENDER_FORMAT_R32_SINT;
		auto oitListHeadRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		oitListHead = oitListHeadRef->Get()->Cast<Texture>();
		rc->ClearRenderTarget(oitListHead->GetRenderTargetView(0, 0, 1), -1.0f);

		std::vector<TranslucentRenderObj> oitBatch;
		for (auto & obj : objs)
		{
			if (obj.mat->IsRefraction())
			{
				if (oitBatch.size() > 0)
				{
					OITRenderObjs(
						view,
						target,
						oitBatch,
						oitCounter,
						oitListHead,
						oitListLink,
						oitTransmittanceList,
						oitLightingList,
						oitDepthList);

					oitBatch.clear();
				}

				ForwardRenderObj(view, target, obj);
			}
			else
				oitBatch.push_back(obj);
		}
		if (oitBatch.size() > 0)
		{
			OITRenderObjs(
				view,
				target,
				oitBatch,
				oitCounter,
				oitListHead,
				oitListLink,
				oitTransmittanceList,
				oitLightingList,
				oitDepthList);
		}
	}
}