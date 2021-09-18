/// <reference path="../declarations.d.ts"/>

import ts from "./typescript/typescript";
import compilerHost from "./compilerHost";

//Based on https://github.com/Microsoft/TypeScript/wiki/Using-the-Compiler-API#using-the-type-checker


interface DocEntry {
    name?: string;
    fileName?: string;
    documentation?: string;
    type?: string;
    constructors?: DocEntry[];
    parameters?: DocEntry[];
    returnType?: string;
    tags?: Record<string, string>;
}

//TODO multiple class support to array

if (!AcornFileSystem.CurrentFile) {
    throw new Error("No current file");
}

export let documentation: DocEntry[];
try {
    /** Generate documentation for all classes in a set of .ts files */
    interface DocEntry {
        name?: string;
        fileName?: string;
        documentation?: string;
        type?: string;
        constructors?: DocEntry[];
        parameters?: DocEntry[];
        returnType?: string;
    }

    /** Generate documentation for all classes in a set of .ts files */
    function generateDocumentation(
        fileNames: string[],
        options: ts.CompilerOptions
    ): any {
        // Build a program using the set of root file names in fileNames
        let program = ts.createProgram(fileNames, options, compilerHost);
        print("ROOT FILES:", program.getRootFileNames());
        // Get the checker, we will use it to find more about classes
        let checker = program.getTypeChecker();

        let allOutput = [];
        let output = null;
        let exportStatementFound = false;

        let currentMethod = null;
        let fileIndex = 0;
        // Visit the sourceFile for each "source file" in the program
        //ie don't use program.getSourceFiles() as it gets all the imports as well

        for (let i = 0; i < fileNames.length; i++) {
            const fileName = fileNames[i];
            const sourceFile = program.getSourceFile(fileName);
            // print("sourceFile.kind:", sourceFile.kind);
            if ((sourceFile.kind as number) === ts.SyntaxKind.ImportDeclaration) {
                print("IMPORT");
            }
            exportStatementFound = false;
            if (!sourceFile.isDeclarationFile) {
                // Walk the tree to search for classes
                output = {
                    fileName: fileName,
                    component: [],
                    fields: [],
                    methods: []
                };

                ts.forEachChild(sourceFile, visit);
                if (output) {
                    allOutput.push(output);
                }
                if (!exportStatementFound) {
                    print("WARNING: no export statement found in:", fileName);
                }
            }
        }

        output.fields = output.fields.filter(field => !field.name.startsWith("#"));

        // print out the definitions
        return output;
        /** visit nodes */
        function visit(node: ts.Node) {
            if (!output) {
                return;
            }
            if (node.kind === ts.SyntaxKind.ImportDeclaration) {
                print("IMPORT");
                //output = null;
                return;
            }
            if (node.kind === ts.SyntaxKind.DefaultKeyword) {
                print("DEFAULT");
                return;
            }
            if (node.kind === ts.SyntaxKind.ExportKeyword) {
                exportStatementFound = true;
                print("EXPORT");
                return;
            }

            if (ts.isClassDeclaration(node) && node.name) {
                // This is a top level class, get its symbol
                let symbol = checker.getSymbolAtLocation(node.name);
                if (symbol) {
                    //need localSymbol for the name, if there is one because otherwise exported as "default"
                    symbol = ((symbol.valueDeclaration as any).localSymbol) ? (symbol.valueDeclaration as any).localSymbol : symbol;
                    const details = serializeClass(symbol);
                    output.component.push(details);
                }
                ts.forEachChild(node, visit);
            }
            else if (ts.isPropertyDeclaration(node)) {
                let symbol = checker.getSymbolAtLocation(node.name);
                if (symbol) {
                    output.fields.push(serializeField(symbol));
                }
            } else if (ts.isMethodDeclaration(node)) {
                let symbol = checker.getSymbolAtLocation(node.name);
                if (symbol) {
                    currentMethod = serializeMethod(symbol);
                    output.methods.push(currentMethod);
                }
                ts.forEachChild(node, visit);
            }


        }

        /** Serialize a symbol into a json object */
        function serializeSymbol(symbol: ts.Symbol): DocEntry {
            const tags = symbol.getJsDocTags();
            let tagMap: any = null;
            if (tags?.length) {
                print("TAGS:", tags);
                for (let i = 0; i < tags.length; i++) {
                    const tag = tags[i];
                    if (tag.name !== "param") {
                        tagMap = tagMap ? tagMap : {};
                        tagMap[tag.name] = tag.text;
                    }
                }
            }
            return {
                name: symbol.getName(),
                documentation: ts.displayPartsToString(symbol.getDocumentationComment(checker)),
                type: checker.typeToString(
                    checker.getTypeOfSymbolAtLocation(symbol, symbol.valueDeclaration!)
                ),
                tags: tagMap,
            } as any;
        }

        /** Serialize a class symbol information */
        function serializeClass(symbol: ts.Symbol) {
            let details = serializeSymbol(symbol);

            // Get the construct signatures
            let constructorType = checker.getTypeOfSymbolAtLocation(
                symbol,
                symbol.valueDeclaration!
            );
            details.constructors = constructorType
                .getConstructSignatures()
                .map(serializeSignature);
            return details;
        }

        function serializeField(symbol: ts.Symbol) {
            return serializeSymbol(symbol);
        }

        function serializeMethod(symbol: ts.Symbol) {
            let details = serializeSymbol(symbol);

            // Get the construct signatures
            let methodType = checker.getTypeOfSymbolAtLocation(
                symbol,
                symbol.valueDeclaration!
            );
            let callingDetails = methodType.getCallSignatures()
                .map(serializeSignature)["0"];
            details = { ...details, ...callingDetails };
            return details;
        }

        /** Serialize a signature (call or construct) */
        function serializeSignature(signature: ts.Signature) {
            return {
                parameters: signature.parameters.map(serializeSymbol),
                returnType: checker.typeToString(signature.getReturnType()),
                documentation: ts.displayPartsToString(signature.getDocumentationComment(checker))
            };
        }
    }

    documentation = generateDocumentation([AcornFileSystem.CurrentFile], {
        target: ts.ScriptTarget.ES5,
        module: ts.ModuleKind.CommonJS
    });

    print(JSON.stringify(documentation, null, 4));
} catch (e) {
    print(e);
}