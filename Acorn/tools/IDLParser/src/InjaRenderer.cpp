#include "InjaRenderer.h"

namespace Acorn::IDL
{
	InjaTemplate::InjaTemplate(inja::Environment& env, std::string templateText)
	{
		auto tpl = m_TemplateCache.find(templateText);
		if (tpl == m_TemplateCache.end())
		{
			auto createdTpl				  = env.parse_template(templateText);
			m_TemplateCache[templateText] = createdTpl;
			m_Template					  = createdTpl;
		}
		else
		{
			m_Template = tpl->second;
		}
	}

	const inja::Template& InjaTemplate::GetTemplate() const
	{
		return m_Template;
	}

	InjaRenderer::InjaRenderer() {}

	void InjaRenderer::Reset()
	{
		// m_Environment	= inja::Environment();
		m_IsInvalidated = false;
	}

	std::string InjaRenderer::ToText()
	{
		return "";
	}

	void InjaRenderer::Render(const std::string& caller, const InjaTemplate& tpl, const std::unordered_map<std::string, std::string>& templateVars)
	{
		m_CallerStack.emplace_back(caller);

		try
		{
			auto injaTemplate  = tpl.GetTemplate();
			std::string result = m_Environment.render(injaTemplate, templateVars);
		}
		catch (const std::exception& e)
		{
			// NOTE IDK if inja actually throws an exception or not
			std::cout << "Exception: " << e.what() << std::endl;
			throw;
		}
		catch (...)
		{
			m_CallerStack.pop_back();
			throw;
		}
	}
} // namespace Acorn::IDL