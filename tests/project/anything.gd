extends GoomeView

const MAKES := [[Color(0x8B5CF6FF), true, 12.0], [Color(0x2F7BF6FF), false, 20.0], [Color(0xFFD60AFF), true, 0.0], [Color(0xE5484D00), false, 6.0]]

var scene := "kit"
var preset := -1
var make := -1
var focused := false
var buf := "hello you"
var ed := "goome HiAVWTy"
var flags := 0
var hint := ""
var set_focus_once := false
var got_enter := false
var on := true
var rad := 1
var val := 0.4
var tab := 0
var ready_frames := 0


func _init() -> void:
	redraw_every_frame = true
	sync_clipboard = false
	pixel_scale = 1


func setup(ui: GoomeUI) -> void:
	ui.theme_set("font", null)
	ui.theme_set("font_size", 14)
	ui.theme_set("anim_ms", 0)
	ui.config_set("blink_ms", 0)


func _goome_frame(ui: GoomeUI) -> void:
	if ready_frames == 0:
		setup(ui)
	ready_frames += 1
	if scene == "input":
		ui.button("Hi")
		if set_focus_once:
			ui.set_focus()
			set_focus_once = false
		buf = ui.input_ex("in", buf, 23, flags, hint)
		if ui.entered():
			got_enter = true
		return
	var pushed := 0
	if preset >= 0:
		ui.push_preset(preset, {"font_size": 14})
		pushed = 1
	elif make >= 0:
		var m: Array = MAKES[make]
		ui.push_theme_make(m[0], m[1], m[2], {"font_size": 14})
		pushed = 1
	if ui.window("kit", Rect2(10, 10, 300, 680), GoomeUI.WIN_RESIZE):
		if ui.tree("tree", true):
			ui.label("leaf")
			ui.tree_end()
		ui.plot_lines("p", PackedFloat32Array([1, 3, 2, 5, 4, 6]), 0, 7, 40)
		ed = ui.text_edit("ed", ed, 40, GoomeUI.EDIT_MONO)
		ui.row(2)
		ui.link("goome")
		ui.badge("Hi", Color(0, 0, 0, 0))
		tab = ui.tabs("t", PackedStringArray(["A", "B", "C"]), tab)
		ui.heading("goome Hi")
		ui.label("goome")
		ui.button("Hi")
		on = ui.checkbox("c", on)
		on = ui.toggle("t", on)
		rad = ui.radio("r", rad, 1)
		val = ui.slider("s", val, 0, 1, "%.1f")
		if not focused:
			ui.set_focus()
			focused = true
		buf = ui.input("in", buf, 23)
		ui.separator()
		for i in 40:
			ui.label("row %d" % i)
	ui.window_end()
	if pushed:
		ui.pop(pushed)
