extends GoomeView

const RETRO := 0
const SOFT := 1
const HUD := 2
const DENSE := 3
const STARTS := ["theme_make", "Dark", "Light", "Midnight", "Paper", "Mint", "High contrast", "Rose"]
const CODE := ["ui.theme_make(Color(\"8b5cf6\"), true, 12)", "ui.push_theme_make(Color(\"8b5cf6\"), true, 12)", "ui.push_preset(GoomeUI.PRESET_MIDNIGHT)"]
const NAMES := ["guest_01", "Mia", "NOVA-7", "render_pass_b", "sev"]

var cards := []
var looks := []
var art := {}
var start := 0
var zoom := 1.0
var focus_once := true
var pass_text := "hunter22"
var code := ""
var server := "eu-west-2"
var configured := false


func argb(v: int) -> Color:
	return Color8((v >> 16) & 255, (v >> 8) & 255, v & 255, (v >> 24) & 255)


func blank(w: int, h: int) -> Image:
	return Image.create_empty(w, h, false, Image.FORMAT_RGBA8)


func box(w: int, h: int, rects: Array) -> Image:
	var img := blank(w, h)
	for r in rects:
		img.fill_rect(Rect2i(r[0], r[1], r[2], r[3]), argb(r[4]))
	return img


func diamond(img: Image, r: float, col: int) -> Image:
	var c := Vector2(img.get_width(), img.get_height()) / 2.0
	for y in img.get_height():
		for x in img.get_width():
			if absf(x + 0.5 - c.x) + absf(y + 0.5 - c.y) <= r:
				img.set_pixel(x, y, argb(col))
	return img


func arrow(down: bool) -> Image:
	var img := blank(12, 12)
	for i in 4:
		if down:
			img.fill_rect(Rect2i(2 + i, 3 + i * 2, 8 - i * 2, 2), Color.WHITE)
		else:
			img.fill_rect(Rect2i(3 + i * 2, 2 + i, 2, 8 - i * 2), Color.WHITE)
	return img


func cross() -> Image:
	var img := blank(12, 12)
	for i in range(2, 10):
		img.fill_rect(Rect2i(i, i, 1, 1), Color.WHITE)
		img.fill_rect(Rect2i(11 - i, i, 1, 1), Color.WHITE)
	return img


func make_art() -> void:
	art.term_check = box(12, 12, [[2, 2, 8, 8, 0xFFFFFFFF]])
	art.term_dot = box(10, 10, [[2, 2, 6, 6, 0xFFFFFFFF]])
	art.term_knob = box(12, 12, [[0, 0, 12, 12, 0xFFFFFFFF]])
	art.term_toggle = box(12, 12, [[0, 0, 12, 12, 0xFFFFFFFF], [3, 3, 6, 6, 0xFF000000]])
	art.term_box = box(12, 12, [[0, 0, 12, 12, 0xFF1FA64C], [1, 1, 10, 10, 0xFF031408]])
	art.hud_check = diamond(blank(16, 16), 6, 0xFFFFFFFF)
	art.hud_dot = diamond(blank(16, 16), 5, 0xFFFFFFFF)
	art.hud_knob = diamond(diamond(blank(20, 20), 9, 0xFFFFFFFF), 5.5, 0xFF0A1A24)
	art.hud_box = box(18, 18, [[0, 0, 18, 18, 0xFF39E1FF], [1, 1, 16, 16, 0xC0081C28]])
	art.hud_radio = diamond(diamond(blank(18, 18), 8.6, 0xFF39E1FF), 7.2, 0xF0081C28)
	art.frame = box(24, 24, [[0, 0, 24, 24, 0x5539E1FF], [1, 1, 22, 22, 0xA00A1622], [0, 0, 8, 2, 0xFF39E1FF], [0, 0, 2, 8, 0xFF39E1FF],
		[16, 22, 8, 2, 0xFF39E1FF], [22, 16, 2, 8, 0xFF39E1FF]])
	art.plate = box(24, 24, [[0, 0, 24, 24, 0xFF39E1FF], [1, 1, 22, 22, 0xFF0D2533]])
	art.tab = box(24, 24, [[0, 0, 24, 24, 0xFF39E1FF]])
	art.down = arrow(true)
	art.right = arrow(false)
	art.close = cross()


