#pragma once

#include <inja/inja.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace Acorn::IDL
{
	class InjaTemplate
	{
public:
		InjaTemplate(inja::Environment& env, std::string templateText = "");

		const inja::Template& GetTemplate() const;

private:
		inja::Template m_Template;

		std::unordered_map<std::string, inja::Template> m_TemplateCache;
	};

	class InjaRenderer
	{
		InjaRenderer();

		void Reset();
		inline void Invalidate()
		{
			m_IsInvalidated = true;
		}

		std::string ToText();

		void Render(const std::string& caller, const InjaTemplate& tpl, const inja::json& templateVars = {});

private:
		inja::Environment m_Environment;
		bool m_IsInvalidated{ false };

		std::vector<std::string> m_CallerStack;
	};
} // namespace Acorn::IDL