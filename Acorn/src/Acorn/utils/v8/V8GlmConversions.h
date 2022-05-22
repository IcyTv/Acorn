
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#include "core/Core.h"

#include "utils/v8/V8Import.h"

#include <v8pp/convert.hpp>
#include <glm/glm.hpp>

#define AC_V8_GET(type, name) \
	v8::Local<v8::Value> v8##name = obj->Get(context, v8pp::to_v8(isolate, #name)).ToLocalChecked(); \
	type name = v8pp::from_v8<type>(isolate, v8##name);

static v8::Local<v8::Function> LoadModule(v8::Isolate* isolate, std::string_view name) {
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Module> module = Acorn::V8Import::ResolveBuiltin(context, name);
	auto instantiated = module->InstantiateModule(context, Acorn::V8Import::CallResolve).FromJust();
	AC_CORE_ASSERT(instantiated, "Failed to instantiate module");
	module->Evaluate(context).ToLocalChecked();
	v8::Local<v8::Object> ns = module->GetModuleNamespace().As<v8::Object>();
	v8::Local<v8::Function> constructor = ns->Get(context, v8pp::to_v8(isolate, "default")).ToLocalChecked().As<v8::Function>();
	return constructor;

}

template<>
struct v8pp::convert<glm::vec2>
{
	using from_type = glm::vec2;
	using to_type = v8::Local<v8::Object>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!value->IsObject())
		{
			return false;
		}
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Object> obj = value->ToObject(context).ToLocalChecked();
		bool hasX = obj->Has(context, v8pp::to_v8(isolate, "x")).ToChecked();
		bool hasY = obj->Has(context, v8pp::to_v8(isolate, "y")).ToChecked();
		return hasX && hasY;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!is_valid(isolate, value))
		{
			throw std::invalid_argument("Invalid conversion from v8 value to glm::vec2");
		}

		v8::HandleScope scope(isolate);
		v8::Local<v8::Object> obj = value.As<v8::Object>();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		//v8::Local<v8::Value> x = obj->Get(context, v8pp::to_v8(isolate, "x")).ToLocalChecked();
		AC_V8_GET(float, x);
		AC_V8_GET(float, y);
		return glm::vec2(x, y);
	}

	static to_type to_v8(v8::Isolate* isolate, const from_type& val)
	{
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Function> vec2Constructor = LoadModule(isolate, "@math.gl/core/dist/esm/classes/vector2.js");
		v8::Local<v8::Value> args[2]{ v8pp::to_v8(isolate, val.x), v8pp::to_v8(isolate, val.y) };
		auto instance = vec2Constructor->NewInstance(context, 2, args).ToLocalChecked();
		return scope.Escape(instance);
	}
};

template<>
struct v8pp::is_wrapped_class<glm::vec2> : std::false_type {};

template<>
struct v8pp::convert<glm::vec3>
{
	using from_type = glm::vec3;
	using to_type = v8::Local<v8::Object>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!value->IsObject())
		{
			return false;
		}
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Object> obj = value->ToObject(context).ToLocalChecked();
		bool hasX = obj->Has(context, v8pp::to_v8(isolate, "x")).ToChecked();
		bool hasY = obj->Has(context, v8pp::to_v8(isolate, "y")).ToChecked();
		bool hasZ = obj->Has(context, v8pp::to_v8(isolate, "z")).ToChecked();
		return hasX && hasY && hasZ;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!is_valid(isolate, value))
		{
			throw std::invalid_argument("Invalid conversion from v8 value to glm::vec3");
		}

		v8::HandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Object> obj = value.As<v8::Object>();
		AC_V8_GET(float, x);
		AC_V8_GET(float, y);
		AC_V8_GET(float, z);
		return glm::vec3(x, y, z);
	}

	static to_type to_v8(v8::Isolate* isolate, const from_type& val)
	{
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Function> vec3Constructor = LoadModule(isolate, "@math.gl/core/dist/esm/classes/vector3.js");
		v8::Local<v8::Value> args[3]{ v8pp::to_v8(isolate, val.x), v8pp::to_v8(isolate, val.y), v8pp::to_v8(isolate, val.z) };
		auto instance = vec3Constructor->NewInstance(context, 3, args).ToLocalChecked();
		return scope.Escape(instance);
	}
};

template<>
struct v8pp::is_wrapped_class<glm::vec3> : std::false_type {};