func retro() -> Dictionary:
	var t := {"bg": 0xFF000000, "panel": 0xFF020A04, "border": 0xFF1F7A3A, "text": 0xFF3DFF7A, "text_dim": 0xFF1FA64C, "accent": 0xFF3DFF7A,
		"accent_text": 0xFF001A08, "hover": 0xFF0A2A14, "active": 0xFF124422, "input_bg": 0xFF031408, "shadow": 0x5533FF77,
		"caret": 0xB03DFF7A, "select": 0x663DFF7A, "knob": 0xFF3DFF7A}
	var n := {"shadow_blur": 18, "shadow_y": 0, "radius": 0, "check_radius": 0, "tip_radius": 0, "border_width": 1, "anim_ms": 0,
		"ease": GoomeUI.EASE_LINEAR, "font_size": 15, "heading_size": 17, "caret_w": 9, "focus_w": 1, "focus_gap": 2, "toggle_w": 34,
		"toggle_h": 18, "slider_track": 2, "knob_size": 6, "font": "monospace", "font_mono": "monospace", "font_heading": "monospace"}
	return {"base": "dark", "theme": mix(t, n),
		"icons": {GoomeUI.ICON_CHECK: art.term_check, GoomeUI.ICON_RADIO_DOT: art.term_dot, GoomeUI.ICON_SLIDER_KNOB: art.term_knob,
			GoomeUI.ICON_TOGGLE_KNOB: art.term_toggle, GoomeUI.ICON_ARROW_DOWN: art.down, GoomeUI.ICON_ARROW_RIGHT: art.right},
		"skins": {GoomeUI.SKIN_CHECKBOX: [art.term_box, 3], GoomeUI.SKIN_RADIO: [art.term_box, 3], GoomeUI.SKIN_TOGGLE: [art.term_box, 3]}}


func soft() -> Dictionary:
	var t := {"bg": 0xFFF2F4F8, "panel": 0xFFFFFFFF, "border": 0x14000000, "input_bg": 0xFFEEF1F6, "hover": 0xFFE3E8F0, "shadow": 0x22203060}
	var n := {"font_size": 17, "heading_size": 22, "pad": 14, "gap": 10, "item_h": 44, "tab_h": 42, "win_title_h": 48, "check_size": 22,
		"check_radius": 11, "toggle_w": 46, "toggle_h": 28, "slider_h": 28, "knob_size": 12, "slider_track": 8, "shadow_blur": 34,
		"shadow_y": 10, "focus_w": 3, "focus_gap": 4, "anim_ms": 260, "hover_ms": 200, "press_ms": 160, "popup_ms": 320, "ease": GoomeUI.EASE_BACK}
	return {"make": [argb(0xFF2F7BF6), false, 20.0], "theme": mix(t, n), "icons": {}, "skins": {}}


func hud() -> Dictionary:
	var t := {"bg": 0x00000000, "panel": 0x8A06121C, "border": 0x6639E1FF, "text": 0xFFE6FBFF, "text_dim": 0xFF7FC4D6, "accent": 0xFF39E1FF,
		"accent_text": 0xFF041820, "hover": 0xFF12384A, "active": 0xFF1A4C63, "input_bg": 0xB0081C28, "shadow": 0x5539E1FF, "knob": 0xFF39E1FF}
	var n := {"shadow_blur": 26, "shadow_y": 0, "radius": 0, "check_radius": 0, "tip_radius": 0, "font_size": 16, "heading_size": 22,
		"item_h": 38, "win_title_h": 42, "knob_size": 9, "slider_track": 4, "check_size": 18, "ease": GoomeUI.EASE_OUT, "anim_ms": 160,
		"font": "sans-serif:bold"}
	return {"base": "dark", "theme": mix(t, n),
		"icons": {GoomeUI.ICON_CHECK: art.hud_check, GoomeUI.ICON_RADIO_DOT: art.hud_dot, GoomeUI.ICON_SLIDER_KNOB: art.hud_knob,
			GoomeUI.ICON_TOGGLE_KNOB: art.hud_knob, GoomeUI.ICON_ARROW_DOWN: art.down, GoomeUI.ICON_ARROW_RIGHT: art.right, GoomeUI.ICON_CLOSE: art.close},
		"skins": {GoomeUI.SKIN_PANEL: [art.frame, 8], GoomeUI.SKIN_BUTTON: [art.plate, 8], GoomeUI.SKIN_INPUT: [art.plate, 8],
			GoomeUI.SKIN_TAB: [art.tab, 7], GoomeUI.SKIN_CHECKBOX: [art.hud_box, 3], GoomeUI.SKIN_RADIO: [art.hud_radio, 0]}}


