#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderPass.h"

namespace ToyGE
{
	Ptr<RenderTechnique> RenderTechnique::Load(const rapidxml::xml_node<> * node, const Ptr<RenderEffect> & effect)
	{
		if (nullptr == node)
			return Ptr<RenderTechnique>();

		auto technique = std::make_shared<RenderTechnique>();
		technique->_effect = effect;
		auto nameAttri = node->first_attribute("name");
		if (nameAttri)
			technique->_name = nameAttri->value();

		auto passNode = node->first_node("pass");
		while (passNode)
		{
			auto pass = RenderPass::Load(passNode, technique);
			if (pass)
			{
				technique->_pass.push_back(pass);
			}

			passNode = passNode->next_sibling("pass");
		}

		return technique;
	}

	Ptr<RenderPass> RenderTechnique::PassByName(const String & name) const
	{
		for (auto & pass : _pass)
		{
			if (pass->Name() == name)
				return pass;
		}

		return Ptr<RenderPass>();
	}
}