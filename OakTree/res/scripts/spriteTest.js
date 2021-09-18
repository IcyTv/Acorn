/// <reference path="./declarations.d.ts" />

class SpriteTest extends ScriptSuperClass {
    #spriteRenderer;
    #color;

    OnCreate() {
        this.#spriteRenderer = super.GetComponent(ComponentTypes.SpriteRenderer);
        this.#color = new math.vec3(0, 0, 0);
    }

    OnUpdate(timestep) {
        // print("onUpdate");

        this.#spriteRenderer.Color.x += timestep;
        this.#spriteRenderer.Color.x %= 1;
        this.#spriteRenderer.Color.y += timestep * 0.5;
        this.#spriteRenderer.Color.y %= 1;
        this.#spriteRenderer.Color.z += timestep * 0.1;
        this.#spriteRenderer.Color.z %= 1;

        // this.#color.x += timestep;
        // this.#color.x %= 1;
        // this.#color.y += timestep * 0.5;
        // this.#color.y %= 1;
        // this.#color.z += timestep * 0.1;
        // this.#color.z %= 1;

        // print(this.#color.x);
        // print(this.#spriteRenderer.Color.x);

        // this.#spriteRenderer.Color = this.#color;

        // print(this.#spriteRenderer.Color.x);
    }
}

module.exports = SpriteTest;