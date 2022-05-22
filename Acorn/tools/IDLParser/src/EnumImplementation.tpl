## for enum in enums
	void Bind{{ enum.name }}(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global)
	{
		v8::Local<v8::ObjectTemplate> enumDef = v8::ObjectTemplate::New(isolate);
## for value in enum.values
		enumDef->Set(isolate, "{{ value }}", v8pp::to_v8(isolate, {{ enum.name }}::{{ value }}));
## endfor
		enumDef->Set(isolate, "NameOf", v8pp::wrap_function_template(isolate, &{{ enum.name }}ToString));

		global->Set(isolate, "{{ enum.name }}", enumDef);

	}
## endfor