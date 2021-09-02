#pragma once

#include "core/Core.h"
#include "utils/PlatformCapabilities.h"

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