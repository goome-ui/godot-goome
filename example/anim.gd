extends GoomeView

const ROWS := [["linear", GoomeUI.EASE_LINEAR, Color("7c8cff")], ["in", GoomeUI.EASE_IN, Color("4fb3f7")], ["out", GoomeUI.EASE_OUT, Color("3fc8b4")], ["in out", GoomeUI.EASE_IN_OUT, Color("6fdc5f")], ["smooth", GoomeUI.EASE_SMOOTH, Color("e8d85a")], ["back", GoomeUI.EASE_BACK, Color("ffb443")], ["spring", GoomeUI.EASE_SPRING, Color("ff7a59")], ["bounce", GoomeUI.EASE_BOUNCE, Color("ff6fa8")]]
const MOODS := [Color("2d6cdf"), Color("e0483e"), Color("21a66b"), Color("f2b33d"), Color(0, 0, 0, 0)]
const MOOD_NAMES := ["blue", "red", "green", "gold", "see through"]

var side := 0
var autoplay := true
var mood := 0
var on := false
var count := 0
var kick := false
var flipped := 0.0


func fade(c: Color, k: float) -> Color:
	return Color(c, c.a * clampf(k, 0, 1))


func text_at(ui: GoomeUI, size: float, x: float, cy: float, s: String, c: Color, center: bool) -> void:
	var box := ui.text_size(s, size)
	ui.draw_text(Vector2(x - box.x / 2 if center else x, roundf(cy - box.y / 2)), s, c, size)


func ease_rows(ui: GoomeUI) -> void:
	for row in ROWS:
		var it := ui.custom(row[0], 0, 30, GoomeUI.ITEM_NO_HIT)
		var k := ui.anim(row[0], side, 900, row[1])
		var tx: float = it.x + 86
		var tw: float = it.w - 96
		var bw := 46.0
		var run := (tw - bw) * 0.76
		var x0 := tx + (tw - bw - run) / 2
		text_at(ui, 13, it.x + 4, it.y + 15, row[0], ui.theme_get("text_dim"), false)
		ui.draw_rect(Rect2(tx, it.y + 3, tw, 24), Color(1, 1, 1, 0.08), 12.0)
		ui.draw_rect(Rect2(x0 + bw / 2, it.y + 14, 1, 2), Color(1, 1, 1, 0.19))
		ui.draw_rect(Rect2(x0 + run + bw / 2, it.y + 14, 1, 2), Color(1, 1, 1, 0.19))
		ui.draw_shadow(Rect2(x0 + k * run, it.y + 8, bw, 16), fade(row[2], 0.45), 8, 8)
		ui.draw_rect(Rect2(x0 + k * run, it.y + 6, bw, 18), row[2], 9.0)


func color_card(ui: GoomeUI) -> void:
	var it := ui.custom("mood", 0, 92)
	if it.clicked:
		mood = (mood + 1) % MOODS.size()
	ui.tooltip("click for the next color")
	var col := ui.anim_color("mood", MOODS[mood], 600, GoomeUI.EASE_IN_OUT)
	var lift := ui.anim("mood lift", 1.0 if it.hovered else 0.0, 160, GoomeUI.EASE_OUT)
	var y: float = it.y - lift * 2
	for i in 12:
		ui.draw_rect(Rect2(it.x + i * it.w / 12, it.y + 8, it.w / 24, 76), Color(1, 1, 1, 0.06))
	ui.draw_shadow(Rect2(it.x, y + 6, it.w, 84), fade(col, 0.4 + lift * 0.2), 14, 18)
	ui.draw_rect(Rect2(it.x, y, it.w, 84), col, 14.0)
	ui.draw_ring(Vector2(it.x + 30, y + 42), 12, Color(1, 1, 1, 0.31), 2)
	text_at(ui, 15, it.x + 54, y + 42, "%s  #%s" % [MOOD_NAMES[mood], col.to_html().to_upper()], ui.theme_get("text"), false)


