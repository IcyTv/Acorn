#include "GPUDriverImpl.h"
#include "Ultralight/platform/GPUDriver.h"

namespace Acorn
{
	GPUDriverImpl::GPUDriverImpl()
		: m_BatchCount(0)
	{
	}

	bool GPUDriverImpl::HasCommandsPending()
	{
		return !m_CommandList.empty();
	}

	void GPUDriverImpl::DrawCommandList()
	{
		if (m_CommandList.empty())
			return;

		m_BatchCount = 0;

		for (auto& cmd : m_CommandList)
		{
			if (cmd.command_type == ultralight::kCommandType_DrawGeometry)
				DrawGeometry(cmd.geometry_id, cmd.indices_count, cmd.indices_offset, cmd.gpu_state);
			else if (cmd.command_type == ultralight::kCommandType_ClearRenderBuffer)
				ClearRenderBuffer(cmd.gpu_state.render_buffer_id);

			m_BatchCount++;
		}

		m_CommandList.clear();
	}

	int GPUDriverImpl::batch_count() const
	{
		return m_BatchCount;
	}

	void GPUDriverImpl::BeginSynchronize() {}

	void GPUDriverImpl::EndSynchronize() {}

	uint32_t GPUDriverImpl::NextTextureId()
	{
		return m_NextTextureId++;
	}

	uint32_t GPUDriverImpl::NextRenderBufferId()
	{
		return m_NextRenderBufferId++;
	}

	uint32_t GPUDriverImpl::NextGeometryId()
	{
		return m_NextGeometryId++;
	}

	void GPUDriverImpl::UpdateCommandList(const ultralight::CommandList& list)
	{
		if (list.size)
		{
			m_CommandList.resize(list.size);
			memcpy(m_CommandList.data(), list.commands, list.size * sizeof(ultralight::Command));
		}
	}
}