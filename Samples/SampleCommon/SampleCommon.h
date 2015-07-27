#pragma once
#ifndef SAMPLECOMMON_H
#define SAMPLECOMMON_H

#include "ToyGE\ToyGE.h"

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