extends GoomeView

var in_text := ""
var ed_text := ""
var focus := 0
var flags := 0
var actions := []
var n := 0
var log := []
var texts := []
var preedits := []


func _init() -> void:
	redraw_every_frame = true
	sync_clipboard = false
	pixel_scale = 1
	ime_text.connect(func(t: String) -> void: texts.append(t))
	ime_preedit.connect(func(t: String, caret: int) -> void: preedits.append([t, caret]))


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
	if n == 0:
		ui.theme_set("font", null)
		ui.theme_set("font_size", 14)
		ui.theme_set("anim_ms", 0)
		ui.config_set("blink_ms", 0)
	if n > 0:
		log[n - 1].hash = argb_fnv(get_image())
	if n == 0 and focus == 1:
		ui.set_focus()
	in_text = ui.input_ex("in", in_text, 63, GoomeUI.INPUT_NO_SELECT_ON_FOCUS | flags)
	if n == 0 and focus == 2:
		ui.set_focus()
	ed_text = ui.text_edit("ed", ed_text, 80)
	log.append({"in": in_text, "ed": ed_text, "hash": ""})
	if n >= 1 and actions.size():
		var a: Array = actions.pop_front()
		match a[0]:
			"pre":
				push_preedit(a[1], a[2])
			"text":
				push_ime_text(a[1])
			"key":
				push_key(a[1], a[2])
			"flags":
				flags = a[1]
	n += 1