func spring_toggle(ui: GoomeUI) -> void:
	var it := ui.custom("springy", 64, 36, GoomeUI.ITEM_FOCUSABLE)
	var key: int = it.key & 0xFF
	if it.clicked or key == GoomeUI.K_SPACE or key == GoomeUI.K_ENTER:
		on = not on
	var k := ui.anim("springy", 1.0 if on else 0.0, 520, GoomeUI.EASE_SPRING)
	var squish := ui.anim("squish", 1.0 if it.held else 0.0, 220, GoomeUI.EASE_SPRING)
	var track := ui.anim_color("springy", ui.theme_get("accent") if on else Color("3a3f49"), 260, GoomeUI.EASE_OUT)
	var x: float = it.x + 2
	var y: float = it.y + 4
	var kw := 22 + squish * 8
	if it.focused:
		ui.draw_rect(Rect2(x - 3, y - 3, 66, 34), fade(ui.theme_get("focus"), 0.6), 17.0)
	ui.draw_rect(Rect2(x, y, 60, 28), track, 14.0)
	var kx := x + 3 + k * (54 - kw)
	ui.draw_shadow(Rect2(kx, y + 5, kw, 22), Color(0, 0, 0, 0.38), 11, 6)
	ui.draw_rect(Rect2(kx, y + 3, kw, 22), Color("f4f5f7"), 11.0)


func bell(ui: GoomeUI) -> void:
	var it := ui.custom("bell", 140, 64)
	if it.clicked:
		count += 1
		kick = true
	var drop := ui.anim("drop", -46.0 if kick else 0.0, 0 if kick else 900, GoomeUI.EASE_BOUNCE)
	var pop := ui.anim("pop", 0.4 if kick else 1.0, 0 if kick else 500, GoomeUI.EASE_SPRING)
	var shown := ui.anim("shown", 1.0 if count > 0 else 0.0, 200, GoomeUI.EASE_OUT)
	kick = false
	var bg := ui.anim_color("bell bg", Color("2e333c") if it.hovered else Color("262a31"), 140, GoomeUI.EASE_OUT)
	ui.draw_rect(Rect2(it.x, it.y + 8, 120, 52), bg, 12.0)
	text_at(ui, 14, it.x + 60, it.y + 34, "notify me", ui.theme_get("text"), true)
	if shown <= 0.01:
		return
	var r := 13 * pop
	var c := Vector2(it.x + 116, it.y + 10 + drop)
	ui.draw_shadow(Rect2(c.x - r, c.y - r + 3, 2 * r, 2 * r), fade(Color(0, 0, 0, 0.56), shown), r, 8)
	ui.draw_circle(c, r, fade(Color("e0483e"), shown))
	if pop > 0.6:
		text_at(ui, 12 * pop, c.x, c.y, str(count), fade(Color.WHITE, shown), true)


func _goome_frame(ui: GoomeUI) -> void:
	var now := Time.get_ticks_msec() / 1000.0
	if autoplay and now - flipped >= 1.5:
		flipped = now
		side = 1 - side
		if side:
			mood = (mood + 1) % MOODS.size()
			on = not on
			count += 1
			kick = true
	ui.app_begin("Animations", GoomeUI.APP_STATUSBAR)
	ui.heading("Every ease, same 900 ms")
	if ui.button("Swap sides"):
		side = 1 - side
		flipped = now
	ui.same_line()
	autoplay = ui.toggle("Play by itself", autoplay)
	ui.space(4)
	ease_rows(ui)
	ui.space(8)
	ui.heading("Color fade")
	color_card(ui)
	ui.space(8)
	ui.heading("Spring and bounce")
	spring_toggle(ui)
	ui.same_line()
	ui.label("on, and it wobbled a bit" if on else "off")
	ui.same_line()
	ui.space(40)
	ui.same_line()
	bell(ui)
	ui.statusbar("moving" if ui.animating() else "still")
	ui.app_end()
