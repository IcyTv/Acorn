## for enum in enums
	enum class {{ enum.name }}
	{
		{{ join(enum.values, ",\n\t\t\t") }}
	};

	static std::string {{ enum.name }}ToString({{ enum.name }} value)
	{
		switch (value)
		{
##for value in enum.values
		case {{ enum.name }}::{{ value }}:
			return "{{ value }}";
## endfor
		default:
			return "<unknown>";
		};
	}

	void Bind{{ enum.name }}(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global);
##endfor