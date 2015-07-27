#pragma once
#ifndef RENDEREFFECT_H
#define RENDEREFFECT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\Kernel\IOHelper.h"
#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderPass.h"

namespace ToyGE
{
	class ShaderProgram;
	class File;

	class RenderEffectCompileData
	{
	public:
		std::vector<MacroDesc> macros;
		uint64_t macroHashKey;
		std::map<String, Ptr<ShaderProgram>> compileDataMap; //ShaderEntry -> CompileData
		std::map<String, Ptr<RenderEffectVariable>> macroRelevantVarsMap;
		bool bInit;
		RenderEffectCompileData()
			: bInit(false)
		{
		}
	};

	class TOYGE_CORE_API RenderEffect : public std::enable_shared_from_this<RenderEffect>
	{
	public:
		static Ptr<RenderEffect> Load(const Ptr<File> & file);

		int32_t NumTechniques() const
		{
			return static_cast<int32_t>(_techniques.size());
		}

		void AddTechnique(const Ptr<RenderTechnique> & tech)
		{
			if (tech)
				_techniques.push_back(tech);
		}

		Ptr<RenderTechnique> TechniqueByIndex(int32_t index) const
		{
			return _techniques[index];
		}

		Ptr<RenderTechnique> TechniqueByName(const String & name) const;

		Ptr<RenderEffectVariable> VariableByName(const String & name) const;

		void SetExtraMacros(const std::vector<MacroDesc> & macros);

		void AddExtraMacro(const String & name, const String & value);

		void RemoveExtraMacro(const String & name);

		void UpdateData();

		const std::vector<MacroDesc> & GetExtraMacros() const
		{
			return _extraMacros;
		}

		const MacroDesc & MacroByName(const String & name) const;

		WString GetCompiledDataSavePath();

		void Save();

	private:
		WString _effectName;
		String _effectCode;
		std::map<String, Ptr<RenderEffectVariable>> _variableMap;
		std::map<String, MacroDesc> _macrosMap;
		std::vector<MacroDesc> _effectMacros;
		std::vector<MacroDesc> _extraMacros;
		uint64_t _macrosHashKey;
		std::map<uint64_t, RenderEffectCompileData> _effectCompileMap;
		std::vector<Ptr<RenderTechnique>> _techniques;

		void InitCompileData();

		void InitPassShaderPrograms();

		void SaveCompiledData(const Ptr<Writer> & writer);

		void LoadCompiledData(const Ptr<Reader> & reader);
	};
}

#endif