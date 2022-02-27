from pathlib import Path
import sys
import os


def main():
    if len(sys.argv) != 2:
        print("Usage: tpl_generator.py <template_file>")
        sys.exit(1)
    file_name = sys.argv[1]
    own_folder = Path(__file__).parent.absolute() / "src"
    template_path = own_folder / file_name
    out_path = own_folder / (file_name.replace(".tpl", ".h"))

    tpl_name = template_path.stem

    with open(out_path, "w") as out:
        with open(template_path, "r") as tpl:
            out_path.write_text(f"""
#pragma once

namespace Acorn::IDL
{{
	const char* {tpl_name} = R"~~~({tpl.read()})~~~";
}}
			""")


if __name__ == '__main__':
    main()
