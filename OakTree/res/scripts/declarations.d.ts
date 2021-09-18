declare function print(...args: any[]): void;

declare enum ComponentTypes {
    Tag = 0,
    Transform = 1,
    SpriteRenderer = 2,
    Camera = 3,
    NativeScript = 4,
    V8Script = 5,
}

declare class ScriptSuperClass {
    HasComponent(type: ComponentTypes): boolean;
    GetComponent(type: ComponentTypes.Tag): Components.Tag;
    GetComponent(type: ComponentTypes.Transform): Components.Transform;
    GetComponent(type: ComponentTypes.SpriteRenderer): Components.SpriteRenderer;
}

declare class Input {
    static IsKeyPressed(key: number | string): boolean;
    static IsMouseButtonPressed(button: number): boolean;
    static GetMouseX(): number;
    static GetMouseY(): number;
    static GetMousePosition(): [number, number];
}

declare namespace math {
    export class vec2 {
        constructor(x: number, y: number);
        x: number;
        y: number;
    }

    export class vec3 {
        constructor(x: number, y: number, z: number);
        x: number;
        y: number;
        z: number;
    }

    export class vec4 {
        constructor(x: number, y: number, z: number, w: number);
        x: number;
        y: number;
        z: number;
        w: number;
    }
}

declare namespace Components {
    export class Tag {
        TagName: string;
    }

    export class Transform {
        Position: math.vec3;
        Rotation: math.vec3;
        Scale: math.vec3;
    }

    export class SpriteRenderer {
        Color: math.vec4;
    }
}

declare class AcornFileSystem {
    static FileExists(path: string): boolean;
    static ReadFile(path: string): string | undefined;
    static WriteFile(path: string, content: string): void;
    static CurrentFile?: string;
}

declare const module: any;