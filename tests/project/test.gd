extends Node

const INTERNAL_DEVICE := -2

var view: GoomeView
var game_clicks := 0
var game_keys := 0
var game_moves := 0
var signal_frames := 0
var failed := false


func _ready() -> void:
	get_tree().create_timer(240.0).timeout.connect(func() -> void:
		print("goome test timed out")
		get_tree().quit(2))
	Input.use_accumulated_input = false
	get_window().size = Vector2i(640, 480)
	var game := Control.new()
	game.set_anchors_preset(Control.PRESET_FULL_RECT)
	game.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(game)
	view = preload("res://menu.gd").new()
	view.goome_frame.connect(func(ui: GoomeUI) -> void: signal_frames += 1)
	add_child(view)
	view.position = Vector2.ZERO
	view.size = Vector2(640, 480)
	run.call_deferred()


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed:
		game_clicks += 1
	if event is InputEventKey and event.pressed:
		game_keys += 1
	if event is InputEventMouseMotion and event.device != INTERNAL_DEVICE:
		game_moves += 1


func check(ok: bool, what: String) -> void:
	if ok:
		print("ok   ", what)
	else:
		print("FAIL ", what)
		failed = true


func frames(n: int) -> void:
	for i in n:
		await get_tree().process_frame


func move(p: Vector2) -> void:
	var e := InputEventMouseMotion.new()
	e.position = p
	e.global_position = p
	Input.parse_input_event(e)


func press(p: Vector2, down: bool) -> void:
	move(p)
	var e := InputEventMouseButton.new()
	e.button_index = MOUSE_BUTTON_LEFT
	e.pressed = down
	e.position = p
	e.global_position = p
	Input.parse_input_event(e)


func click(p: Vector2) -> void:
	move(p)
	for down in [true, false]:
		var e := InputEventMouseButton.new()
		e.button_index = MOUSE_BUTTON_LEFT
		e.pressed = down
		e.position = p
		e.global_position = p
		Input.parse_input_event(e)


func type_text(s: String) -> void:
	for c in s:
		for down in [true, false]:
			var e := InputEventKey.new()
			e.keycode = c.to_upper().unicode_at(0) as Key
			e.unicode = c.unicode_at(0)
			e.pressed = down
			Input.parse_input_event(e)


func key(k: Key) -> void:
	for down in [true, false]:
		var e := InputEventKey.new()
		e.keycode = k
		e.pressed = down
		Input.parse_input_event(e)


func find(flag: String, x: float) -> Vector2:
	for y in range(24, 260, 3):
		move(Vector2(x, y))
		await frames(2)
		if view.get(flag):
			return Vector2(x, y + 4)
	return Vector2(-1, -1)


func run() -> void:
	await frames(4)
	check(ClassDB.class_exists("GoomeView"), "extension loaded")
	var img: Image = view.get_image()
	check(img != null, "got a frame")
	if img == null:
		finish()
		return
	check(img.get_width() == 640 and img.get_height() == 480, "frame is 640x480, got %dx%d" % [img.get_width(), img.get_height()])
	check(img.get_pixel(40, 40).a > 0.5, "window drawn at 40,40")
	check(img.get_pixel(600, 420).a == 0.0, "nothing drawn at 600,420")
	check(view.frames > 0, "_goome_frame ran %d times" % view.frames)
	check(signal_frames == view.frames, "goome_frame signal ran too (%d)" % signal_frames)
	check(view.hit(Vector2(40, 40)) and not view.hit(Vector2(600, 420)), "hit test")

	var before := img.get_pixel(40, 40)
	var at := await find("over_button", 50)
	check(at.x > 0, "found the button at %s" % at)
	click(at)
	await frames(3)
	check(view.clicks == 1, "button click reached gdscript (%d)" % view.clicks)
	check(game_clicks == 0, "game didnt get the ui click (%d)" % game_clicks)

	click(Vector2(600, 420))
	await frames(2)
	check(game_clicks == 1, "click next to the ui fell through to the game (%d)" % game_clicks)
	check(view.clicks == 1, "that click didnt press the button")

	var knob := await find("over_slider", 150)
	check(knob.x > 0, "found the slider at %s" % knob)
	var vol_before: float = view.volume
	press(knob, true)
	await frames(2)
	game_moves = 0
	for i in 6:
		move(knob + Vector2(10 * (i + 1), 40 + i * 30))
		await frames(1)
	press(knob + Vector2(60, 220), false)
	await frames(2)
	check(view.volume > vol_before, "dragging the slider moved it (%.2f -> %.2f)" % [vol_before, view.volume])
	check(game_moves == 0, "game got no mouse moves during the drag, even off the ui (%d)" % game_moves)
	move(Vector2(600, 300))
	await frames(2)
	check(game_moves > 0, "moves next to the ui reach the game again (%d)" % game_moves)

	var spot := await find("over_input", 150)
	check(spot.x > 0, "found the input at %s" % spot)
	click(spot)
	await frames(3)
	check(view.wants_keyboard(), "input took the keyboard")
	key(KEY_END)
	type_text("hi")
	await frames(3)
	check(view.name_text == "goomehi", "typed into the input (%s)" % view.name_text)
	check(game_keys == 0, "game didnt get the typing (%d)" % game_keys)

	click(Vector2(600, 420))
	await frames(3)
	check(not view.wants_keyboard(), "clicking away gave the keyboard back")
	type_text("x")
	await frames(2)
	check(game_keys == 1, "keys reach the game again (%d)" % game_keys)
	check(view.name_text == "goomehi", "and dont go into the input")

	img = view.get_image()
	check(img.get_pixel(40, 40).a > 0.5 and before.a > 0.5, "frame still there after partial updates")
	await custom_widgets()
	await icon_hashes()
	await anything()
	consts05()
	await anims()
	await emoji_rtl()
	await ime()
	await a11y()
	finish()


