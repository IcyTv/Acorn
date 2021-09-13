#pragma once

#include "core/Core.h"

#include <memory>
#include <string>
#include <vector>

#include <v8.h>

namespace Acorn
{
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
			m_Scripts.push_back(script);
		}

		~V8Engine();

	private:
		V8Engine();

	private:
		std::unique_ptr<v8::Platform> m_Platform;

		std::vector<V8Script*> m_Scripts;
	};

	class V8Script
	{
		friend class V8Engine;

	public:
		V8Script();
		V8Script(const std::string& source);
		~V8Script();

		void Run();

	private:
		v8::Local<v8::Context> CreateShellContext();

	private:
		//TODO do we need a seperate isolate for each script? -> Isolate Pool
		v8::Isolate* m_Isolate;

		//TODO figure out if we need this
		std::string m_Source;
	};
}