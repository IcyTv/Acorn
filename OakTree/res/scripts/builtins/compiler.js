///<reference path="../declarations.d.ts" />
import ts from "./typescript/typescript";
import compilerHost from "./compilerHost";
import swc from "./swc/wasm";
function compile(fileNames, options) {
    let program = ts.createProgram(fileNames, options, compilerHost);
    let emitResult = program.emit();
    let allDiagnostics = ts
        .getPreEmitDiagnostics(program)
        .concat(emitResult.diagnostics);
    allDiagnostics.forEach(diagnostic => {
        if (diagnostic.file) {
            let { line, character } = ts.getLineAndCharacterOfPosition(diagnostic.file, diagnostic.start);
            let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
            print(`${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`);
        }
        else {
            print(ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n"));
        }
    });
    return {
        outputText: (emitResult.emittedFiles || []).join(","),
    };
}
print(swc);
if (!AcornFileSystem.CurrentFile) {
    throw new Error("No file to compile!");
}
export const result = compile([AcornFileSystem.CurrentFile], {
    module: ts.ModuleKind.ESNext,
    target: ts.ScriptTarget.ESNext,
    skipLibCheck: true,
});
