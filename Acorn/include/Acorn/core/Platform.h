#pragma once

#include "core/Timestep.h"
#include "renderer/RendererApi.h"

namespace Acorn
{
	class Platform
	{
	public:
		inline static float GetTime() { return s_Platform->GetTimeImpl(); }
		inline static const char* GetName() { return s_Platform->GetNameImpl(); }
		inline static void* GetCurrentContext() { return s_Platform->GetCurrentContextImpl(); }

		inline void* GetNativePlatform() { return (void*)s_Platform; }

	protected:
		virtual float GetTimeImpl() const = 0;
		virtual const char* GetNameImpl() const = 0;
		virtual void* GetCurrentContextImpl() = 0;

	private:
		static Platform* s_Platform;
	};
}