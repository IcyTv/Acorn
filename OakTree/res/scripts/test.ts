/// <reference path="./declarations.d.ts" />
class Test extends ScriptSuperClass {
    #rigidBody: Components.RigidBody2d;
    #isPressing = false;
    #initialPosition = new math.vec2(0, 0);
    forceScale = 5;

    OnCreate() {
        this.#rigidBody = this.GetComponent(ComponentTypes.RigidBody2d);
    }

    OnUpdate() {
        if (Input.IsMouseButtonPressed(0) && !this.#isPressing) {
            this.#isPressing = true;
            this.#initialPosition = new math.vec2(Input.GetMouseX(), Input.GetMouseY());
        }
        else if (!Input.IsMouseButtonPressed(0) && this.#isPressing) {
            this.#isPressing = false;
            this.#rigidBody.AddForce(new math.vec2((Input.GetMouseX() - this.#initialPosition.x) * this.forceScale, -(Input.GetMouseY() - this.#initialPosition.y) * this.forceScale));
        }
    }
}

module.exports = Test;