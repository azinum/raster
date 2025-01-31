// module.js

let wasm = null;

const KEY_UNDEFINED = 0;
const KEY_SPACE = 1;
const KEY_LEFT_ARROW = 2;
const KEY_UP_ARROW = 3;
const KEY_RIGHT_ARROW = 4;
const KEY_DOWN_ARROW = 5;
const KEY_0 = 6;
const KEY_1 = 7;
const KEY_2 = 8;
const KEY_3 = 9;
const KEY_4 = 10;
const KEY_5 = 11;
const KEY_6 = 12;
const KEY_7 = 13;
const KEY_8 = 14;
const KEY_9 = 15;
const KEY_A = 16;
const KEY_B = 17;
const KEY_C = 18;
const KEY_D = 19;
const KEY_E = 20;
const KEY_F = 21;
const KEY_G = 22;
const KEY_H = 23;
const KEY_I = 24;
const KEY_J = 25;
const KEY_K = 26;
const KEY_L = 27;
const KEY_M = 28;
const KEY_N = 29;
const KEY_O = 30;
const KEY_P = 31;
const KEY_Q = 32;
const KEY_R = 33;
const KEY_S = 34;
const KEY_T = 35;
const KEY_U = 36;
const KEY_V = 37;
const KEY_W = 38;
const KEY_X = 39;
const KEY_Y = 40;
const KEY_Z = 41;

const KEY_EVENT_DOWN = 0;
const KEY_EVENT_UP = 1;

// map from javascript key codes to a standardized key map
const keyMap = [
	KEY_SPACE,
	KEY_UNDEFINED,  // page up
	KEY_UNDEFINED,  // page down
	KEY_UNDEFINED,  // end
	KEY_UNDEFINED,  // home
	KEY_LEFT_ARROW,
	KEY_UP_ARROW,
	KEY_RIGHT_ARROW,
	KEY_DOWN_ARROW,
	KEY_UNDEFINED, // unsued
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // print screen
	KEY_UNDEFINED, // insert
	KEY_UNDEFINED, // delete
	KEY_UNDEFINED, // unused
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_UNDEFINED, // unused
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
];

let options = {
	rasterWidth: 0,
	rasterHeight: 0,
	displayWidth: 0,
	displayHeight: 0,
}

function strLength(memory, ptr) {
	let length = 0;
	while (memory[ptr] != 0) {
		length++;
		ptr++;
	}
	return length;
}

function strFromPointer(memoryBuffer, ptr) {
	const memory = new Uint8Array(memoryBuffer);
	const length = strLength(memory, ptr);
	const bytes = new Uint8Array(memoryBuffer, ptr, length);
	return new TextDecoder().decode(bytes);
}

function write(fd, messagePtr, length) {
	const buffer = wasm.instance.exports.memory.buffer;
	const message = strFromPointer(buffer, messagePtr);
	console.log(message);
}

function fullscreen() {
	const canvas = document.getElementById("main-canvas");
	if (canvas.requestFullscreen) {
		canvas.requestFullscreen();
	}
	else if (canvas.webkitRequestFullScreen) {
		canvas.webkitRequestFullScreen();
	}
	else if (canvas.mozRequestFullScreen) {
		canvas.mozRequestFullScreen();
	}
}

(async function init() {
	WebAssembly.instantiateStreaming(fetch("raster.wasm"), {
		env: {
			write,
			sinf: Math.sin,
			cosf: Math.cos,
			sqrtf: Math.sqrt,
			tanf: Math.tan,
			expf: Math.exp,
			logf: Math.log,
			ceilf: Math.ceil,
			floorf: Math.floor,
			roundf: Math.round,
			time: Date.now,
			window_toggle_fullscreen: fullscreen
		}
	})
	.then(obj => {
		wasm = obj;
		const instance = obj.instance;
		const memoryView = new Uint8Array(obj.instance.exports.memory.buffer);
		obj.instance.exports.wasm_main();
		const displayAddress = instance.exports.display_get_addr();
		const width = instance.exports.display_get_width();
		const height = instance.exports.display_get_height();
		const displaySize = width * height;

		const canvas = document.getElementById("main-canvas");
		const context = canvas.getContext("2d");
		canvas.width = width;
		canvas.height = height;
		context.imageSmoothingEnabled = false;
		options.rasterWidth = width;
		options.rasterHeight = height;
		options.displayWidth = canvas.clientWidth;
		options.displayHeight = canvas.clientHeight;
		let events = [];

		document.addEventListener("keydown", e => {
			if (e.repeat) {
				return;
			}
			if (e.which >= 32 && e.which <= 90) {
				events.push({ code: keyMap[e.which - 32], type: KEY_EVENT_DOWN });
			}
		});
		document.addEventListener("keyup", e => {
			if (e.repeat) {
				return;
			}
			if (e.which >= 32 && e.which <= 90) {
				events.push({ code: keyMap[e.which - 32], type: KEY_EVENT_UP });
			}
		});
		canvas.addEventListener("mousemove", e => {
			const bounds = e.target.getBoundingClientRect();
			let x = e.clientX - bounds.left;
			let y = e.clientY - bounds.top;
			wasm.instance.exports.input_mouse_move(x, y);
		});
		canvas.addEventListener("mousedown", e => {
			const bounds = e.target.getBoundingClientRect();
			let x = e.clientX - bounds.left;
			let y = e.clientY - bounds.top;
			instance.exports.input_mouse_left_click(x, y);

		});
		canvas.addEventListener("mouseup", e => {
			const bounds = e.target.getBoundingClientRect();
			let x = e.clientX - bounds.left;
			let y = e.clientY - bounds.top;
			instance.exports.input_mouse_left_release(x, y);
		});

		function processInputEvents() {
			for (let i = 0; i < events.length; ++i) {
				let e = events[i];
				instance.exports.input_event(e.code, e.type);
			}
			events.length = 0;
		}

		let prevTime = 0;
		let currentTime = performance.now();
		function frame_step(timestamp) {
			prevTime = currentTime;
			currentTime = performance.now();
			const dt = (currentTime - prevTime) * 0.001;
			instance.exports.clear_input_events();
			processInputEvents();
			instance.exports.update_and_render(dt);
			const frame = new ImageData(
				new Uint8ClampedArray(
					memoryView.subarray(
						displayAddress,
						displayAddress + 4 * displaySize
					)
				),
				width, height
			);
			context.putImageData(frame, 0, 0);
			window.requestAnimationFrame(frame_step);
		}
		window.requestAnimationFrame(frame_step);
	});
})().catch((e) => console.error(e));
