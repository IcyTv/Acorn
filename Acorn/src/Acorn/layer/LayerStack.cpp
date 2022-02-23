#include "acpch.h"

#include "layer/LayerStack.h"

namespace Acorn
{

	LayerStack::LayerStack()
	{
		AC_PROFILE_FUNCTION();
	}

	LayerStack::~LayerStack()
	{
		AC_PROFILE_FUNCTION();

		AC_CORE_INFO("Destroying Layer Stack");

		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		AC_PROFILE_FUNCTION();

		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		AC_PROFILE_FUNCTION();

		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		AC_PROFILE_FUNCTION();

		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
		else
		{
			AC_CORE_WARN("Tried to pop a layer that was not in the layer stack!");
		}
		layer->OnDetach();
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		AC_PROFILE_FUNCTION();

		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);

		if (it != m_Layers.end())
			m_Layers.erase(it);
		else
			AC_CORE_WARN("Tried to pop an overlay that was not in the layer stack!");

		overlay->OnDetach();
	}

}
