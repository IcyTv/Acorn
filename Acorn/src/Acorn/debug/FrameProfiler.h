#pragma once

#include "core/Core.h"
#include "renderer/RendererApi.h"

namespace Acorn
{
	class FrameProfiler
	{
	public:
		virtual ~FrameProfiler() = default;
		static Ref<FrameProfiler> Create();

		virtual void SendFrame(int width, int height) = 0;

	private:
	};
}