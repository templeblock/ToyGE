#pragma once
#ifndef RENDERSHAREDENVIROMENT_H
#define RENDERSHAREDENVIROMENT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\Texture.h"

namespace ToyGE
{
	class RenderView;

	class TOYGE_CORE_API CommonRenderShareName
	{
	public:
		static String GBuffer(int32_t index);

		static String Lighting(int32_t index);

		static String RawDepth();

		static String LinearDepth();

		static String Velocity();
	};

	class SharedParamBase;

	class TOYGE_CORE_API RenderSharedEnviroment
	{
	public:
		void SetParam(const String & name, const std::shared_ptr<SharedParamBase> & param)
		{
			_sharedParams[name] = param;
		}

		std::shared_ptr<SharedParamBase> ParamByName(const String & name) const
		{
			auto paramFind = _sharedParams.find(name);
			if (paramFind != _sharedParams.end())
				return paramFind->second;
			else
				return nullptr;
		}

		void SetView(const Ptr<RenderView> & view)
		{
			_view = view;
		}

		Ptr<RenderView> GetView() const
		{
			return _view.lock();
		}

		void Clear();

		

	private:
		std::map<String, std::shared_ptr<SharedParamBase>> _sharedParams;
		std::weak_ptr<RenderView> _view;
	};

	class TOYGE_CORE_API SharedParamBase : public std::enable_shared_from_this<SharedParamBase>
	{
	public:
		template <class T>
		std::shared_ptr<T> As()
		{
			return std::static_pointer_cast<T>(shared_from_this());
		}

		virtual void Release()
		{
		}
	};

	template <typename T>
	class TOYGE_CORE_API SharedParam : public SharedParamBase
	{
	public:
		SharedParam(const T & value)
			: _value(value)
		{
		}

		CLASS_GET(Value, T, _value);
		CLASS_SET(Value, T, _value);

	protected:
		T _value;
	};

	template <>
	class TOYGE_CORE_API SharedParam<Ptr<Texture>> : public SharedParamBase
	{
	public:
		SharedParam(const Ptr<Texture> & value)
			: _value(value)
		{
		}

		CLASS_GET(Value, Ptr<Texture>, _value);
		CLASS_SET(Value, Ptr<Texture>, _value);

		void Release() override
		{
			if (_value)
			{
				_value->Release();
				_value = nullptr;
			}
		}

	protected:
		Ptr<Texture> _value;
	};
}

#endif
