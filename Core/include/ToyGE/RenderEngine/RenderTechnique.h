#ifndef RENDERTECHNIQUE_H
#define RENDERTECHNIQUE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

#include "rapidxml.hpp"

namespace ToyGE
{
	class RenderPass;

	class TOYGE_CORE_API RenderTechnique : public std::enable_shared_from_this<RenderTechnique>
	{
		friend class RenderEffect;

	public:
		static Ptr<RenderTechnique> Load(const rapidxml::xml_node<> * node, const Ptr<RenderEffect> & effect);

		const std::weak_ptr<RenderEffect> & Effect() const
		{
			return _effect;
		}

		void SetName(const String & name)
		{
			_name = name;
		}

		const String & Name() const
		{
			return _name;
		}

		void AddPass(const Ptr<RenderPass> & pass)
		{
			if (pass)
				_pass.push_back(pass);
		}

		int32_t NumPasses() const
		{
			return static_cast<int32_t>(_pass.size());
		}

		Ptr<RenderPass> PassByIndex(int32_t index) const
		{
			return _pass[index];
		}

		Ptr<RenderPass> PassByName(const String & name) const;

	private:
		std::weak_ptr<RenderEffect> _effect;

		String _name;
		std::vector<Ptr<RenderPass>> _pass;
	};
}

#endif