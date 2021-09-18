import ts from "./typescript/typescript";

/**
 * @type {import("./typescript").CompilerHost}
 */
const compilerHost = {
    getSourceFile: (fileName, languageVersion, onError) => {
        const source = AcornFileSystem.ReadFile(fileName);
        if (source === undefined) {
            if (onError) {
                print(`Error reading file ${fileName}`);
                onError(`Unable to read file: ${fileName}`);
            }
            return undefined;
        }
        return ts.createSourceFile(fileName, source, languageVersion, /*setParentNodes */ true);
    },
    getDefaultLibFileName: (defaultLibOptions) => "res/scripts/builtins/typescript/" + ts.getDefaultLibFileName(defaultLibOptions),
    writeFile: (fileName, data) => {
        try {
            print(`Writing file ${fileName}`);
            AcornFileSystem.WriteFile(fileName, data);
        }
        catch (e) {
            print(`Error writing file ${fileName}`);
            print(e);
            throw e;
        }
    },
    getCurrentDirectory: () => "res/scripts/builtins/typescript/",
    getDirectories: (path) => [],
    fileExists: (fileName) => {
        return AcornFileSystem.FileExists(fileName);
    },
    readFile: (fileName) => {
        return AcornFileSystem.ReadFile(fileName);
    },
    getCanonicalFileName: (fileName) => fileName,
    useCaseSensitiveFileNames: () => true,
    getNewLine: () => "\n",
    getEnvironmentVariable: () => "" // do nothing
};

export default compilerHost;