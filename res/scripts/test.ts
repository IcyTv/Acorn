/// <reference path="./declarations.d.ts" />
class Test extends ScriptSuperClass {
	#rigidBody: Components.RigidBody2d;
	#spriteRenderer: Components.SpriteRenderer;
	#boxCollider: Components.BoxCollider2d;
	#origColor: math.vec4;
	#isPressing = false;
	#initialPosition = new math.vec2(0, 0);
	test2 = 6;
	forceScale: number = 5;
	test: boolean = false;

	OnCreate() {
		this.#rigidBody = this.GetComponent(ComponentTypes.RigidBody2d);
		this.#spriteRenderer = this.GetComponent(ComponentTypes.SpriteRenderer);
		this.#boxCollider = this.GetComponent(ComponentTypes.BoxCollider2d);
		this.#origColor = this.#spriteRenderer.Color;
	}

	OnUpdate() {
		let [x, y] = Input.GetMousePosition();
		print(`Mouse position: ${x}, ${y}`);
		// if (this.#boxCollider.IsInside(new math.vec2(x, y))) {
		// 	print("Is Inside!");
		// 	this.#spriteRenderer.Color = new math.vec4(1, 0, 0, 1);
		// }
		// else {
		// 	this.#spriteRenderer.Color = this.#origColor;
		// }

		// if (Input.IsMouseButtonPressed(0) && !this.#isPressing) {
		//     this.#isPressing = true;
		//     this.#initialPosition = new math.vec2(Input.GetMouseX(), Input.GetMouseY());
		// }
		// else if (!Input.IsMouseButtonPressed(0) && this.#isPressing) {
		//     this.#isPressing = false;
		//     this.#rigidBody.AddForce(new math.vec2((Input.GetMouseX() - this.#initialPosition.x) * this.forceScale, -(Input.GetMouseY() - this.#initialPosition.y) * this.forceScale));
		// }
	}
}

let a = Test;

module.exports = Test;