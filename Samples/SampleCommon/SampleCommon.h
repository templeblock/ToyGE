#pragma once
#ifndef SAMPLECOMMON_H
#define SAMPLECOMMON_H

#include "ToyGE\ToyGE.h"
#include "ToyGE\RenderEngine\Effects\TweakBarRenderer.h"

class SharedParamRender : public ToyGE::RenderAction
{
public:
	SharedParamRender();

	void Render(const ToyGE::Ptr<ToyGE::RenderSharedEnviroment> & sharedEnviroment) override;

	CLASS_GET(RenderParam, ToyGE::String, _renderParam);
	CLASS_SET(RenderParam, ToyGE::String, _renderParam);

	CLASS_GET(RenderParamColorWrite, ToyGE::Vector4<ToyGE::ColorWriteMask>, _renderParamColorWrite);
	CLASS_SET(RenderParamColorWrite, ToyGE::Vector4<ToyGE::ColorWriteMask>, _renderParamColorWrite);

	CLASS_GET(RenderParamAsNormal, bool, _renderParamAsNormal);
	CLASS_SET(RenderParamAsNormal, bool, _renderParamAsNormal);

private:
	ToyGE::String _renderParam;
	ToyGE::Vector4<ToyGE::ColorWriteMask> _renderParamColorWrite;
	bool _renderParamAsNormal;
};

class SampleCommon : public ToyGE::App, public std::enable_shared_from_this<SampleCommon>
{
public:
	static void Start(const ToyGE::Ptr<App> & app);

	SampleCommon();

	virtual ~SampleCommon() = default;

	virtual void Startup() override;

	virtual void Update(float elapsedTime) override;

	virtual void Destroy() override{};

protected:
	ToyGE::WString _sampleName;
	TwBar * _twBar;
	float _cameraMoveSpeed;
	float _cameraRotateSpeed;
	bool _bMouseDown;
	ToyGE::Ptr<ToyGE::RenderView> _renderView;

	void MouseDown(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, const ToyGE::MouseButton button);

	void MouseUp(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, const ToyGE::MouseButton button);

	void MouseMove(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, int relativeX, int relativeY);

	bool IsPosInWindow(const ToyGE::int2 & pos) const;
};


#endif