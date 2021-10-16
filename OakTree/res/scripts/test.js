/// <reference path="./declarations.d.ts" />
let Test = class Test extends ScriptSuperClass {
    #rigidBody;
    #isPressing = false;
    #initialPosition = new math.vec2(0, 0);
    OnCreate() {
        this.#rigidBody = this.GetComponent(ComponentTypes.RigidBody2d);
    }
    OnUpdate() {
        if (Input.IsMouseButtonPressed(0) && !this.#isPressing) {
            this.#isPressing = true;
            this.#initialPosition = new math.vec2(Input.GetMouseX(), Input.GetMouseY());
        } else if (!Input.IsMouseButtonPressed(0) && this.#isPressing) {
            this.#isPressing = false;
            this.#rigidBody.AddForce(new math.vec2((Input.GetMouseX() - this.#initialPosition.x) * this.forceScale, -(Input.GetMouseY() - this.#initialPosition.y) * this.forceScale));
        }
    }
    constructor(...args){
        super(...args);
        this.forceScale = 5;
        this.test = false;
    }
};
let a = Test;
module.exports = Test;