func red(c: Color) -> bool:
	return c.r > 0.9 and c.g < 0.1 and c.b < 0.1 and c.a > 0.9


func shot(at: Vector2) -> Image:
	move(at)
	await frames(4)
	return view.get_image().duplicate()


func custom_widgets() -> void:
	view.queue_free()
	view = preload("res://custom.gd").new()
	add_child(view)
	view.position = Vector2.ZERO
	view.size = Vector2(640, 480)
	await frames(4)
	var o: Vector2 = view.origin
	var away := o + Vector2(600, 460)

	var plain := await shot(away)
	view.styled = 1
	var styled := await shot(away)
	view.styled = 2
	var dict := await shot(away)
	view.styled = 3
	var themed := await shot(away)
	view.styled = 4
	var skinned := await shot(away)
	view.styled = 0
	view.leave_pushed = true
	await frames(2)
	var after := await shot(away)
	view.leave_pushed = false
	var corner := Vector2i(o + Vector2(21, 21))
	var one := Vector2i(o + Vector2(28, 40))
	var two := Vector2i(o + Vector2(28, 100))
	check(red(styled.get_pixel(one.x, one.y)), "push_color made the button red")
	check(red(styled.get_pixel(corner.x, corner.y)) and not red(plain.get_pixel(corner.x, corner.y)), "push_num radius 0 made the corner square")
	check(not red(plain.get_pixel(one.x, one.y)), "no push, no red")
	check(styled.get_pixel(two.x, two.y) == plain.get_pixel(two.x, two.y), "pop put the style back for the next button")
	check(view.pushed == 2 and red(dict.get_pixel(corner.x, corner.y)) and dict.get_pixel(two.x, two.y) == plain.get_pixel(two.x, two.y), "push_style did the same with a dictionary (%d pushed)" % view.pushed)
	check(red(themed.get_pixel(corner.x, corner.y)) and themed.get_pixel(two.x, two.y) == plain.get_pixel(two.x, two.y), "push_theme swapped the whole theme and popped as one")
	check(red(skinned.get_pixel(one.x, one.y)) and skinned.get_pixel(two.x, two.y) == plain.get_pixel(two.x, two.y), "push_skin drew the button from the Image")
	check(after.get_pixel(one.x, one.y) == plain.get_pixel(one.x, one.y), "a push left open at the end didnt leak into the next frame")

	var s: Dictionary = view.spot
	check(s.x == o.x + 200 and s.y == o.y + 30 and s.w == 50 and s.h == 30, "pos and the size put it exactly there (%s)" % s.rect)
	var g := plain
	var x0 := int(s.x)
	var y0 := int(s.y)
	check(g.get_pixel(x0, y0).g > 0.9 and g.get_pixel(x0 + 49, y0 + 29).g > 0.9, "draw_rect fills the whole spot")
	check(g.get_pixel(x0 - 1, y0 + 10).g < 0.5 and g.get_pixel(x0 + 50, y0 + 10).g < 0.5 and g.get_pixel(x0 + 10, y0 + 30).g < 0.5, "and nothing outside it")
	check(view.text_width > 0 and view.text_box.x > 0 and view.text_box.y > 0, "draw_text and text_size measured something")

	var free: Dictionary = view.free_box
	var grab := Vector2(free.x + 30, free.y + 30)
	press(grab, true)
	await frames(2)
	game_moves = 0
	for i in 5:
		move(grab + Vector2(6, 4) * (i + 1))
		await frames(1)
	await frames(1)
	check(view.free_moved.distance_to(Vector2(30, 20)) < 0.01, "free drag moved by 30,20 (%s)" % view.free_moved)
	check(view.free_drag.distance_to(Vector2(30, 20)) < 0.01, "drag_x drag_y add up (%s)" % view.free_drag)
	move(grab + Vector2(250, 300))
	await frames(2)
	check(view.free_box.held, "still held when the mouse leaves the box")
	check(game_moves == 0, "the game got no moves during the drag (%d)" % game_moves)
	press(grab + Vector2(250, 300), false)
	await frames(2)
	check(not view.free_box.held and view.free_released == 1, "let go once")
	check(view.presses == 1, "store kept the press count (%d)" % view.presses)
	await frames(3)
	check(view.presses == 1, "and still has it a few frames later")
	check(view.fresh.size() == 3 and view.fresh[0] == 0 and view.fresh[2] == 0, "a new store comes back zeroed")

	var xb: Dictionary = view.x_box
	var xg := Vector2(xb.x + 30, xb.y + 20)
	press(xg, true)
	await frames(2)
	for i in 5:
		move(xg + Vector2(6, 9) * (i + 1))
		await frames(1)
	press(xg + Vector2(30, 45), false)
	await frames(2)
	check(view.x_moved.distance_to(Vector2(30, 0)) < 0.01, "DRAG_X alone only moves sideways (%s)" % view.x_moved)
	check(view.x_drag.y == 0 and absf(view.x_drag.x - 30) < 0.01, "and drag_y stays 0 (%s)" % view.x_drag)

	var kb: Dictionary = view.knob
	click(Vector2(kb.x + 30, kb.y + 30))
	await frames(3)
	check(view.knob.focused, "clicking a FOCUSABLE item focuses it")
	check(view.wants_keyboard(), "and the view takes the keyboard for it")
	var clicks: int = view.knob_clicks
	key(KEY_RIGHT)
	await frames(2)
	check(view.keys.has(GoomeUI.K_RIGHT), "right arrow reached the item (%s)" % [view.keys])
	key(KEY_ENTER)
	await frames(2)
	check(view.knob_clicks == clicks + 1, "enter clicks it")
	key(KEY_ESCAPE)
	await frames(2)
	check(not view.knob.focused, "esc lets go of the focus")


