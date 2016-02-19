#pragma once
#ifndef SAMPLECOMMON_H
#define SAMPLECOMMON_H

#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\MeshAsset.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\Input\InputEngine.h"
#include "ToyGE\RenderEngine\RenderEngineInclude.h"
#include "ToyGE\RenderEngine\Effects\Effects.h"

class SharedParamRender : public ToyGE::RenderAction
{
public:
	bool bDecode = false;

	SharedParamRender();

	void Render(const ToyGE::Ptr<ToyGE::RenderView> & view) override;

	CLASS_GET(RenderParam, ToyGE::String, _renderParam);
	CLASS_SET(RenderParam, ToyGE::String, _renderParam);

	const ToyGE::Vector<ToyGE::ColorWriteMask, 4> & GetRenderParamColorWrite() const
	{
		return _renderParamColorWrite;
	}
	void SetRenderParamColorWrite(const ToyGE::Vector<ToyGE::ColorWriteMask, 4> & v)
	{
		_renderParamColorWrite = v;
	}

	CLASS_GET(RenderParamAsNormal, bool, _renderParamAsNormal);
	CLASS_SET(RenderParamAsNormal, bool, _renderParamAsNormal);

	CLASS_GET(ViewFormat, ToyGE::RenderFormat, _format);
	CLASS_SET(ViewFormat, ToyGE::RenderFormat, _format);

private:
	ToyGE::String _renderParam;
	ToyGE::Vector<ToyGE::ColorWriteMask, 4> _renderParamColorWrite;
	bool _renderParamAsNormal = false;
	ToyGE::RenderFormat _format = ToyGE::RenderFormat::RENDER_FORMAT_UNDEFINED;
};

class SampleCommon : public ToyGE::App, public std::enable_shared_from_this<SampleCommon>
{
public:
	static void Start(const ToyGE::Ptr<App> & app);

	SampleCommon();

	virtual ~SampleCommon() = default;

	virtual void Init() override;

	virtual void Update(float elapsedTime) override;

	virtual void Destroy() override{};

protected:
	ToyGE::String _sampleName;
	TwBar * _twBar;
	float _cameraMoveSpeed;
	float _cameraRotateSpeed;
	bool _bMouseDown;
	ToyGE::Ptr<ToyGE::RenderView> _renderView;

	void MouseDown(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, const ToyGE::MouseButton button);

	void MouseUp(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, const ToyGE::MouseButton button);

	void MouseMove(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, int relativeX, int relativeY);

	void KeyUp(const ToyGE::Ptr<ToyGE::InputKeyboard> & keyboard, ToyGE::KeyCode code);

	bool IsPosInWindow(const ToyGE::int2 & pos) const;
};


#endif