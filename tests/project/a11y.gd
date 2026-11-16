extends GoomeView

var ok := 0
var dark := false
var mode := 0
var fruit := 0
var tab := 0
var vol := 2.0
var name_text := "Bo"
var note := ""
var custom_hits := 0
var n := 0


func _init() -> void:
	redraw_every_frame = true
	sync_clipboard = false
	pixel_scale = 1


func _goome_frame(ui: GoomeUI) -> void:
	if n == 0:
		ui.theme_set("font", null)
		ui.a11y(true)
	n += 1
	ui.app_begin("Demo", GoomeUI.APP_TITLEBAR | GoomeUI.APP_STATUSBAR)
	ui.label("Hello there")
	if ui.button("OK"):
		ok += 1
	dark = ui.checkbox("Dark", dark)
	mode = ui.radio("Big", mode, 1)
	vol = ui.slider("Volume", vol, 0, 10, "%.1f")
	name_text = ui.input("Name", name_text)
	fruit = ui.combo("Fruit", fruit, PackedStringArray(["Mango", "Kiwi", "Fig"]))
	tab = ui.tabs("tabs", PackedStringArray(["One", "Two"]), tab)
	if ui.tree("Folder"):
		ui.label("inside")
		ui.tree_end()
	ui.progress(0.25)
	note = ui.text_edit("Notes", note, 60)
	ui.a11y_role(GoomeUI.ROLE_BUTTON, "Knob")
	ui.a11y_value("11")
	if ui.custom("knob", 40, 40).clicked:
		custom_hits += 1
	ui.statusbar("ready")
	ui.app_end()
