let canvas = document.getElementById("app")
let ctx = canvas.getContext("2d")
let wasm = 0

let prev = 0
let fps = 30
let fpsInterval = 1000 / fps

function toColor(num) {
  num >>>= 0
  var b = num & 0xFF,
      g = (num & 0xFF00) >>> 8,
      r = (num & 0xFF0000) >>> 16;
  return "rgb(" + [r, g, b].join(",") + ")";
}

function fill_rect(w, h, x, y, color) {
  ctx.fillStyle = toColor(color)
  ctx.fillRect(x, y, w, h)
}

function fill_text(text, x, y, color) {
  ctx.fillStyle = toColor(color)
  ctx.fillText(text, x, y)
}

function loop(now) {
  elapsed = now - prev
  
  if (elapsed > fpsInterval) {
    wasm.instance.exports.game_update((now - prev) * 0.001)
    wasm.instance.exports.game_render()
    prev = now - (elapsed % fpsInterval)
  }

  window.requestAnimationFrame(loop)
}

async function init() {
  wasm = await WebAssembly.instantiateStreaming(
    fetch("./game.wasm"),
    {
      env: { fill_rect, fill_text } 
    }
  )

  wasm.instance.exports.game_init(canvas.width, canvas.height)
  window.requestAnimationFrame(loop)
}
init()
