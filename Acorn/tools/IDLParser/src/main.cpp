#include "IDLParser.h"
#include "IDLTypes.h"

#include <args.hxx>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <unordered_set>

extern std::vector<std::string_view> s_HeaderSearchPaths;

namespace Acorn::IDL
{
	// void GenerateConstructorHeader(Interface const&);
	// void GenerateConstructorImplementation(Interface const&);
	void GeneratePrototypeHeader(Interface const&);
	void GeneratePrototypeImplementation(Interface const&);
	void GenerateHeader(Interface const&);
	void GenerateImplementation(Interface const&);
	void GenerateIteratorPrototypeHeader(Interface const&);
	void GenerateIteratorPrototypeImplementation(Interface const&);
	void GenerateIteratorHeader(Interface const&);
	void GenerateIteratorImplementation(Interface const&);
} // namespace Acorn::IDL

int main(int argc, const char* argv[])
{
	args::ArgumentParser parser("IDL Parser", "Parses IDL files and outputs them to C++ code.");
	args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
	args::CompletionFlag completion(parser, { "complete", "completion" });
	args::CounterFlag verbose(parser, "verbose", "Verbose output", { 'v', "verbose" });

	args::Flag headerMode(parser, "header", "Generates the wrapper .h file.", { 'H', "header" });
	args::Flag implementationMode(parser, "implementation", "Generates the wrapper .cpp file.", { 'I', "implementation" });
	// args::Flag constructorHeaderMode(parser, "constructor-header", "Generates the constructor .h file.", { 'C', "constructor-header" });
	// args::Flag constructorImplementationMode(parser, "constructor-implementation", "Generates the constructor .cpp file.", { 'c', "constructor-implementation" });
	args::Flag prototypeHeaderMode(parser, "prototype-header", "Generates the prototype .h file.", { 'P', "prototype-header" });
	args::Flag prototypeImplementationMode(parser, "prototype-implementation", "Generates the prototype .cpp file.", { 'p', "prototype-implementation" });
	args::Flag iteratorHeaderMode(parser, "iterator-header", "Generates the iterator .h file.", { "iterator-header" });
	args::Flag iteratorImplementationMode(parser, "iterator-implementation", "Generates the iterator .cpp file.", { "iterator-implementation" });
	args::Flag iteratorPrototypeHeaderMode(parser, "iterator-prototype-header", "Generates the iterator prototype .h file.", { "iterator-prototype-header" });
	args::Flag iteratorPrototypeImplementationMode(
		parser, "iterator-prototype-implementation", "Generates the iterator prototype .cpp file.", { "iterator-prototype-implementation" }
	);
	args::ValueFlagList<std::string, std::unordered_set> headerSearchPaths(
		parser, "header-search-path", "Add a header search path passed to the compiler.", { 'i', "header-include-path" }
	);
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
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "Failed to open file: " << path.Get() << std::endl;
		return 1;
	}

	// Define the namespace as the parent's parent path
	auto parentPath		  = filePath.parent_path();
	auto parentParentPath = parentPath.parent_path();
	auto namespace_		  = parentParentPath.filename().string();

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

	try
	{
		if (headerMode.Get())
			Acorn::IDL::GenerateHeader(*interface);

		if (implementationMode.Get())
			Acorn::IDL::GenerateImplementation(*interface);

		// if (constructorHeaderMode.Get())
		// 	Acorn::IDL::GenerateConstructorHeader(*interface);

		// if (constructorImplementationMode)
		// 	Acorn::IDL::GenerateConstructorImplementation(*interface);

		if (prototypeHeaderMode.Get())
			Acorn::IDL::GeneratePrototypeHeader(*interface);

		if (prototypeImplementationMode.Get())
			Acorn::IDL::GeneratePrototypeImplementation(*interface);

		if (iteratorHeaderMode.Get())
			Acorn::IDL::GenerateIteratorHeader(*interface);

		if (iteratorImplementationMode.Get())
			Acorn::IDL::GenerateIteratorImplementation(*interface);

		if (iteratorPrototypeHeaderMode.Get())
			Acorn::IDL::GenerateIteratorPrototypeHeader(*interface);

		if (iteratorPrototypeImplementationMode.Get())
			Acorn::IDL::GenerateIteratorPrototypeImplementation(*interface);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}