func dense() -> Dictionary:
	var t := {"bg": 0xFF1B1C1F, "panel": 0xFF232428, "border": 0xFF34363C, "text": 0xFFD5D7DC, "text_dim": 0xFF8A8F98, "accent": 0xFF4C8DF6,
		"accent_text": 0xFFFFFFFF, "hover": 0xFF2E3036, "active": 0xFF383B42, "input_bg": 0xFF1A1B1E, "shadow": 0x66000000}
	var n := {"shadow_blur": 10, "shadow_y": 3, "radius": 3, "check_radius": 2, "tip_radius": 3, "font_size": 12, "heading_size": 13, "pad": 6,
		"gap": 6, "item_h": 24, "tab_h": 24, "win_title_h": 26, "check_size": 13, "toggle_w": 26, "toggle_h": 14, "slider_h": 14, "knob_size": 5,
		"slider_track": 3, "scroll_w": 3, "focus_w": 1, "focus_gap": 1, "anim_ms": 70, "tip_pad_x": 6, "tip_pad_y": 4}
	return {"base": "dark", "theme": mix(t, n), "icons": {}, "skins": {}}


func mix(colors: Dictionary, nums: Dictionary) -> Dictionary:
	var out := nums.duplicate()
	for k in colors:
		out[k] = argb(colors[k])
	return out


func _ready() -> void:
	make_art()
	looks = [retro(), soft(), hud(), dense()]
	for i in 5:
		cards.append({"tab": i % 3, "sound": true, "online": i != DENSE, "level": 1 + (i & 1), "volume": 35.0 + i * 12, "mode": i % 3, "name": NAMES[i]})


func widgets(ui: GoomeUI, c: Dictionary, big: bool, focus: bool) -> void:
	c.tab = ui.tabs("tabs", PackedStringArray(["Play", "Stats", "Gear"]), c.tab)
	ui.heading("Player one")
	if focus:
		ui.set_focus()
	c.name = ui.input("Name", c.name, 47)
	c.volume = ui.slider("Volume", c.volume, 0, 100, "%.0f")
	ui.row(2)
	c.sound = ui.checkbox("Sound", c.sound)
	c.online = ui.toggle("Online", c.online)
	c.level = ui.radio("Easy", c.level, 0)
	ui.same_line()
	c.level = ui.radio("Normal", c.level, 1)
	ui.same_line()
	c.level = ui.radio("Hard", c.level, 2)
	if not big:
		c.mode = ui.combo("Mode", c.mode, PackedStringArray(["Story", "Arena", "Sandbox"]))
		ui.progress(c.volume / 100.0, "loading")
	ui.row(2)
	ui.button("Start")
	ui.button("Quit")


func scene(ui: GoomeUI, r: Rect2) -> void:
	ui.pos(r.position.x, r.position.y)
	var at: Rect2 = ui.custom("scene", r.size.x, r.size.y, GoomeUI.ITEM_NO_HIT).rect
	var x := at.position.x
	var y := at.position.y
	var w := at.size.x
	var h := at.size.y
	ui.draw_gradient(at, argb(0xFF1B2747), argb(0xFFE0794F), 14.0)
	ui.draw_circle(Vector2(x + w * 0.72, y + h * 0.62), 46, argb(0xFFFFC27A))
	var hills := PackedVector2Array([Vector2(x, y + h), Vector2(x, y + h * 0.7), Vector2(x + w * 0.22, y + h * 0.56), Vector2(x + w * 0.45, y + h * 0.74),
		Vector2(x + w * 0.7, y + h * 0.6), Vector2(x + w, y + h * 0.72), Vector2(x + w, y + h)])
	ui.draw_polygon(hills, argb(0xFF1A1F38))
	var near := PackedVector2Array([Vector2(x, y + h), Vector2(x, y + h * 0.86), Vector2(x + w * 0.3, y + h * 0.8), Vector2(x + w * 0.62, y + h * 0.9),
		Vector2(x + w, y + h * 0.82), Vector2(x + w, y + h)])
	ui.draw_polygon(near, argb(0xFF0D1022))


