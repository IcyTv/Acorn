#include "Tracy.hpp"
#include "core/Core.h"
#include "debug/FrameProfiler.h"

#include "platform/opengl/OpenGLFrameProfiler.h"

#include <glad/glad.h>

constexpr uint32_t IMAGE_WIDTH = 320;
constexpr uint32_t IMAGE_HEIGHT = 180;

namespace Acorn
{
	OpenGLFrameProfiler::OpenGLFrameProfiler()
	{
		glGenTextures(4, m_FiTexture);
		glGenFramebuffers(4, m_FiFrameBuffer);
		glGenBuffers(4, m_FiPbo);

		for (int i = 0; i < 4; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_FiTexture[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glBindFramebuffer(GL_FRAMEBUFFER, m_FiFrameBuffer[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FiTexture[i], 0);

			glBindBuffer(GL_PIXEL_PACK_BUFFER, m_FiPbo[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, IMAGE_WIDTH * IMAGE_HEIGHT * 4, nullptr, GL_STREAM_READ);
		}
	}

	OpenGLFrameProfiler::~OpenGLFrameProfiler()
	{
		glDeleteTextures(4, m_FiTexture);
		glDeleteFramebuffers(4, m_FiFrameBuffer);
		glDeleteBuffers(4, m_FiPbo);
	}

	void OpenGLFrameProfiler::SendFrame(int width, int height)
	{
		while (!m_FiQueue.empty())
		{
			const auto fiIdx = m_FiQueue.front();
			if (glClientWaitSync(m_FiFence[fiIdx], 0, 0) == GL_TIMEOUT_EXPIRED)
				break;
			glDeleteSync(m_FiFence[fiIdx]);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, m_FiPbo[fiIdx]);
			auto ptr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, IMAGE_WIDTH * IMAGE_HEIGHT * 4, GL_MAP_READ_BIT);
			FrameImage(ptr, IMAGE_WIDTH, IMAGE_HEIGHT, m_FiQueue.size(), true);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			m_FiQueue.erase(m_FiQueue.begin());
		}

		AC_CORE_ASSERT(m_FiQueue.empty() || m_FiQueue.front() != m_FiIdx, "Frame profiler queue is not empty");
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FiFrameBuffer[m_FiIdx]);

		glBlitFramebuffer(0, 0, width, height, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FiFrameBuffer[m_FiIdx]);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_FiPbo[m_FiIdx]);
		glReadPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		m_FiFence[m_FiIdx] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_FiQueue.emplace_back(m_FiIdx);
		m_FiIdx = (m_FiIdx + 1) % 4;
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
}