/// <reference path="./declarations.d.ts" />

import { Vector2 } from "@math.gl/core";

class TestScript {
	onCreate() {
		let v = new Vector2(1, 2);
		let test = GetComponent(ComponentTypes.Transform);
		print(test.Translation);
	}

	onUpdate(ts) {
	}
}

export default TestScript;