func fnv(img: Image) -> String:
	var h := 2166136261
	for b in img.get_data():
		h = ((h ^ b) * 16777619) & 0xFFFFFFFF
	return "%08x" % h


func icon_hashes() -> void:
	check(GoomeUI.ICON_ARROW_DOWN == 0 and GoomeUI.ICON_CLOSE == 6 and GoomeUI.ICON_TOAST == 20 and GoomeUI.ICON_PICKER == 21, "icon constants match goome.h")
	check(GoomeUI.SKIN_INPUT == 2 and GoomeUI.SKIN_CHECKBOX == 3 and GoomeUI.SKIN_SLIDER_TRACK == 6 and GoomeUI.SKIN_POPUP == 13, "skin constants match goome.h")
	view.queue_free()
	view = preload("res://icons.gd").new()
	add_child(view)
	view.position = Vector2.ZERO
	view.size = Vector2(300, 260)
	view.pixel_scale = 1
	move(Vector2(620, 470))
	var phases := [["base", 0, false, 0], ["dressed", 0, true, 0], ["as_is", 0, true, 1], ["pushed", 3, false, 0], ["theme_swap", 4, false, 0], ["after", 0, false, 0]]
	var got := {}
	for p in phases:
		view.mode = p[1]
		view.dressed = p[2]
		view.tint = p[3]
		await frames(8)
		if p[3] == 1:
			check(view.tint_seen == 1, "theme_get reads icon_tint back (%s)" % view.tint_seen)
		got[p[0]] = fnv(view.get_image())
	print("icon hashes ", got)
	check(got.base != got.dressed and got.dressed != got.as_is and got.base != got.pushed, "icons and skins change the pixels")
	check(got.pushed == got.theme_swap, "pop put the checkbox icon and skin back")
	check(got.base == got.after, "icons didnt leak into later frames")
	var want := {"base": "7ede4686", "dressed": "ce8bd05c", "as_is": "e33f332e", "pushed": "8b5b60db", "theme_swap": "8b5b60db", "after": "7ede4686"}
	if FileAccess.file_exists("res://icons_c.txt"):
		want = {}
		for line in FileAccess.get_file_as_string("res://icons_c.txt").split("\n", false):
			var kv := line.strip_edges().split(" ")
			want[kv[0]] = kv[1]
	else:
		print("no icons_c.txt, checking against the hashes from goome.h 0c2f311")
	for k in want:
		check(got.get(k) == want[k], "%s hash matches c (%s vs %s)" % [k, got.get(k), want[k]])

	view.mode = 3
	view.heart_src = ImageTexture.create_from_image(view.heart)
	await frames(8)
	check(fnv(view.get_image()) == got.pushed, "push_icon takes a Texture2D too")
	view.heart_src = "res://heart.png"
	await frames(8)
	check(fnv(view.get_image()) == got.pushed, "push_icon takes a res:// path too")
	view.mode = 0
	view.heart_src = view.heart
	await frames(8)
	check(fnv(view.get_image()) == got.base, "and pops back")
	check(view.tint_seen == 0, "theme_get reads icon_tint back")


