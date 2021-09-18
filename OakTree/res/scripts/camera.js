/// <reference path="./declarations.d.ts" />
class Test extends ScriptSuperClass {
    testVar = "Test";
    print = false;
    movementSpeed = 0.1;
    #transformComponent;
    get value() {
        return this.testVar;
    }
    OnCreate() {
        this.#transformComponent = super.GetComponent(ComponentTypes.Transform);
    }
    OnUpdate(timestep) {
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
