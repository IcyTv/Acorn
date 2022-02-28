	enum class {{ enum.name }}
	{
		{{ join(enum.values, ",\n\t\t\t") }}
	}

	static std::string {{ to_pascal_case(enum.name) }}ToString({{ enum.name }} value)
	{
		switch (value)
		{
##for value in enum.values
		case {{ value }}:
			return "{{ value }}";
## endfor
		default:
			return "<unknown>";
		};
	}