func argb_fnv(img: Image) -> String:
	var d := img.get_data()
	var h := 2166136261
	var n := d.size()
	var i := 0
	while i < n:
		h = ((h ^ d[i + 2]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i + 1]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i + 3]) * 16777619) & 0xFFFFFFFF
		i += 4
	return "%08x" % h


func fresh_any(scene: String) -> GoomeView:
	if view:
		view.queue_free()
	view = preload("res://anything.gd").new()
	view.scene = scene
	add_child(view)
	view.position = Vector2.ZERO
	view.size = Vector2(420, 700)
	move(Vector2(-1000, -1000))
	await frames(3)
	return view


func combo(k: Key, ctrl := false, alt := false) -> void:
	for down in [true, false]:
		var e := InputEventKey.new()
		e.keycode = k
		e.ctrl_pressed = ctrl
		e.alt_pressed = alt
		e.pressed = down
		Input.parse_input_event(e)
	await frames(2)


func typed(s: String) -> void:
	for c in s:
		if c == "\n":
			await combo(KEY_ENTER)
		else:
			type_text(c)
			await frames(2)


func input_run(flags: int, text: String, hint: String, focus: bool, keys: Array) -> Array:
	var v := await fresh_any("input")
	v.flags = flags
	v.hint = hint
	v.buf = text
	v.set_focus_once = focus
	await frames(3)
	if focus:
		v.grab_focus()
	for k in keys:
		if k is String:
			await typed(k)
		else:
			await combo(k[0], k[1], k.size() > 2 and k[2])
	await frames(6)
	return [argb_fnv(v.get_image()), v.buf, v.got_enter]


