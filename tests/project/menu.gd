extends GoomeView

var clicks := 0
var name_text := "goome"
var volume := 0.5
var over_button := false
var over_input := false
var over_slider := false
var frames := 0


func _goome_frame(ui: GoomeUI) -> void:
	frames += 1
	if ui.window("Test", Rect2(20, 20, 260, 0), GoomeUI.WIN_MOVE):
		if ui.button("Click me"):
			clicks += 1
		over_button = ui.hovered()
		volume = ui.slider("Volume", volume, 0.0, 1.0)
		over_slider = ui.hovered()
		name_text = ui.input("Name", name_text)
		over_input = ui.hovered()
		ui.label("clicked %d times" % clicks)
	ui.window_end()
