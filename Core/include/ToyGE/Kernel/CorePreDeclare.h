#pragma once 
#ifndef COREPREDECLARE_H
#define COREPREDECLARE_H

//#include "ToyGE\Kernel\PreIncludes.h"

#ifdef TOYGE_CORE_SOURCE
#define TOYGE_CORE_API _declspec(dllexport)
#else
#define TOYGE_CORE_API _declspec(dllimport)
#endif

//namespace ToyGE
//{
//	//Kernel
//	class EngineDriver;
//	class Global;
//	class File;
//	class Image;
//	class App;
//	class Mesh;
//	class Material;
//	class Model;
//	class FontFactory;
//	class Font;
//	class FontRenderer;
//	class ResourceManagerBase;
//	class Reader;
//	class Writer;
//
//	//Input
//	class InputDevice;
//	class InputEngine;
//	class InputKeyboard;
//	class InputMouse;
//
//	//Platform
//	class DllLoader;
//	class DllObj;
//	class Looper;
//	class PlatformFactory;
//	class Window;
//	class WindowsDllLoader;
//	class WindowsDllObj;
//	class WindowsLooper;
//	class WindowsPlatformFactory;
//	class WindowsWindow;
//
//	//Scene
//	class Scene;
//	using Ptr<Scene> = std::shared_ptr < Scene > ;
//	class SceneObject;
//	class SceneObjectComponent;
//	class TranformComponent;
//	class RenderComponent;
//	class SceneCuller;
//	class Cullable;
//
//	//RenderEngine
//	class RenderEngine;
//	class RenderContext;
//	class RenderFactory;
//	class RenderFramework;
//	class Shader;
//	using Ptr<Shader> = std::shared_ptr < Shader > ;
//	class ShaderProgram;
//	using Ptr<ShaderProgram> = std::shared_ptr < ShaderProgram >;
//	class RenderView;
//	using Ptr<RenderView> = std::shared_ptr < RenderView >;
//	class RenderResource;
//	using Ptr<RenderResource> = std::shared_ptr < RenderResource > ;
//	class Texture;
//	using Ptr<Texture> = std::shared_ptr < Texture > ;
//	class RenderBuffer;
//	using Ptr<RenderBuffer> = std::shared_ptr < RenderBuffer > ;
//	class BlendState;
//	using Ptr<BlendState> = std::shared_ptr < BlendState > ;
//	class DepthStencilState;
//	using Ptr<DepthStencilState> = std::shared_ptr < DepthStencilState > ;
//	class RasterizerState;
//	using Ptr<RasterizerState> = std::shared_ptr < RasterizerState > ;
//	class Sampler;
//	using Ptr<Sampler> = std::shared_ptr < Sampler > ;
//	class RenderInput;
//	using Ptr<RenderInput> = std::shared_ptr < RenderInput > ;
//	class Camera;
//	using Ptr<Camera> = std::shared_ptr < Camera > ;
//	class RenderAction;
//	using Ptr<RenderAction> = std::shared_ptr < RenderAction > ;
//	class LightComponent;
//	using Ptr<LightComponent> = std::shared_ptr < LightComponent >;
//	class PointLightComponent;
//	using Ptr<PointLightComponent> = std::shared_ptr < PointLightComponent >;
//	class SpotLightComponent;
//	using Ptr<SpotLightComponent> = std::shared_ptr < SpotLightComponent >;
//	class DirectionalLightComponent;
//	using Ptr<DirectionalLightComponent> = std::shared_ptr < DirectionalLightComponent >;
//	class RenderEffect;
//	using Ptr<RenderEffect> = std::shared_ptr < RenderEffect > ;
//	class RenderTechnique;
//	using Ptr<RenderTechnique> = std::shared_ptr < RenderTechnique > ;
//	class RenderPass;
//	using Ptr<RenderPass> = std::shared_ptr < RenderPass > ;
//	class RenderEffectVariable;
//	using Ptr<RenderEffectVariable> = std::shared_ptr < RenderEffectVariable > ;
//	class RenderEffectVariable_ShaderResource;
//	using RenderEffectVariable_ShaderResourcePtr = std::shared_ptr < RenderEffectVariable_ShaderResource >;
//	class RenderEffectVariable_UAV;
//	using Ptr<RenderEffectVariable_UAV> = std::shared_ptr < RenderEffectVariable_UAV > ;
//	class RenderEffectVariable_Sampler;
//	using RenderEffectVariable_Ptr<Sampler> = std::shared_ptr < RenderEffectVariable_Sampler >;
//	class RenderEffectVariable_ConstantBuffer;
//	using RenderEffectVariable_ConstantBuffer = std::shared_ptr < RenderEffectVariable_ConstantBuffer > ;
//	class RenderEffectVariable_Scalar;
//	using Ptr<RenderEffectVariable_Scalar> = std::shared_ptr < RenderEffectVariable_Scalar > ;
//	class RenderEffectVariable_BlendState;
//	using RenderEffectVariable_Ptr<BlendState> = std::shared_ptr < RenderEffectVariable_BlendState > ;
//	class RenderEffectVariable_DepthStencilState;
//	using RenderEffectVariable_Ptr<DepthStencilState> = std::shared_ptr < RenderEffectVariable_DepthStencilState > ;
//	class RenderEffectVariable_RasterizerState;
//	using RenderEffectVariable_Ptr<RasterizerState> = std::shared_ptr < RenderEffectVariable_RasterizerState > ;
//	class Label;
//	using LabelPtr = std::shared_ptr < Label > ;
//	class DebugInfoRender;
//	using Ptr<DebugInfoRender> = std::shared_ptr < DebugInfoRender > ;
//	class RenderSharedEnviroment;
//	using Ptr<RenderSharedEnviroment> = std::shared_ptr < RenderSharedEnviroment > ;
//	//class SharedParam;
//	//using SharedParamPtr = std::shared_ptr < SharedParam > ;
//	class ShadowTechnique;
//	using Ptr<ShadowTechnique> = std::shared_ptr < ShadowTechnique > ;
//	class ShadowDepthTechnique;
//	using Ptr<ShadowDepthTechnique> = std::shared_ptr < ShadowDepthTechnique >;
//	class ShadowRenderTechnique;
//	using Ptr<ShadowRenderTechnique> = std::shared_ptr < ShadowRenderTechnique >;
//	class PostProcess;
//	using PostProcessPtr = std::shared_ptr < PostProcess > ;
//	class ReflectionMap;
//	using Ptr<ReflectionMap> = std::shared_ptr < ReflectionMap > ;
//	class RenderConfig;
//	using Ptr<RenderConfig> = std::shared_ptr < RenderConfig > ;
//	class TransientBuffer;
//	using Ptr<TransientBuffer> = std::shared_ptr < TransientBuffer > ;
//	class Widget;
//	using Ptr<Widget> = std::shared_ptr < Widget > ;
//	class WidgetContainer;
//	using Ptr<WidgetContainer> = std::shared_ptr < WidgetContainer >;
//	class SceneRenderer;
//	using Ptr<SceneRenderer> = std::shared_ptr < SceneRenderer > ;
//}

#endif