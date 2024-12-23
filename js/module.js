// module.js

(async function init() {
	fetch('game.wasm')
	.then(res => res.arrayBuffer())
	.then(bytes => WebAssembly.instantiate(bytes))
	.then(obj => {
		const instance = obj.instance;
		const memoryView = new Uint8Array(obj.instance.exports.memory.buffer);
		instance.exports.init();
		const displayAddress = instance.exports.display_get_addr();
		const width = instance.exports.display_get_width();
		const height = instance.exports.display_get_height();
		const displaySize = width * height;

		const canvas = document.getElementById("main-canvas");
		const context = canvas.getContext("2d");
		canvas.width = width;
		canvas.height = height;
		canvas.addEventListener("mousedown", e => {
			instance.exports.mouse_click(e.offsetX, e.offsetY);
		});
		document.addEventListener("keydown", e => {
			if (e.which >= 32 && e.which <= 90) {
				instance.exports.input_event(key_map[e.which - 32]);
			}
		});

		let startTime = 0;
		function frame_step(timestamp) {
			if (startTime === undefined) {
				startTime = timestamp;
			}
			const dt = (timestamp - startTime) * 0.001;
			startTime = timestamp;
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
