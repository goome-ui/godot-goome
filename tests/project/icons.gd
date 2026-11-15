extends GoomeView

var mode := 0
var dressed := false
var tint := 0
var heart_src: Variant = null
var icons: Array[Image] = []
var heart: Image
var skin: Image
var check := true
var check2 := false
var radio := 1
var on := true
var sel := 1
var tab := 0
var val := 0.4
var num := 3.0
var tint_seen: Variant = null


static func icon_image(i: int) -> Image:
	var data := PackedByteArray()
	data.resize(16 * 16 * 4)
	for y in 16:
		for x in 16:
			var dx := x - 8
			var dy := y - 8
			var v := (x * 7 + y * 13 + i * 29) & 255
			var a := 255 if dx * dx + dy * dy < 40 + i else (120 if (x + y + i) % 5 == 0 else 0)
			var o := (y * 16 + x) * 4
			data[o] = v
			data[o + 1] = 255 - v
			data[o + 2] = (i * 11) & 255
			data[o + 3] = a
	return Image.create_from_data(16, 16, false, Image.FORMAT_RGBA8, data)


static func skin_image() -> Image:
	var data := PackedByteArray()
	for y in 12:
		for x in 12:
			var edge := x < 3 or y < 3 or x > 8 or y > 8
			data.append_array(PackedByteArray([0x7B, 0x4F, 0xD0, 0xFF]) if edge else PackedByteArray([0xF0, 0xE8, 0xFF, 0xFF]))
	return Image.create_from_data(12, 12, false, Image.FORMAT_RGBA8, data)


func _init() -> void:
	for i in GoomeUI.ICON_PICKER + 1:
		icons.append(icon_image(i))
	heart = icon_image(40)
	heart_src = heart
	skin = skin_image()
	redraw_every_frame = true


func dress(ui: GoomeUI) -> void:
	for i in icons.size():
		ui.theme_set_icon(i, icons[i] if dressed else null)
	for k in range(GoomeUI.SKIN_CHECKBOX, GoomeUI.SKIN_POPUP + 1):
		ui.theme_set_skin(k, skin if dressed else null, 3, 3, 3, 3)
	ui.theme_set("icon_tint", tint)
	tint_seen = ui.theme_get("icon_tint")


func _goome_frame(ui: GoomeUI) -> void:
	ui.theme_set("anim_ms", 0)
	ui.theme_set("font_size", 14)
	dress(ui)
	ui.push_font(null)
	if ui.window("icons", Rect2(6, 6, 288, 248), GoomeUI.WIN_MOVE | GoomeUI.WIN_RESIZE | GoomeUI.WIN_COLLAPSE | GoomeUI.WIN_CLOSE):
		if mode == 3:
			ui.push_icon(GoomeUI.ICON_CHECK, heart_src)
			ui.push_skin(GoomeUI.SKIN_CHECKBOX, skin, 3, 3, 3, 3)
		if mode == 4:
			ui.theme_set_icon(GoomeUI.ICON_CHECK, heart)
			ui.theme_set_skin(GoomeUI.SKIN_CHECKBOX, skin, 3, 3, 3, 3)
		check = ui.checkbox("Check", check)
		if mode == 3:
			ui.pop(2)
		if mode == 4:
			ui.theme_set_icon(GoomeUI.ICON_CHECK, null)
			ui.theme_set_skin(GoomeUI.SKIN_CHECKBOX, null)
		check2 = ui.checkbox("Other", check2)
		radio = ui.radio("A", radio, 0)
		ui.same_line()
		radio = ui.radio("B", radio, 1)
		on = ui.toggle("On", on)
		val = ui.slider("Val", val, 0.0, 1.0, "%.2f")
		sel = ui.combo("Pick", sel, PackedStringArray(["one", "two", "three"]))
		tab = ui.tabs("t", PackedStringArray(["One", "Two"]), tab)
		num = ui.number("N", num, 1, "%.0f")
		if ui.collapsing("More"):
			ui.label("inside")
	ui.window_end()
	ui.pop(1)
