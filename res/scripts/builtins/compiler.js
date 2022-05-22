// // import {transpileModule, ModuleKind, ScriptTarget} from './res/scripts/builtins/typescript.js';
import ts from './typescript';

const source = "class Test {\n"
    + "    constructor(asdf: string) {\n"
    + "        let b;\n"
    + "        this.test = 'test';\n"
    + "    }\n"
    + "}";

export const result = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2015 } });

// const ts = {};

// if (typeof module !== "undefined" && module.exports) {
//     print("Overinding export");
//     module.exports = ts;
// } else {
//     print(typeof module);
//     print(module.exports);
//     print(!!module.exports);
// }

// print(typeof module);
// print(typeof module.exports);

// import babel from "./babel.min"

// export const result = babel.transform("class Test {\n"
//     + "    constructor(asdf: string) {\n"
//     + "        let b;\n"
//     + "        this.test = 'test';\n"
//     + "    }\n"
//     + "}", { "presets": ["es2015"] });