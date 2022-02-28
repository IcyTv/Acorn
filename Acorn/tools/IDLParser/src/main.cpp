#include "IDLParser.h"
#include "IDLTypes.h"
#include "Generator.h"

#include <args.hxx>
#include <boost/exception/diagnostic_information.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

extern std::vector<std::string_view> s_HeaderSearchPaths;

int main(int argc, const char* argv[])
{
	args::ArgumentParser parser("IDL Parser", "Parses IDL files and outputs them to C++ code.");
	args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
	args::CompletionFlag completion(parser, { "complete", "completion" });
	args::CounterFlag verbose(parser, "verbose", "Verbose output", { 'v', "verbose" });

	args::Flag headerMode(parser, "header", "Generates the wrapper .h file.", { 'H', "header" });
	args::Flag implementationMode(parser, "implementation", "Generates the wrapper .cpp file.", { 'I', "implementation" });
	args::ValueFlagList<std::string> outputPath(parser, "output", "Output path for the generated files.", { 'o', "output" });
	args::Positional<std::filesystem::path> path(parser, "IDL File", "The IDL file to parse.", args::Options::Required);
	args::Positional<std::filesystem::path> importBasePath(parser, "Import Base Path", "The base path to use for imports.");

	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (const args::Completion& e)
	{
		std::cout << e.what();
		return 0;
	}
	catch (const args::Help&)
	{
		std::cout << parser;
		return 0;
	}
	catch (const args::RequiredError& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (const args::ParseError& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;

		return 1;
	}

	auto filePath = path.Get();

	std::cout << "Parsing " << filePath << std::endl;

	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "Failed to open file: " << path.Get() << std::endl;
		return 1;
	}

	try
	{
		// Define the namespace as the parent's parent path
		auto parentPath		  = filePath.parent_path();
		auto headerPath		  = outputPath.Get()[0];
		auto implementationPath = outputPath.Get()[1];

		std::cerr << "Generating namespace: " << headerPath << std::endl;

//		// auto parentParentPath = parentPath.parent_path();
//		auto headerOutPath = parentPath / filePath.stem();
//		headerOutPath.replace_extension(".h");
//		auto implementationOutPath = parentPath / filePath.stem();
//		implementationOutPath.replace_extension(".cpp");

		auto data = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		std::filesystem::path basePath;
		if (importBasePath.Matched())
			basePath = importBasePath.Get();
		else
			basePath = parentPath;

		auto interface = Acorn::IDL::Parser(filePath.string(), data, basePath.string()).Parse();

		// TODO builtin namespaces?
		interface->FullyQualifiedName = interface->Name;

		if (verbose.Get() > 1)
		{
			std::cout << "Attributes:" << std::endl;
			for (auto& attribute : interface->Attributes)
			{
				std::cout << "\t";
				if (attribute.ReadOnly)
					std::cout << "readonly ";
				std::cout << attribute.Type->Name;
				if (attribute.Type->Nullable)
					std::cout << "?";
				std::cout << " " << attribute.Name << std::endl;
			}

			std::cout << "Functions:" << std::endl;
			for (auto& functions : interface->Functions)
			{
				std::cout << "\t";
				std::cout << functions.ReturnType->Name;
				if (functions.ReturnType->Nullable)
					std::cout << "?";
				std::cout << " " << functions.Name << "(" << std::endl;

				for (auto& parameters : functions.Parameters)
				{
					std::cout << "\t\t";
					std::cout << parameters.Type->Name;
					if (parameters.Type->Nullable)
						std::cout << "?";
					std::cout << " " << parameters.Name << std::endl;
				}
			}

			std::cout << "Static Functions:" << std::endl;
			for (auto& function : interface->StaticFunctions)
			{
				std::cout << "\t";
				std::cout << function.ReturnType->Name;
				if (function.ReturnType->Nullable)
					std::cout << "?";
				std::cout << " " << function.Name << "(" << std::endl;

				for (auto& parameters : function.Parameters)
				{
					std::cout << "\t\t";
					std::cout << parameters.Type->Name;
					if (parameters.Type->Nullable)
						std::cout << "?";
					std::cout << " " << parameters.Name << std::endl;
				}
			}
		}

		// FIXME: This is a hack and might not work on all platforms
		std::filesystem::path programLocation(argv[0]);
		programLocation = programLocation.parent_path();
		std::string templatePath = programLocation.string() + "/";

		if (headerMode.Get())
			Acorn::IDL::GenerateHeader(*interface, headerPath, templatePath);

		if (implementationMode.Get())
			Acorn::IDL::GenerateImplementation(*interface, implementationPath, templatePath);

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	catch(...)
	{
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
	}

	return 0;
}