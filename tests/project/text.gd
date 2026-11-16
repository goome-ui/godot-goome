extends GoomeView

const S := {
	"mix": "616220f09f988020f09f9880f09f8fbb20f09f9880e2808df09f988020e29da420e29da4efb88f",
	"cat": "f09f90b1",
	"cats": "f09f90b1f09f90b1",
	"tone": "f09f9880f09f8fbb",
	"zwj": "f09f9880e2808df09f9880",
	"heart": "e29da4",
	"heart_vs": "e29da4efb88f",
	"goome": "676f6f6d652120f09f9880",
	"goome_hi": "676f6f6d652120f09f9880204869",
	"heb": "616220d790d791d79220283132292063",
	"arab_short": "d8a8d984d8a7d98520d8a8d8a8d8a820d984d985",
	"arab": "d8a8d984d8a7d98520d8a8d8a8d8a820d984d98520d8a8d98ed8a8",
	"arab_apart": "d8a820d984d8a7d98520d8a820d8a820d8a820d984d985",
}

var fonts := {}
var jobs := []
var n := 0
var hashes := []
var sizes := {}
var mid := Color()


func _init() -> void:
	redraw_every_frame = true
	sync_clipboard = false
	pixel_scale = 1
	for f in ["colr", "cbdt", "sbix", "sub"]:
		fonts[f] = FileAccess.get_file_as_bytes("res://fonts/%s.bin" % f)
	var white := Color(1, 1, 1)
	jobs = [
		[["colr", 30, "mix", Color(0x2040A0FF), 0, Vector2(4, 4), 60]],
		[["cbdt", 16, "cat", white, 0, Vector2(4, 4), 0], ["cbdt", 64, "cat", white, 0, Vector2(30, 4), 0], ["sbix", 40, "cats", Color(1, 1, 1, 128 / 255.0), 0, Vector2(120, 4), 0]],
		[["sub", 20, "goome_hi", white, 0, Vector2(4, 4), 40]],
		[["colr", 24, "heb", white, 0, Vector2(4, 4), 40]],
		[["colr", 24, "heb", white, GoomeUI.DIR_RTL, Vector2(4, 4), 40]],
		[["colr", 24, "heb", white, GoomeUI.DIR_LTR, Vector2(4, 4), 40]],
		[["colr", 30, "arab", white, 0, Vector2(4, 4), 50]],
		[["colr", 30, "arab_apart", white, 0, Vector2(4, 4), 50]],
	]


func text(k: String) -> String:
	return S[k].hex_decode().get_string_from_utf8()


func argb_fnv(img: Image) -> String:
	var d := img.get_data()
	var h := 2166136261
	var i := 0
	while i < d.size():
		h = ((h ^ d[i + 2]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i + 1]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i + 3]) * 16777619) & 0xFFFFFFFF
		i += 4
	return "%08x" % h


func _goome_frame(ui: GoomeUI) -> void:
	if n > 0 and n <= jobs.size():
		var img := get_image()
		hashes.append(argb_fnv(img))
		if n == 2:
			mid = img.get_pixel(65, 44)
	if n == 0:
		for k in ["tone", "zwj", "heart", "heart_vs"]:
			sizes[k] = ui.text_size(text(k), 30, fonts.colr).x
		sizes.arab_short = ui.text_size(text("arab_short"), 20, fonts.colr).x
	if n == 2:
		ui.font_fallback(fonts.sub, fonts.colr)
		ui.font_fallback(fonts.sub, fonts.sub)
		sizes.goome = ui.text_size(text("goome"), 20, fonts.sub).x
		sizes.goome_plain = ui.text_size("goome! ", 20, fonts.sub).x
	if n < jobs.size():
		for j in jobs[n]:
			ui.text_dir(j[4])
			if j[6]:
				ui.push_clip(Rect2(0, 0, 400, j[6]))
			ui.draw_text(j[5], text(j[2]), j[3], j[1], fonts[j[0]])
			if j[6]:
				ui.pop_clip()
			ui.text_dir(GoomeUI.DIR_AUTO)
	n += 1