func anything() -> void:
	check(GoomeUI.S_FOCUS == 18 and GoomeUI.S_DIM == 27 and GoomeUI.S_CHECK_SIZE == 28 and GoomeUI.S_TOAST_FADE_MS == 71, "new style constants match goome.h")
	check(GoomeUI.PRESET_ROSE == 6 and GoomeUI.INPUT_ENTER_RETURNS == 16 and GoomeUI.ACT_DOC_END == 11 and GoomeUI.CUR_CUSTOM_XY == 11 and GoomeUI.EASE_BACK == 4 and GoomeUI.ALIGN_RIGHT == 3, "preset, input, action, cursor, ease and align constants match goome.h")
	var v := await fresh_any("kit")
	var ui: GoomeUI = v.get_ui()
	await frames(8)
	var kit := argb_fnv(v.get_image())
	check(kit == "d064e9fc", "kit hash matches c (%s)" % kit)
	var want_p := ["d064e9fc", "20984a84", "bedf5364", "dc6d4703", "f4f61716", "fd60aa64", "86f84612"]
	var want_m := ["35faaf7a", "5edb0223", "8fdeb769", "0a59d080"]
	var all := 2166136261
	var got := []
	for i in 7:
		v.preset = i
		await frames(8)
		var h := argb_fnv(v.get_image())
		got.append(h)
		all = ((all ^ h.hex_to_int()) * 16777619) & 0xFFFFFFFF
	v.preset = -1
	for i in 4:
		v.make = i
		await frames(8)
		var h := argb_fnv(v.get_image())
		got.append(h)
		all = ((all ^ h.hex_to_int()) * 16777619) & 0xFFFFFFFF
	v.make = -1
	print("preset hashes ", got)
	check(got == want_p + want_m, "every preset and theme_make hash matches c")
	check("%08x" % all == "315ea656", "preset hashes add up to the c one (%08x)" % all)

	ui.scale(2)
	check(ui.theme_get("font_size") == 28 and ui.theme_get("pad") == 20 and ui.theme_get("scroll_w") == 8 and ui.theme_get("tip_x") == 28 and ui.theme_get("check_size") == 18 and ui.theme_get("hover_ms") < 0, "scale(2) doubled the sizes and left the em ones")
	await frames(8)
	var big := argb_fnv(v.get_image())
	check(big == "a3ad7bbf", "scale 2 hash matches c after it settles (%s)" % big)
	ui.scale(0.5)
	await frames(8)
	check(argb_fnv(v.get_image()) == kit and ui.theme_get("font_size") == 14, "scale 0.5 put it back")

	check(ui.config_get("tip_delay_ms") == 500 and ui.config_get("dbl_click_ms") == 450 and ui.config_get("blink_ms") == 0 and ui.config_get("tab_nav") and ui.config_get("esc_closes") and not ui.config_get("focus_on_mouse"), "config defaults like c")
	check(ui.config_get("cursor_input") == GoomeUI.CURSOR_IBEAM and ui.config_get("mask") == "*", "cursor and mask defaults")
	ui.config_set("cursor_button", GoomeUI.CURSOR_HAND)
	check(ui.config_get("cursor_button") == GoomeUI.CURSOR_HAND, "config_set cursor_button")

	v = await fresh_any("input")
	ui = v.get_ui()
	v.set_focus_once = true
	await frames(4)
	v.grab_focus()
	check(ui.bind(GoomeUI.ACT_COPY, "ctrl+k") and ui.bind(GoomeUI.ACT_SELECT_ALL, "alt+l, ctrl+q"), "bind took the new keys")
	check(not ui.bind(GoomeUI.ACT_DOC_END + 1, "ctrl+k") and not ui.bind(GoomeUI.ACT_UNDO, "ctrl+nope"), "bind says no to a bad action or key")
	await combo(KEY_C, true)
	await combo(KEY_END)
	await combo(KEY_V, true)
	check(v.buf == "hello you", "ctrl+c doesnt copy anymore (%s)" % v.buf)
	await combo(KEY_Q, true)
	await combo(KEY_K, true)
	await combo(KEY_END)
	await combo(KEY_V, true)
	check(v.buf == "hello youhello you", "ctrl+q selects all and ctrl+k copies (%s)" % v.buf)
	check(ui.bind(GoomeUI.ACT_PASTE, ""), "unbind paste")
	await combo(KEY_V, true)
	check(v.buf == "hello youhello you", "ctrl+v does nothing now (%s)" % v.buf)
	ui.bind(GoomeUI.ACT_WORD_LEFT, "alt+left")
	v.buf = "ab cd"
	await frames(2)
	await combo(KEY_END)
	await combo(KEY_LEFT, false, true)
	await typed("_")
	check(v.buf == "ab _cd", "alt+left jumps a word (%s)" % v.buf)
	await combo(KEY_LEFT, true)
	await typed("-")
	check(v.buf == "ab -_cd", "ctrl+left is just left now (%s)" % v.buf)

	var plain = await input_run(0, "abc", "", false, [])
	var pw = await input_run(GoomeUI.INPUT_PASSWORD, "abc", "", false, [])
	var pw2 = await input_run(GoomeUI.INPUT_PASSWORD, "xyz", "", false, [])
	check(plain[0] == "f92831cd", "plain input hash matches c (%s)" % plain[0])
	check(pw[0] == "839d2d42" and pw2[0] == pw[0], "password hash matches c and hides the letters (%s)" % pw[0])
	var hinted = await input_run(0, "", "type here", false, [])
	check(hinted[0] == "8d8a6252", "hint hash matches c (%s)" % hinted[0])
	var r = await input_run(GoomeUI.INPUT_PASSWORD, "secret", "", true, [[KEY_C, true], [KEY_END, false], [KEY_V, true]])
	check(r[1] == "secret", "password wont copy (%s)" % r[1])
	r = await input_run(0, "secret", "", true, [[KEY_C, true], [KEY_END, false], [KEY_V, true]])
	check(r[1] == "secretsecret", "plain input does copy (%s)" % r[1])
	r = await input_run(GoomeUI.INPUT_PASSWORD, "secret", "", true, [[KEY_X, true], [KEY_V, true], "q"])
	check(r[1] == "q", "password cut clears but copies nothing (%s)" % r[1])
	r = await input_run(GoomeUI.INPUT_READONLY, "keep", "", true, ["zz"])
	check(r[1] == "keep", "readonly keeps it (%s)" % r[1])
	r = await input_run(GoomeUI.INPUT_NUMBER, "", "", true, ["a1-.x2+"])
	check(r[1] == "1-.2+", "number only takes numbers (%s)" % r[1])
	r = await input_run(GoomeUI.INPUT_NO_SELECT_ON_FOCUS, "ab", "", true, ["z"])
	check(r[1] == "abz", "no select on focus (%s)" % r[1])
	r = await input_run(GoomeUI.INPUT_ENTER_RETURNS, "ab", "", true, ["\n"])
	check(r[2], "enter returns sets entered()")
	r = await input_run(0, "ab", "", true, ["\n"])
	check(not r[2], "no flag, no entered()")

	v = await fresh_any("input")
	v.get_ui().config_set("mask", "#")
	check(v.get_ui().config_get("mask") == "#", "config mask reads back")
	v.flags = GoomeUI.INPUT_PASSWORD
	v.buf = "abc"
	await frames(6)
	check(argb_fnv(v.get_image()) != pw[0], "a # mask draws different")
	v.queue_free()
	view = null