template<>
struct v8pp::convert<glm::vec4>
{
	using from_type = glm::vec4;
	using to_type = v8::Local<v8::Object>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!value->IsObject())
		{
			return false;
		}
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Object> obj = value->ToObject(context).ToLocalChecked();
		bool hasX = obj->Has(context, v8pp::to_v8(isolate, "x")).ToChecked();
		bool hasY = obj->Has(context, v8pp::to_v8(isolate, "y")).ToChecked();
		bool hasZ = obj->Has(context, v8pp::to_v8(isolate, "z")).ToChecked();
		bool hasW = obj->Has(context, v8pp::to_v8(isolate, "w")).ToChecked();
		return hasX && hasY && hasZ && hasW;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!is_valid(isolate, value))
		{
			throw std::invalid_argument("Invalid conversion from v8 value to glm::vec3");
		}

		v8::HandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Object> obj = value.As<v8::Object>();
		AC_V8_GET(float, x);
		AC_V8_GET(float, y);
		AC_V8_GET(float, z);
		AC_V8_GET(float, w);
		return glm::vec4(x, y, z, w);
	}

	static to_type to_v8(v8::Isolate* isolate, const from_type& val)
	{
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Function> vec3Constructor = LoadModule(isolate, "@math.gl/core/dist/esm/classes/vector4.js");
		v8::Local<v8::Value> args[4]{ v8pp::to_v8(isolate, val.x), v8pp::to_v8(isolate, val.y), v8pp::to_v8(isolate, val.z), v8pp::to_v8(isolate, val.w) };
		auto instance = vec3Constructor->NewInstance(context, 4, args).ToLocalChecked();
		return scope.Escape(instance);
	}
};

template<>
struct v8pp::is_wrapped_class<glm::vec4> : std::false_type {};

template<>
struct v8pp::convert<glm::mat3>
{
	using from_type = glm::mat3;
	using to_type = v8::Local<v8::Object>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!value->IsArray())
		{
			return false;
		}

		v8::Local<v8::Array> arr = value.As<v8::Array>();
		return arr->Length() == 9;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!is_valid(isolate, value))
		{
			throw std::invalid_argument("Invalid conversion from v8 value to glm::vec3");
		}

		v8::HandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Array> arr = value.As<v8::Array>();
		return glm::mat3 {
			v8pp::from_v8<float>(isolate, arr->Get(context, 0).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 1).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 2).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 3).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 4).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 5).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 6).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 7).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 8).ToLocalChecked()),
		};
	}

	static to_type to_v8(v8::Isolate* isolate, const from_type& val)
	{
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Function> vec3Constructor = LoadModule(isolate, "@math.gl/core/dist/esm/classes/matrix3.js");
		v8::Local<v8::Value> args[9]{
			v8pp::to_v8(isolate, val[0][0]), v8pp::to_v8(isolate, val[0][1]), v8pp::to_v8(isolate, val[0][2]),
			v8pp::to_v8(isolate, val[1][0]), v8pp::to_v8(isolate, val[1][1]), v8pp::to_v8(isolate, val[1][2]),
			v8pp::to_v8(isolate, val[2][0]), v8pp::to_v8(isolate, val[2][1]), v8pp::to_v8(isolate, val[2][2])
		};
		auto instance = vec3Constructor->NewInstance(context, 9, args).ToLocalChecked();
		return scope.Escape(instance);
	}
};

template<>
struct v8pp::is_wrapped_class<glm::mat3> : std::false_type {};

template<>
struct v8pp::convert<glm::mat4>
{
	using from_type = glm::mat4;
	using to_type = v8::Local<v8::Object>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!value->IsArray())
		{
			return false;
		}

		v8::Local<v8::Array> arr = value.As<v8::Array>();
		return arr->Length() == 16;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if(!is_valid(isolate, value))
		{
			throw std::invalid_argument("Invalid conversion from v8 value to glm::vec3");
		}

		v8::HandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Array> arr = value.As<v8::Array>();
		return glm::mat4 {
			v8pp::from_v8<float>(isolate, arr->Get(context, 0).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 1).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 2).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 3).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 4).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 5).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 6).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 7).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 8).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 9).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 10).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 11).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 12).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 13).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 14).ToLocalChecked()),
			v8pp::from_v8<float>(isolate, arr->Get(context, 15).ToLocalChecked()),
		};
	}

	static to_type to_v8(v8::Isolate* isolate, const from_type& val)
	{
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Function> vec3Constructor = LoadModule(isolate, "@math.gl/core/dist/esm/classes/matrix3.js");
		v8::Local<v8::Value> args[16]{
			v8pp::to_v8(isolate, val[0][0]), v8pp::to_v8(isolate, val[0][1]), v8pp::to_v8(isolate, val[0][2]), v8pp::to_v8(isolate, val[0][3]),
			v8pp::to_v8(isolate, val[1][0]), v8pp::to_v8(isolate, val[1][1]), v8pp::to_v8(isolate, val[1][2]), v8pp::to_v8(isolate, val[1][3]),
			v8pp::to_v8(isolate, val[2][0]), v8pp::to_v8(isolate, val[2][1]), v8pp::to_v8(isolate, val[2][2]), v8pp::to_v8(isolate, val[2][3]),
		    v8pp::to_v8(isolate, val[3][0]), v8pp::to_v8(isolate, val[3][1]), v8pp::to_v8(isolate, val[3][2]), v8pp::to_v8(isolate, val[3][3])
		};
		auto instance = vec3Constructor->NewInstance(context, 16, args).ToLocalChecked();
		return scope.Escape(instance);
	}
};

template<>
struct v8pp::is_wrapped_class<glm::mat4> : std::false_type {};