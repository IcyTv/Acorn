#pragma once

#include "core/Core.h"

namespace Acorn
{
	class PlatformCapabilities
	{
	public:
		static uint32_t GetMaxTextureUnits()
		{
			return s_Instance->GetMaxTextureUnits_();
		}

		static void Init();

	protected:
		virtual uint32_t GetMaxTextureUnits_() = 0;

	private:
		static PlatformCapabilities* s_Instance;
	};

}