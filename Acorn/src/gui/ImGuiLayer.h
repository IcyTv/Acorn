#pragma once

#include "layer/Layer.h"

#include "events/ApplicationEvent.h"
#include "events/Event.h"
#include "events/KeyEvent.h"
#include "events/MouseEvent.h"

#include "utils/FixedQueue.h"

constexpr uint32_t MAX_FRAME_TIMES = 512;

namespace Acorn
{
	class AC_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender(Timestep timestep) override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool val) { m_BlockEvents = val; }

	private:
		void SetDarkThemeColors();

		bool OnKeyDown(KeyEvent& ev);

	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};
}