func fresh(script: Script, at: Vector2) -> GoomeView:
	if view:
		view.queue_free()
	view = script.new()
	add_child(view)
	view.position = Vector2.ZERO
	view.size = at
	move(Vector2(-1000, -1000))
	return view


func until(what: Callable, most: int) -> void:
	for i in most:
		if what.call():
			return
		await frames(1)


func consts05() -> void:
	check(GoomeUI.EASE_SMOOTH == 0 and GoomeUI.EASE_CUSTOM == 5 and GoomeUI.EASE_IN == 6 and GoomeUI.EASE_SPRING == 7 and GoomeUI.EASE_BOUNCE == 8, "ease constants match goome.h")
	check(GoomeUI.DIR_AUTO == 0 and GoomeUI.DIR_LTR == 1 and GoomeUI.DIR_RTL == 2, "text dir constants match goome.h")
	check(GoomeUI.ROLE_NONE == 0 and GoomeUI.ROLE_WINDOW == 1 and GoomeUI.ROLE_INPUT == 7 and GoomeUI.ROLE_MENUITEM == 13 and GoomeUI.ROLE_PROGRESS == 16 and GoomeUI.ROLE_DIALOG == 19, "role constants match goome.h")
	check(GoomeUI.A11Y_FOCUSED == 1 and GoomeUI.A11Y_CHECKED == 2 and GoomeUI.A11Y_DISABLED == 4 and GoomeUI.A11Y_EXPANDED == 8 and GoomeUI.A11Y_SELECTED == 16, "a11y state constants match goome.h")


func anims() -> void:
	var ui := GoomeUI.new()
	check(ui.ease(GoomeUI.EASE_LINEAR, 0.25) == 0.25 and ui.ease(GoomeUI.EASE_IN, 0.5) == 0.125 and ui.ease(GoomeUI.EASE_OUT, 0.5) == 0.875, "ease linear in out")
	check(ui.ease(GoomeUI.EASE_IN_OUT, 0.25) == 0.0625 and ui.ease(GoomeUI.EASE_IN_OUT, 0.75) == 0.9375 and ui.ease(GoomeUI.EASE_SMOOTH, 0.5) == 0.5, "ease in_out smooth")
	check(ui.ease(GoomeUI.EASE_BOUNCE, 0.25) == 0.47265625 and ui.ease(GoomeUI.EASE_OUT, -3) == 0 and ui.ease(GoomeUI.EASE_IN, 7) == 1, "ease bounce and clamping")
	var peak := 0.0
	for i in range(1, 100):
		peak = maxf(peak, ui.ease(GoomeUI.EASE_SPRING, i / 100.0))
	check(peak > 1.1 and peak < 1.25, "spring overshoots a bit (%.3f)" % peak)
	ui.free()
	var v := fresh(preload("res://anim.gd"), Vector2(300, 200))
	await until(func() -> bool: return v.final_last >= 0, 4000)
	var lg: Array = v.log
	check(lg.size() >= 66, "anim ran (%d frames logged)" % lg.size())
	if lg.size() < 66:
		return
	check(lg[0].box == 10 and lg[0].sp == 0 and lg[0].col.to_argb32() == 0 and lg[0].last == 0, "anims start at the target")
	check(not lg[1].animating, "nothing moves before the switch")
	var over := 0.0
	var moving := 0
	var up := true
	for i in 60:
		var f: Dictionary = lg[i + 2]
		over = maxf(over, f.sp)
		up = up and f.box >= lg[i + 1].box
		if f.animating:
			moving = i + 1
	check(up, "box only goes up")
	check(lg[61].box == 200 and lg[61].sp == 1 and lg[61].col.to_argb32() == 0xFF20D060 and lg[61].last == 5, "anims land on the target")
	check(over > 1.05 and moving >= 19 and moving <= 22 and not lg[61].animating, "spring overshoots and it all stops (%.3f, %d)" % [over, moving])
	var m: int = lg[8].col.to_argb32()
	check((m >> 24) > 0x70 and (m >> 24) < 0x90 and ((m >> 8) & 255) == 0xD0 and ((m >> 16) & 255) == 0x20, "anim_color fades the alpha (%08x)" % m)
	check(lg[62].animating and lg[62].box == 200 and lg[63].box < 200 and lg[63].box > 10, "going back starts moving")
	check(lg[64].box < 200 and lg[64].animating, "flipping again mid way")
	check(v.final_last == 0, "a dropped anim starts fresh")
	var got := ["%08x" % lg[3].hash, "%08x" % lg[6].hash, "%08x" % lg[10].hash, "%08x" % lg[61].hash]
	print("anim hashes ", got)
	check(got == ["0226f2f5", "906d3ce5", "db5848ad", "12bea72d"], "anim frame hashes match c")