func look(ui: GoomeUI, k: int, title: String, r: Rect2) -> void:
	var l: Dictionary = looks[k]
	if l.has("make"):
		ui.push_theme_make(l.make[0], l.make[1], l.make[2], l.theme)
	else:
		ui.push_theme(l.base, l.theme)
	var n := 1
	for i in l.icons:
		ui.push_icon(i, l.icons[i])
		n += 1
	for s in l.skins:
		var e: float = l.skins[s][1]
		ui.push_skin(s, l.skins[s][0], e, e, e, e)
		n += 1
	ui.scale(zoom)
	ui.push_id(title)
	if ui.window(title, r, 0):
		widgets(ui, cards[k], k == SOFT or k == HUD, k == RETRO and focus_once)
		if k == RETRO:
			focus_once = false
	ui.window_end()
	ui.pop_id()
	ui.pop(n)


func simple(ui: GoomeUI, r: Rect2) -> void:
	if start == 0:
		ui.push_theme_make(Color("8b5cf6"), true, 12)
	else:
		ui.push_preset(start - 1)
	ui.scale(zoom)
	if ui.window("The simple path", r, 0):
		ui.wrapped("one accent color, dark or light and a corner radius:")
		ui.push_font("monospace")
		ui.push_num(GoomeUI.S_FONT_SIZE, 12)
		for line in CODE:
			ui.label(line)
		ui.pop(2)
		ui.space(4)
		start = ui.combo("Start from", start, PackedStringArray(STARTS))
		zoom = ui.slider("UI size", zoom, 0.8, 1.3, "%.2fx")
		ui.separator()
		var c: Dictionary = cards[4]
		ui.row(2)
		c.name = ui.input("Name", c.name, 47)
		pass_text = ui.password("Password", pass_text, 31, "8 letters or more")
		if ui.entered():
			ui.toast("logged in", 1500)
		ui.row(2)
		code = ui.input_ex("Room code", code, 7, GoomeUI.INPUT_NUMBER, "numbers only")
		server = ui.input_ex("Server", server, 15, GoomeUI.INPUT_READONLY)
		ui.row(2)
		c.sound = ui.checkbox("Sound", c.sound)
		c.online = ui.toggle("Online", c.online)
		ui.separator()
		ui.label("one push changes one widget")
		ui.row(3)
		ui.push_num(GoomeUI.S_RADIUS, 0)
		ui.button("Square")
		ui.pop()
		ui.push_style({"radius": 99, "align": GoomeUI.ALIGN_LEFT})
		ui.button("Pill")
		ui.pop(2)
		ui.push_color(GoomeUI.S_ACCENT, argb(0xFFF59E0B))
		ui.push_num(GoomeUI.S_CHECK_SIZE, 24)
		ui.push_num(GoomeUI.S_CHECK_RADIUS, 12)
		c.sound = ui.checkbox("Big", c.sound)
		ui.pop(3)
		ui.separator()
		ui.label("every preset, same widgets")
		for i in GoomeUI.PRESET_ROSE + 1:
			ui.push_preset(i)
			ui.push_id(STARTS[i + 1])
			ui.row(3)
			ui.badge(STARTS[i + 1], Color(0, 0, 0, 0))
			ui.button("Open")
			ui.toggle("##t", (i & 1) == 1)
			ui.pop_id()
			ui.pop()
	ui.window_end()
	ui.pop()


func _goome_frame(ui: GoomeUI) -> void:
	if not configured:
		ui.config_set("tip_delay_ms", 300)
		ui.config_set("cursor_button", GoomeUI.CURSOR_HAND)
		ui.bind(GoomeUI.ACT_SELECT_ALL, "ctrl+a super+a ctrl+l")
		configured = true
	scene(ui, Rect2(452, 496, 420, 428))
	look(ui, RETRO, "> retro terminal_", Rect2(16, 16, 420, 0))
	look(ui, SOFT, "Soft and round", Rect2(452, 16, 420, 0))
	look(ui, HUD, "GAME HUD", Rect2(476, 520, 372, 0))
	look(ui, DENSE, "dense pro tool", Rect2(16, 456, 420, 0))
	simple(ui, Rect2(888, 16, 416, 908))
