/// <reference path="./declarations.d.ts" />

class Test extends ScriptSuperClass {
    testVar = "Test";
    print: boolean = false;
    movementSpeed: number = 0.1;
    #transformComponent: Components.Transform;

    get value() {
        return this.testVar;
    }

    OnCreate() {
        this.#transformComponent = super.GetComponent(ComponentTypes.Transform);
    }

    OnUpdate(timestep: number) {
        if (Input.IsKeyPressed("W")) {
            this.#transformComponent.Position.y += this.movementSpeed * timestep;
        }
        if (Input.IsKeyPressed("S")) {
            this.#transformComponent.Position.y -= this.movementSpeed * timestep;
        }
        if (Input.IsKeyPressed("A")) {
            this.#transformComponent.Position.x -= this.movementSpeed * timestep;
        }
        if (Input.IsKeyPressed("D")) {
            this.#transformComponent.Position.x += this.movementSpeed * timestep;
        }
    }
}

module.exports = Test;