func near(a: float, b: float) -> bool:
	return absf(a - b) < 0.01


func emoji_rtl() -> void:
	var v := fresh(preload("res://text.gd"), Vector2(400, 120))
	await until(func() -> bool: return v.hashes.size() >= v.jobs.size(), 200)
	var z: Dictionary = v.sizes
	check(near(z.tone, 24) and near(z.zwj, 24) and near(z.heart, 18) and near(z.heart_vs, 24), "emoji sequences measure like c (%s)" % z)
	check(near(z.goome, z.goome_plain + 16), "fallback font fills in the emoji")
	check(near(z.arab_short, 93.2), "arabic forms measure like c (%.2f)" % z.arab_short)
	var c: Color = v.mid
	check(c.a8 == 255 and c.r8 > 200 and c.g8 > 100 and c.g8 < 210 and c.b8 < 120, "cbdt cat is orange in the middle (%s)" % c)
	var h: Array = v.hashes
	print("emoji rtl hashes ", h)
	check(h[4] != h[3] and h[5] == h[3] and h[7] != h[6], "rtl and ltr and shaping change the pixels")
	var got := [h[0], h[1], h[2], h[3], h[4], h[6]]
	check(got == ["3bab3875", "85fe883a", "d3add387", "b79518dd", "49a72d7d", "469719e1"], "emoji and rtl hashes match c")


func ime_run(in_text: String, ed_text: String, focus: int, actions: Array) -> GoomeView:
	var v := fresh(preload("res://ime.gd"), Vector2(300, 200))
	v.in_text = in_text
	v.ed_text = ed_text
	v.focus = focus
	v.actions = actions
	var want := actions.size() + 3
	await until(func() -> bool: return v.log.size() > want, 200)
	return v


func ime() -> void:
	var v := await ime_run("ab", "", 1, [["pre", "xyz", 1], ["pre", "xyz", 3], ["pre", "", 0], ["text", "日本"], ["key", 99, 0], ["key", 122, GoomeUI.MOD_CTRL], ["key", 122, GoomeUI.MOD_CTRL], ["key", 97, GoomeUI.MOD_CTRL], ["text", "é"], ["key", 122, GoomeUI.MOD_CTRL]])
	var lg: Array = v.log
	var h := [lg[1].hash, lg[2].hash, lg[3].hash]
	check(h[0] != h[1] and h[1] != h[2] and lg[3].in == "ab", "preedit draws without touching the text")
	check(lg[4].hash == h[0], "empty preedit puts it back")
	check(lg[5].in == "ab日本" and lg[6].in == "ab日本c", "committed text goes in (%s)" % lg[6].in)
	check(lg[7].in == "ab日本" and lg[8].in == "ab日本c", "undo and redo (%s %s)" % [lg[7].in, lg[8].in])
	check(lg[10].in == "é" and lg[11].in == "ab日本c", "commit over a selection undoes in one go (%s %s)" % [lg[10].in, lg[11].in])
	check(v.preedits.size() >= 3 and v.preedits[0] == ["xyz", 1] and v.preedits[1] == ["xyz", 3] and v.texts.has("日本"), "ime_preedit and ime_text signals (%s)" % [v.preedits])
	v = await ime_run("", "line", 2, [["pre", "kana", 2], ["pre", "", 0], ["text", "かな\n"], ["key", 122, GoomeUI.MOD_CTRL]])
	lg = v.log
	var edh: String = lg[2].hash
	check(edh != lg[1].hash, "text_edit draws the preedit")
	check(lg[4].ed == "かな\nline" and lg[4].in == "", "text_edit takes the commit (%s)" % lg[4].ed)
	check(lg[5].ed == "line", "and undoes it")
	print("ime hashes ", h, " ", edh)
	check(h[1] == "2e20540f" and h[2] == "ceafcbad" and edh == "c9c1add5", "preedit hashes match c")
	v = await ime_run("", "", 1, [["flags", GoomeUI.INPUT_NUMBER], ["text", "1a2.5"]])
	check(v.log[3].in == "12.5", "number input filters committed text (%s)" % v.log[3].in)

	v = await ime_run("", "", 1, [])
	await frames(2)
	check(v.is_ime_active(), "focused input turns the ime on")
	var r: Rect2 = v.get_ime_caret()
	check(r.position.x > 0 and r.position.y > 0 and r.size.y >= 14, "and puts the caret there (%s)" % r)
	v.grab_focus()
	for ch in "жё":
		var e := InputEventKey.new()
		e.unicode = ch.unicode_at(0)
		e.pressed = true
		Input.parse_input_event(e)
	await frames(4)
	check(v.in_text == "жё" and v.texts == ["ж", "ё"], "godot ime keys come in as text (%s %s)" % [v.in_text, v.texts])
	click(Vector2(250, 190))
	await frames(4)
	check(not v.is_ime_active(), "clicking away turns the ime off")


