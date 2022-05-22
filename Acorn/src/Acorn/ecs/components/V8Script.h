#pragma once

#include "TSCompiler.h"
#include "core/Core.h"
#include "core/Timestep.h"
#include "ecs/Scene.h"
#include "ecs/Entity.h"

#include <boost/variant.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// TODO remove v8 header dependency
//  #include <v8-persistent-handle.h>
#include <glm/glm.hpp>
#include <v8.h>

namespace Acorn
{
	namespace Components
	{
		struct Camera;
	}

	enum class V8Types
	{
		Unknown,
		Undefined,
		Null,
		Boolean,
		Number,
		String,
		Object,
		Array,
		Function,
	};

	V8Types ToV8Type(const TsType type);

	struct V8Data
	{
		glm::mat4 PrimaryCameraViewProjectionMatrix;
	};

	// TODO rename to TSScript
	class V8Script
	{
		friend class V8Engine;

	public:
		explicit V8Script(Entity entity);
		explicit V8Script(Entity entity, const std::string& filePath);
		~V8Script();

		void Load(Entity entity);
		void Dispose();

		void Compile();
		void Watch();

		void OnUpdate(Timestep ts, Camera* camera = nullptr); // TODO make a camera main

		template <typename T>
		void SetValue(std::string parameterName, T value);

		template <typename T>
		T& GetValue(std::string parameterName);

		std::unordered_map<std::string, std::string> GetParameters();
		void SetParameters(std::unordered_map<std::string, std::string> params);

		inline std::string GetFilePath() const { return m_TSFilePath; }

		inline float GetLastElapsedTime() const { return m_LastExecutionTime; }

		inline const TSScriptData& GetScriptData() const { return m_Data; }

	private:
		// v8::Local<v8::Context> CreateShellContext();
		v8::Local<v8::Context> CreateShellContext();

		void GetComponent(const v8::FunctionCallbackInfo<v8::Value>& args);

	private:
		v8::Isolate* m_Isolate;

		std::string m_TSFilePath = "";
		std::string m_JSFilePath = "";
		std::string m_Name = "";

		TSScriptData m_Data;
		std::unordered_map<std::string, boost::variant<bool, float, std::string>> m_Parameters;

		v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>> m_Context;

		v8::Persistent<v8::Object> m_Class;
		v8::Persistent<v8::Function> m_OnUpdate;

		// Scope<v8::SnapshotCreator> m_SnapshotCreator;

		Entity m_Entity;

		float m_LastExecutionTime = 0.0f;
	};

	class V8Engine
	{
		friend class V8Script;

	public:
		V8Engine(const V8Engine&) = delete;
		V8Engine& operator=(const V8Engine&) = delete;
		V8Engine(V8Engine&&) = delete;
		V8Engine& operator=(V8Engine&&) = delete;

		static V8Engine& instance()
		{
			static V8Engine instance;
			return instance;
		}

		void AddScript(V8Script* script)
		{
			if (m_Scripts.empty() && !m_Running)
			{
				Initialize();
			}
			m_Scripts.push_back(script);
		}

		void SetData(const V8Data& data)
		{
			m_Data = data;
		}

		V8Data& GetData()
		{
			return m_Data;
		}

		void RemoveScript(V8Script* script)
		{
			m_Scripts.erase(std::remove(m_Scripts.begin(), m_Scripts.end(), script), m_Scripts.end());
		}

		inline bool isRunning() const { return m_Running; }

		~V8Engine();

	private:
		V8Engine();

		void Initialize();
		void Shutdown();

	private:
		bool m_Running = false;

		V8Data m_Data;

		std::unique_ptr<v8::Platform> m_Platform;

		std::vector<V8Script*> m_Scripts; // TODO change to a ref
	};
}