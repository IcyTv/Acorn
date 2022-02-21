#pragma once

#include "Acorn/core/Core.h"
#include "Acorn/utils/PlatformCapabilities.h"

namespace Acorn
{
	class OpenGLPlatformCapabilities : public PlatformCapabilities
	{
	public:
		OpenGLPlatformCapabilities() = default;
		virtual ~OpenGLPlatformCapabilities() = default;

	protected:
		virtual uint32_t GetMaxTextureUnits_() override;
	};
}