func node(tree: Array, role: int, label: String) -> int:
	for i in tree.size():
		if tree[i].role == role and tree[i].label == label:
			return i
	return -1


func a11y() -> void:
	var v := fresh(preload("res://a11y.gd"), Vector2(420, 700))
	await frames(4)
	var ui: GoomeUI = v.get_ui()
	var t: Array = ui.a11y_tree()
	check(t.size() > 15, "a11y tree has the widgets (%d)" % t.size())
	if t.size() <= 15:
		return
	var app := node(t, GoomeUI.ROLE_WINDOW, "Demo")
	var ok := node(t, GoomeUI.ROLE_BUTTON, "OK")
	var dark := node(t, GoomeUI.ROLE_CHECKBOX, "Dark")
	var vol := node(t, GoomeUI.ROLE_SLIDER, "Volume")
	var nm := node(t, GoomeUI.ROLE_INPUT, "Name")
	var fruit := node(t, GoomeUI.ROLE_COMBO, "Fruit")
	var tree := node(t, GoomeUI.ROLE_TREE, "Folder")
	var knob := node(t, GoomeUI.ROLE_BUTTON, "Knob")
	var tab2 := node(t, GoomeUI.ROLE_TAB, "Two")
	var big := node(t, GoomeUI.ROLE_RADIO, "Big")
	check(app == 0 and t[app].parent == -1, "app window is the root")
	check(ok > 0 and t[ok].parent == app and t[ok].rect.size.x > 0 and t[ok].rect.size.y > 0, "button sits in the app with a rect")
	check(node(t, GoomeUI.ROLE_TEXT, "Hello there") > 0 and node(t, GoomeUI.ROLE_TEXT, "ready") > 0 and node(t, GoomeUI.ROLE_BUTTON, "Close") > 0, "labels, statusbar and titlebar buttons")
	check(dark > 0 and not (t[dark].states & GoomeUI.A11Y_CHECKED) and big > 0, "checkbox and radio")
	check(vol > 0 and t[vol].num == 2 and t[vol].min == 0 and t[vol].max == 10 and t[vol].value == "2.0", "slider value and range")
	check(nm > 0 and t[nm].value == "Bo" and fruit > 0 and t[fruit].value == "Mango", "input and combo values")
	check(tab2 > 0 and t[t[tab2].parent].role == GoomeUI.ROLE_GROUP and not (t[tab2].states & GoomeUI.A11Y_SELECTED), "tabs live in a group")
	check(tree > 0 and not (t[tree].states & GoomeUI.A11Y_EXPANDED) and node(t, GoomeUI.ROLE_TEXT, "inside") < 0, "closed tree hides its kids")
	check(knob > 0 and t[knob].value == "11", "a11y_role and a11y_value name a custom item")
	for verb in [["press", ok, ""], ["press", dark, ""], ["press", big, ""], ["set", vol, "7.5"], ["set", nm, "hello there"], ["press", knob, ""], ["expand", tree, ""], ["press", tab2, ""]]:
		v.a11y_action(verb[0], t[verb[1]].id, verb[2])
	await frames(4)
	check(v.ok == 1 and v.dark and v.mode == 1 and v.vol == 7.5 and v.name_text == "hello there" and v.custom_hits == 1 and v.tab == 1, "screen reader actions work the widgets")
	t = ui.a11y_tree()
	tree = node(t, GoomeUI.ROLE_TREE, "Folder")
	var inside := node(t, GoomeUI.ROLE_TEXT, "inside")
	check(tree > 0 and (t[tree].states & GoomeUI.A11Y_EXPANDED) and inside > 0 and t[inside].parent == tree, "expanded tree shows its kids")
	dark = node(t, GoomeUI.ROLE_CHECKBOX, "Dark")
	check(dark > 0 and (t[dark].states & GoomeUI.A11Y_CHECKED), "checkbox state follows")
	v.a11y_action("focus", t[node(t, GoomeUI.ROLE_INPUT, "Name")].id)
	await frames(4)
	t = ui.a11y_tree()
	nm = node(t, GoomeUI.ROLE_INPUT, "Name")
	check(nm > 0 and (t[nm].states & GoomeUI.A11Y_FOCUSED) and t[nm].caret == 11, "focus action and caret (%s)" % (t[nm] if nm >= 0 else {}))
	ui.a11y(false)
	await frames(3)
	check(ui.a11y_tree().is_empty(), "a11y off empties the tree")
	v.queue_free()
	view = null


func finish() -> void:
	print("goome test ", "FAILED" if failed else "passed")
	get_tree().quit(1 if failed else 0)
