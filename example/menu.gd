extends GoomeView

@onready var game: Node2D = get_node("../..")

var shown := true
var tab := 0
var mode := 0
var name_text := "goome"
var notes := "you can type here.\nclicks next to the windows go to the game, try it."
var fruit := 0
var fruits := PackedStringArray(["Mango", "Banana", "Cherry", "Kiwi"])
var picked := -1
var sort := Vector2i(0, 0)
var rows := [["Mango", 3, 1.5], ["Banana", 12, 0.25], ["Cherry", 40, 4.0], ["Kiwi", 7, 0.6]]
var history := PackedFloat32Array()


func _unhandled_key_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and event.keycode == KEY_F1:
		shown = not shown
		redraw()


func _goome_frame(ui: GoomeUI) -> void:
	if not shown:
		return
	history.append(Engine.get_frames_per_second())
	if history.size() > 60:
		history.remove_at(0)

	if ui.window("goome in godot", Rect2(24, 24, 340, 0), GoomeUI.WIN_MOVE | GoomeUI.WIN_RESIZE | GoomeUI.WIN_COLLAPSE):
		tab = ui.tabs("tabs", PackedStringArray(["Game", "Widgets", "Table"]), tab)
		ui.push_id(str(tab))
		if tab == 0:
			ui.heading("The square")
			game.spin = ui.slider("Spin", game.spin, -6.0, 6.0, "%.1f")
			game.size = ui.slider("Size", game.size, 20.0, 300.0, "%.0f px")
			game.color = ui.color("Color", game.color)
			if ui.button("Clear dots"):
				game.dots.clear()
				ui.toast("dots gone")
			ui.wrapped("WASD moves the square, F1 hides this. Typing in a text box doesnt move it.")
			ui.plot_lines("fps", history, 0.0, 0.0, 50.0)
		elif tab == 1:
			ui.heading("Widgets")
			name_text = ui.input("Name", name_text)
			ui.label("hi %s" % name_text)
			fruit = ui.combo("Fruit", fruit, fruits)
			ui.row(3)
			mode = ui.radio("Small", mode, 0)
			mode = ui.radio("Medium", mode, 1)
			mode = ui.radio("Large", mode, 2)
			if ui.collapsing("Notes"):
				notes = ui.text_edit("notes", notes, 120.0)
			if ui.button("About"):
				ui.modal_open("About")
		else:
			if ui.table_begin("fruit", 3, 160.0, GoomeUI.TABLE_SORTABLE | GoomeUI.TABLE_STRIPES):
				ui.table_column("Fruit")
				ui.table_column("Count", 70.0)
				ui.table_column("Price", 70.0)
				sort = ui.table_sort(sort)
				if ui.changed():
					rows.sort_custom(func(a, b): return a[sort.x] > b[sort.x] if sort.y else a[sort.x] < b[sort.x])
				for r in rows:
					ui.table_row()
					ui.label(r[0])
					ui.table_next()
					ui.label(str(r[1]))
					ui.table_next()
					ui.label("%.2f" % r[2])
			ui.table_end()
		ui.pop_id()
	ui.window_end()

	if ui.modal_begin("About"):
		ui.label("made with goome, drawn by the godot-goome addon")
		if ui.button("Close"):
			ui.popup_close()
		ui.modal_end()
