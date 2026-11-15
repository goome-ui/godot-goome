extends GoomeView

const ICONS := ["arrow_down", "arrow_right", "arrow_up", "arrow_left", "check", "radio_dot", "close", "minimize", "maximize", "restore", "resize",
	"sort_up", "sort_down", "plus", "minus", "slider_knob", "toggle_knob", "collapse", "expand", "drag_handle", "toast", "picker"]
const SKINS := [
	[GoomeUI.SKIN_BUTTON, "button", 8, 8, 8, 8], [GoomeUI.SKIN_INPUT, "input", 8, 8, 8, 8], [GoomeUI.SKIN_CHECKBOX, "checkbox", 18, 18, 18, 18],
	[GoomeUI.SKIN_RADIO, "radio", 18, 18, 18, 18], [GoomeUI.SKIN_TOGGLE, "toggle", 38, 22, 38, 22], [GoomeUI.SKIN_SLIDER_TRACK, "slider", 3, 3, 3, 3],
	[GoomeUI.SKIN_SLIDER_FILL, "slider_fill", 3, 3, 3, 3], [GoomeUI.SKIN_SCROLLBAR, "scrollbar", 3, 3, 3, 3], [GoomeUI.SKIN_TAB, "tab", 8, 8, 8, 8],
	[GoomeUI.SKIN_TITLEBAR, "titlebar", 12, 12, 12, 3], [GoomeUI.SKIN_TOOLTIP, "tooltip", 8, 8, 8, 8], [GoomeUI.SKIN_MENU, "menu", 10, 10, 10, 10],
	[GoomeUI.SKIN_POPUP, "menu", 10, 10, 10, 10],
]
const FILES := [["sketch.png", "image", 482], ["notes.txt", "text", 12], ["palette.gpl", "colors", 3], ["cover.jpg", "image", 1210],
	["draft.psd", "layers", 8840], ["todo.md", "text", 4], ["brushes.zip", "archive", 2304]]
const TABS := ["General", "Sound", "About"]
const BRUSHES := ["Round", "Pencil", "Ink pen", "Marker", "Charcoal"]

var dressed := false
var first := true
var tab := 0
var snap := true
var hints := false
var fav := true
var size_pick := 1
var corners := true
var autosave := false
var volume := 64.0
var brush := 0
var tint := Color("e0567a")
var copies := 3.0
var opacity := 0.8
var grid := true
var outline := false
var zoom := 1.0
var inspector := true
var reopen := false
var sort := Vector2i(2, 1)
var file_name := "cozy cat sketch"


func argb(v: int) -> Color:
	return Color8((v >> 16) & 255, (v >> 8) & 255, v & 255, (v >> 24) & 255)


func paper(ui: GoomeUI) -> void:
	var colors := {"bg": 0xFFEFE8DD, "panel": 0xFFFAF6F0, "border": 0x2E2B1E35, "text": 0xFF2B2233, "text_dim": 0xFF85798C, "accent": 0xFF7B4FD0,
		"accent_text": 0xFFFFFFFF, "hover": 0xFFE7DEF0, "active": 0xFFDACDEA, "input_bg": 0xFFEDE6DE, "shadow": 0x3A281838}
	for k in colors:
		ui.theme_set(k, argb(colors[k]))
	var nums := {"radius": 9, "pad": 10, "gap": 9, "border_width": 1, "shadow_blur": 24, "anim_ms": 120}
	for k in nums:
		ui.theme_set(k, nums[k])


func dress(ui: GoomeUI) -> void:
	paper(ui)
	for i in ICONS.size():
		ui.theme_set_icon(i, "res://assets/%s.png" % ICONS[i])
	for s in SKINS:
		ui.theme_set_skin(s[0], "res://assets/%s.png" % s[1], s[2], s[3], s[4], s[5])


func menus(ui: GoomeUI) -> void:
	if not ui.menubar_begin():
		return
	if ui.menu_begin("File"):
		ui.menu_item("New", "Ctrl+N")
		ui.menu_item("Open", "Ctrl+O")
		ui.menu_separator()
		if ui.menu_item("Quit", "Ctrl+Q"):
			get_tree().quit()
		ui.menu_end()
	if ui.menu_begin("View"):
		grid = ui.menu_check("Grid", grid)
		outline = ui.menu_check("Outline", outline)
		if ui.menu_begin("Zoom"):
			if ui.menu_item("50%"):
				zoom = 0.5
			if ui.menu_item("100%"):
				zoom = 1
			if ui.menu_item("200%"):
				zoom = 2
			ui.menu_end()
		var was := inspector
		inspector = ui.menu_check("Inspector", inspector)
		reopen = reopen or (inspector and not was)
		ui.menu_end()
	ui.menubar_end()


