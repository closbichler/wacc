const canvas = document.getElementById("app");
const ctx = canvas.getContext("2d");
var wasm;
var memoryBuffer;

const fps = 40;
const fpsInterval = 1000 / fps;
var prev = 0;

function toColor(num) {
  num >>>= 0;
  var b = num & 0xff,
    g = (num & 0xff00) >>> 8,
    r = (num & 0xff0000) >>> 16;
  return "rgb(" + [r, g, b].join(",") + ")";
}

function cstrLen(mem, ptr) {
  let len = 0;
  while (mem[ptr] != 0) {
    len++;
    ptr++;
  }
  return len;
}

function cstrToJsstr(mem_buffer, ptr) {
  const mem = new Uint8Array(mem_buffer);
  const len = cstrLen(mem, ptr);
  const bytes = new Uint8Array(mem_buffer, ptr, len);
  return new TextDecoder().decode(bytes);
}

function rand(min, max) {
  return Math.floor(Math.random() * max) + min; 
}

function fill_rect(x, y, w, h, color) {
  ctx.fillStyle = toColor(color);
  ctx.fillRect(x, y, w, h);
}

function clear_rect(x, y, w, h) {
  ctx.clearRect(x, y, w, h);
}

function fill_text(x, y, textPtr, size, color) {
  const text = cstrToJsstr(memoryBuffer, textPtr);
  ctx.font = size + "px Arial";
  ctx.fillStyle = toColor(color);
  ctx.fillText(text, x, y);
}

function fill_text_centered(x, y, textPtr, size, color) {
  const text = cstrToJsstr(memoryBuffer, textPtr);
  ctx.font = size + "px Arial";
  ctx.fillStyle = toColor(color);
  ctx.fillText(text, x - text.length*size/2, y);
}

function log_console(textPtr) {
  const text = cstrToJsstr(memoryBuffer, textPtr);
  console.log(text);
}

function loop(timestamp) {
  elapsed = timestamp - prev;

  if (elapsed > fpsInterval) {
    wasm.instance.exports.game_update(elapsed / 1000 * 60);
    wasm.instance.exports.game_render();
    prev = timestamp;
  }

  window.requestAnimationFrame(loop);
}

async function init() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;

  wasm = await WebAssembly.instantiateStreaming(fetch("./game.wasm"), {
    env: { fill_rect, clear_rect, fill_text, fill_text_centered, log_console, rand },
  });
  memoryBuffer = wasm.instance.exports.memory.buffer;

  document.onmousedown = (e) => {
    let x = e.pageX, y = e.pageY;
    wasm.instance.exports.on_mouse_down(x, y);
  };

  wasm.instance.exports.game_init(canvas.width, canvas.height, window.devicePixelRatio );
  window.requestAnimationFrame(loop);
}
init();
