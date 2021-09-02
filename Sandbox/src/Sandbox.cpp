#include <Acorn.h>
#include <core/EntryPoint.h>

#include "Sandbox2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <imgui.h>

#include <iostream>

class ExampleLayer : public Acorn::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(16.0f / 9.0f, true)
	{
		m_VertexArray = Acorn::VertexArray::Create();

		float vertices[3 * 7] = {
			-0.5f,
			-0.5f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.5f,
			-0.5f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f,
			0.5f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
		};

		Acorn::Ref<Acorn::VertexBuffer> vertexBuffer;
		vertexBuffer = Acorn::VertexBuffer::Create(vertices, sizeof(vertices));

		Acorn::BufferLayout layout = {
			{Acorn::ShaderDataType::Float3, "a_Position"},
			{Acorn::ShaderDataType::Float4, "a_Color"},
		};
		vertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = {
			0, 1, 2};

		Acorn::Ref<Acorn::IndexBuffer> indexBuffer;
		indexBuffer = Acorn::IndexBuffer::Create(indices, 3);

		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquaredVA = Acorn::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.75f, -0.75f, 0.0f, 0.0f, 0.0f,
			0.75f, -0.75f, 0.0f, 1.0f, 0.0f,
			0.75f, 0.75f, 0.0f, 1.0f, 1.0f,
			-0.75f, 0.75f, 0.0f, 0.0f, 1.0f};

		Acorn::Ref<Acorn::VertexBuffer> squaredVB;
		squaredVB = Acorn::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		squaredVB->SetLayout({
			{Acorn::ShaderDataType::Float3, "a_Position"},
			{Acorn::ShaderDataType::Float2, "a_TexCoord"},
		});

		m_SquaredVA->AddVertexBuffer(squaredVB);

		uint32_t squareIndices[6] = {
			0, 1, 2, 2, 3, 0};

		Acorn::Ref<Acorn::IndexBuffer> squaredIB;
		squaredIB = Acorn::IndexBuffer::Create(squareIndices, 6);

		m_SquaredVA->SetIndexBuffer(squaredIB);

		m_ShaderLibrary.LoadFolder("../Acorn/res/shaders");

		m_Texture = Acorn::Texture2d::Create("res/textures/Checkerboard.png");
		m_ChernoLogoTexture = Acorn::Texture2d::Create("res/textures/ChernoLogo.png");

		m_ShaderLibrary["Textured"]->Bind();
		m_ShaderLibrary["Textured"]->SetInt("u_Texture", 0);
	}

	void OnUpdate(Acorn::Timestep timestep) override
	{
		m_Frame++;
		m_CameraController.OnUpdate(timestep);

		if (Acorn::Input::IsKeyPressed(AC_KEY_TAB))
			AC_TRACE("Tab pressed");

		Acorn::RenderCommand::SetClearColor({0.0f, 0.0f, 0.2f, 1.0f});
		Acorn::RenderCommand::Clear();

		m_SquareTransform = glm::rotate(m_SquareTransform, glm::radians(22.0f * timestep), {0.0f, 0.0f, 1.0f});
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		glm::vec4 redColor(1.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 blueColor(0.0f, 0.0f, 1.0f, 1.0f);

		Acorn::Renderer::BeginScene(m_CameraController.GetCamera());

		m_ShaderLibrary["FlatColor"]->Bind();
		m_ShaderLibrary["FlatColor"]->SetFloat4("u_Color", m_SquareColor);
		for (int i = 0; i < 20; i++)
		{
			glm::mat4 transform = glm::translate(glm::mat4(1.0), glm::vec3(0.2 * (i / 4) + 0.5, 0.2 * (i % 4), 0.0f));
			Acorn::Renderer::Submit(m_ShaderLibrary["FlatColor"], m_SquaredVA, transform * scale * m_SquareTransform);
		}

		m_ShaderLibrary["Textured"]->Bind();
		m_Texture->Bind();
		Acorn::Renderer::Submit(m_ShaderLibrary["Textured"], m_SquaredVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.0)));
		m_ChernoLogoTexture->Bind();
		Acorn::Renderer::Submit(m_ShaderLibrary["Textured"], m_SquaredVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.0)));

		//Triangle
		//Acorn::Renderer::Submit(m_Shader, m_VertexArray, glm::translate(glm::mat4(1.0f), {-0.7f, 0.0f, 0.0f}));

		Acorn::Renderer::EndScene();
	}

	void OnImGuiRender(Acorn::Timestep t) override
	{

		//ImGui::Begin("Colors");
		//ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
		//ImGui::End();
	}

	void OnEvent(Acorn::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

	~ExampleLayer()
	{
	}

private:
	Acorn::Ref<Acorn::VertexArray> m_VertexArray;

	Acorn::ShaderLibrary m_ShaderLibrary;
	Acorn::Ref<Acorn::Texture2d> m_Texture, m_ChernoLogoTexture;
	Acorn::Ref<Acorn::VertexArray> m_SquaredVA;

	glm::mat4 m_SquareTransform = glm::mat4(1.0f);
	glm::vec4 m_SquareColor = {0.2f, 0.3f, 0.7f, 1.0f};

	uint32_t m_Frame;

	Acorn::OrthographicCameraController m_CameraController;
};

class Sandbox : public Acorn::Application
{
public:
	Sandbox(Acorn::ApplicationCommandLineArgs args)
		: Acorn::Application("Sandbox", args)
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{
	}
};

AC_ENTRY(Sandbox)