func left_pane(ui: GoomeUI) -> void:
	ui.heading("Look")
	tab = ui.tabs("tabs", PackedStringArray(TABS), tab)
	ui.space(2)
	ui.row(2)
	snap = ui.checkbox("Snap to grid", snap)
	hints = ui.checkbox("Show hints", hints)
	ui.push_icon(GoomeUI.ICON_CHECK, "res://assets/heart.png")
	ui.push_color(GoomeUI.S_ACCENT, Color("e0567a"))
	fav = ui.checkbox("Favorite", fav)
	ui.pop(2)
	ui.tooltip("this one gets a heart, pushed with push_icon")
	size_pick = ui.radio("Small", size_pick, 0)
	ui.same_line()
	size_pick = ui.radio("Medium", size_pick, 1)
	ui.same_line()
	size_pick = ui.radio("Large", size_pick, 2)
	ui.row(2)
	corners = ui.toggle("Soft corners", corners)
	autosave = ui.toggle("Autosave", autosave)
	volume = ui.slider("Volume", volume, 0, 100, "%.0f%%")
	ui.row(2)
	brush = ui.combo("Brush", brush, PackedStringArray(BRUSHES))
	tint = ui.color("Tint", tint)
	ui.row(2)
	copies = ui.number("Copies", copies, 1, "%.0f")
	opacity = ui.drag_float("Opacity", opacity, 0.005, 0, 1, "%.2f")
	if ui.collapsing("Layers"):
		if ui.tree("Background"):
			ui.label("paper texture")
			ui.label("warm fill")
			ui.tree_end()
		if ui.tree("Sketch"):
			ui.label("lines")
			ui.tree_end()


func right_pane(ui: GoomeUI) -> void:
	ui.heading("Files")
	var rows := FILES.duplicate()
	if ui.table_begin("files", 3, 0, GoomeUI.TABLE_SORTABLE | GoomeUI.TABLE_STRIPES):
		ui.table_column("Name")
		ui.table_column("Kind", 90)
		ui.table_column("KB", 70)
		sort = ui.table_sort(sort)
		if sort.x >= 0:
			rows.sort_custom(func(a: Array, b: Array) -> bool: return a[sort.x] > b[sort.x] if sort.y else a[sort.x] < b[sort.x])
		for f in rows:
			ui.table_row()
			ui.label(f[0])
			ui.table_next()
			ui.label(f[1])
			ui.table_next()
			ui.label(str(f[2]))
	ui.table_end()
	ui.space(4)
	file_name = ui.input("Name", file_name, 64)
	ui.progress(volume / 100, "exporting")
	ui.row(3)
	if ui.button("Export"):
		ui.toast("exported, check your folder", 2500)
	ui.tooltip("saves every layer as its own png")
	ui.button("Share")
	if ui.button("Inspector"):
		reopen = not inspector
		inspector = true


func inspector_window(ui: GoomeUI) -> void:
	if ui.window("Inspector", Rect2(610, 300, 330, 268), GoomeUI.WIN_MOVE | GoomeUI.WIN_RESIZE | GoomeUI.WIN_COLLAPSE | GoomeUI.WIN_CLOSE):
		ui.label("Brush settings")
		opacity = ui.slider("Flow", opacity, 0, 1, "%.2f")
		snap = ui.checkbox("Pressure", snap)
		ui.separator()
		for i in 14:
			ui.label("stroke %d, %d px" % [i + 1, 2 + i * 3])
	ui.window_end()
	inspector = ui.is_window_open("Inspector")


func _goome_frame(ui: GoomeUI) -> void:
	if not dressed:
		dress(ui)
		dressed = true
	if first:
		ui.toast("skin loaded, every icon is a png now", 4000)
		first = false
	ui.push_skin(GoomeUI.SKIN_TITLEBAR, "res://assets/titlebar_app.png", 4, 4, 4, 3)
	ui.app_begin("Skinned", GoomeUI.APP_TITLEBAR | GoomeUI.APP_MENUBAR | GoomeUI.APP_STATUSBAR | GoomeUI.APP_NO_PAD)
	ui.pop()
	menus(ui)
	ui.split_begin("main")
	left_pane(ui)
	ui.split_next()
	right_pane(ui)
	ui.split_end()
	ui.statusbar("%d files, zoom %d%%, every glyph and part here is a png from assets" % [FILES.size(), int(zoom * 100)])
	ui.app_end()
	if inspector:
		if reopen:
			ui.set_window_open("Inspector", true)
			reopen = false
		inspector_window(ui)
