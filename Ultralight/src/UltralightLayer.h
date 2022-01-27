#include <Acorn.h>

namespace Acorn
{
	class UltralightLayer : public Layer
	{
		UltralightLayer();
		virtual ~UltralightLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender(Timestep t) override;
		virtual void OnEvent(Event& e) override;
	};
}