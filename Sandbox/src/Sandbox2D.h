#pragma once

#include <Acorn.h>

#include "ParticleSystem.h"

class Sandbox2D : public Acorn::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Acorn::Timestep ts) override;
	virtual void OnImGuiRender(Acorn::Timestep t) override;
	virtual void OnEvent(Acorn::Event& e) override;

private:
	Acorn::OrthographicCameraController m_CameraController;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	Acorn::Ref<Acorn::Texture2d> m_CheckerboardTexture;
	Acorn::Ref<Acorn::Texture2d> m_SpriteSheet;

	bool m_IsCameraControlsOpen = true;

	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;

	uint32_t m_MapWidth = 0, m_MapHeight = 0;
	std::unordered_map<char, Acorn::Ref<Acorn::ext2d::SubTexture>> m_TileMap;
	Acorn::Ref<Acorn::ext2d::SubTexture> m_BarrelTexture;
};