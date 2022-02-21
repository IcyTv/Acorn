#pragma once

#include "core/Core.h"

#include "debug/FrameProfiler.h"

#include <glad/glad.h>

namespace Acorn
{
	class OpenGLFrameProfiler : public FrameProfiler
	{
	public:
		OpenGLFrameProfiler();
		~OpenGLFrameProfiler();

		void SendFrame(int width, int height) override;

	private:
		GLuint m_FiTexture[4];
		GLuint m_FiFrameBuffer[4];
		GLuint m_FiPbo[4];
		GLsync m_FiFence[4];

		int m_FiIdx = 0;
		std::vector<int> m_FiQueue;
	};

}