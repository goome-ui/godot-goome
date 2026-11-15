extends GoomeView

const SWATCHES := [Color("6fdc5f"), Color("3fc8b4"), Color("4fb3f7"), Color("7c8cff"), Color("b57cff"), Color("ff6fa8"), Color("ff7a59"), Color("ffb443"), Color("e8d85a")]
const NODES := [
	{"name": "Noise", "info": "seed 42", "color": Color("b57cff"), "at": Vector2(20, 40)},
	{"name": "Blur", "info": "radius 6", "color": Color("4fb3f7"), "at": Vector2(240, 120)},
	{"name": "Output", "info": "1280x720", "color": Color("ff7a59"), "at": Vector2(460, 40)},
]
const HZ := ["60", "150", "400", "1k", "2.4k", "6k", "15k"]
const EQ_START := [0.7, 0.62, 0.45, 0.5, 0.58, 0.68, 0.74]
const KEYFRAMES := [1.2, 2.6, 5.0, 6.4, 7.7]

var seg := 1
var tab := 0
var sel := 0
var on := true
var pick := 1
var pink := true
var vol := 0.62
var amount := 0.7
var room := "living room"
var find := "search, but round"


func fade(c: Color, k: float) -> Color:
	return Color(c, c.a * clampf(k, 0, 1))


func snap(v: float, step: float) -> float:
	return floorf(v / step + 0.5) * step


func text_at(ui: GoomeUI, size: float, x: float, cy: float, s: String, c: Color, center: bool) -> void:
	var box := ui.text_size(s, size)
	ui.draw_text(Vector2(x - box.x / 2 if center else x, roundf(cy - box.y / 2)), s, c, size)


func knob(ui: GoomeUI, title: String, start: float, scale: float, fmt: String) -> void:
	var v := ui.store(title, 2)
	if not v[1]:
		v[0] = start
		v[1] = 1
	var it := ui.custom(title, 92, 112, GoomeUI.ITEM_DRAG_Y | GoomeUI.ITEM_FOCUSABLE)
	var k: int = it.key & 0xFF
	if it.held:
		v[0] = clampf(v[0] - it.dy / 180, 0, 1)
	if k == GoomeUI.K_UP or k == GoomeUI.K_RIGHT:
		v[0] = clampf(v[0] + 0.05, 0, 1)
	if k == GoomeUI.K_DOWN or k == GoomeUI.K_LEFT:
		v[0] = clampf(v[0] - 0.05, 0, 1)
	if it.double_clicked:
		v[0] = start
	ui.store_set(title, v)
	ui.tooltip("drag up or down, arrows work too, double click resets it")

	var accent: Color = ui.theme_get("accent")
	var text: Color = ui.theme_get("text")
	var c := Vector2(it.x + it.w / 2, it.y + 40)
	var r := 31.0
	var ri := 21.0
	var a0 := 2.35619449
	var a1 := 7.06858347
	var at := a0 + (a1 - a0) * v[0]
	var lit: bool = it.hovered or it.held
	if lit:
		ui.draw_circle(c, r + 9, fade(accent, 0.16 if it.held else 0.09))
	ui.draw_arc(c, r, a0, a1, fade(text, 0.09), 6)
	if v[0] > 0.004:
		ui.draw_arc(c, r, a0, at, accent, 6)
	ui.draw_shadow(Rect2(c.x - ri, c.y - ri + 4, 2 * ri, 2 * ri), Color(0, 0, 0, 0.44), ri, 10)
	ui.draw_gradient(Rect2(c.x - ri, c.y - ri, 2 * ri, 2 * ri), Color("3a3f49"), Color("24272e"), ri)
	ui.draw_ring(c, ri - 0.5, Color(1, 1, 1, 0.1), 1)
	var dir := Vector2(cos(at), sin(at))
	ui.draw_line(c + dir * (ri - 15), c + dir * (ri - 7), text if lit else ui.theme_get("text_dim").lerp(text, 0.5), 2.6)
	text_at(ui, 14, c.x, it.y + 84, fmt % (v[0] * scale), text, true)
	text_at(ui, 12.5, c.x, it.y + 103, title, ui.theme_get("text_dim"), true)


func timeline(ui: GoomeUI, o: Vector2) -> void:
	var t := ui.store("playhead", 2)
	if not t[1]:
		t[0] = 2.4
		t[1] = 1
	var accent: Color = ui.theme_get("accent")
	var text: Color = ui.theme_get("text")
	var dim: Color = ui.theme_get("text_dim")
	var tr := ui.custom("track", 0, 92, GoomeUI.ITEM_NO_HIT)
	var x0: float = tr.x + 14
	var x1: float = tr.x + tr.w - 14
	var secs := 10.0
	var per := (x1 - x0) / secs
	var ly0: float = tr.y + 34
	var ly1: float = tr.y + 61
	ui.draw_rect(tr.rect, Color("1e2127"), 10.0)
	ui.draw_outline(tr.rect, ui.theme_get("border"), 1, 10.0)
	for i in 21:
		var x := x0 + i * per / 2
		var odd := i % 2 == 1
		ui.draw_rect(Rect2(x - 0.5, tr.y + (18 if odd else 12), 1, 5 if odd else 11), fade(text, 0.14 if odd else 0.28))
		if not odd and i < 20:
			ui.draw_text(Vector2(x + 4, tr.y + 9), "%ds" % (i >> 1), dim, 11)
	var purple := Color("b57cff")
	var blue := Color("4fb3f7")
	ui.draw_rect(Rect2(x0 + 0.4 * per, ly0, 3.6 * per, 22), fade(purple, 0.3), 6.0)
	ui.draw_outline(Rect2(x0 + 0.4 * per, ly0, 3.6 * per, 22), fade(purple, 0.65), 1, 6.0)
	ui.draw_text(Vector2(x0 + 0.4 * per + 9, ly0 + 4), "intro.wav", Color("e6daff"), 12)
	ui.draw_rect(Rect2(x0 + 3.1 * per, ly1, 5.3 * per, 22), fade(blue, 0.28), 6.0)
	ui.draw_outline(Rect2(x0 + 3.1 * per, ly1, 5.3 * per, 22), fade(blue, 0.6), 1, 6.0)
	ui.draw_text(Vector2(x0 + 3.1 * per + 9, ly1 + 4), "drums.wav", Color("d6eeff"), 12)

	ui.pos(x0 + t[0] * per - 9 - o.x, tr.y + 4 - o.y)
	ui.push_num(GoomeUI.S_RADIUS, 6)
	var hd := ui.custom("handle", 18, tr.h - 8, GoomeUI.ITEM_DRAG_X | GoomeUI.ITEM_FOCUSABLE)
	ui.pop(1)
	var k: int = hd.key & 0xFF
	if hd.held and per > 0:
		t[0] = clampf(t[0] + hd.dx / per, 0, secs)
	if k == GoomeUI.K_LEFT or k == GoomeUI.K_RIGHT:
		t[0] = clampf(snap(t[0], 0.5) + (0.5 if k == GoomeUI.K_RIGHT else -0.5), 0, secs)
	if hd.released:
		t[0] = snap(t[0], 0.1)
	ui.store_set("playhead", t)
	ui.tooltip("this one only moves sideways")
	var hx := x0 + t[0] * per

	for at in KEYFRAMES:
		var p := Vector2(x0 + at * per, ly1 + 11)
		var d := 4.5
		ui.draw_polygon(PackedVector2Array([p + Vector2(0, -d), p + Vector2(d, 0), p + Vector2(0, d), p + Vector2(-d, 0)]), accent if at <= t[0] else Color("d6eeff"))
	ui.draw_rect(Rect2(x0, tr.y + tr.h - 5, hx - x0, 2), fade(accent, 0.7))
	var hc := accent.lerp(Color.WHITE, 0.25) if hd.hovered or hd.held else accent
	ui.draw_rect(Rect2(hx - 1, tr.y + 20, 2, tr.h - 25), accent)
	ui.draw_shadow(Rect2(hx - 8, tr.y + 7, 16, 13), Color(0, 0, 0, 0.38), 4, 6)
	ui.draw_rect(Rect2(hx - 8, tr.y + 5, 16, 13), hc, 4.0)
	ui.draw_polygon(PackedVector2Array([Vector2(hx - 6, tr.y + 17), Vector2(hx + 6, tr.y + 17), Vector2(hx, tr.y + 23)]), hc)
	var s := "%.1fs" % t[0]
	var w := ui.text_size(s, 11.5).x
	var bx := clampf(hx + 12, tr.x + 6, tr.x + tr.w - w - 18)
	ui.draw_rect(Rect2(bx, tr.y + 4, w + 10, 16), fade(accent, 0.18), 5.0)
	ui.draw_text(Vector2(bx + 5, tr.y + 5), s, accent, 11.5)


func swatch_strip(ui: GoomeUI) -> void:
	for i in SWATCHES.size():
		if i:
			ui.same_line()
		var it := ui.custom("sw%d" % i, 34, 34, GoomeUI.ITEM_FOCUSABLE)
		if it.clicked:
			sel = i
		var lift := 2.0 if it.hovered and not it.held else 0.0
		var r: Rect2 = it.rect
		r.position.y -= lift
		var col: Color = SWATCHES[i]
		if i == sel:
			ui.draw_outline(r.grow(4), Color.WHITE, 2, 13.0)
		ui.draw_shadow(Rect2(r.position + Vector2(0, 3), r.size), Color(0, 0, 0, 0.4), 9, 8 + lift * 2)
		ui.draw_gradient(r, col.lerp(Color.WHITE, 0.12), col.lerp(Color.BLACK, 0.12), 9.0)
		ui.draw_outline(r, Color(1, 1, 1, 0.15), 1, 9.0)
		if i == sel:
			var c := r.get_center()
			var ink := Color(0.08, 0.09, 0.1, 0.88)
			ui.draw_line(c + Vector2(-6, 0), c + Vector2(-2, 4), ink, 2.4)
			ui.draw_line(c + Vector2(-2, 4), c + Vector2(6.5, -5), ink, 2.4)


func faders(ui: GoomeUI) -> void:
	var v := ui.store("eq", 8)
	if not v[7]:
		for i in 7:
			v[i] = EQ_START[i]
		v[7] = 1
	var accent: Color = ui.theme_get("accent")
	var text: Color = ui.theme_get("text")
	for i in 7:
		if i:
			ui.same_line()
		var it := ui.custom("eq%d" % i, 28, 138, GoomeUI.ITEM_DRAG_Y | GoomeUI.ITEM_FOCUSABLE)
		var top: float = it.y + 8
		var bot: float = it.y + it.h - 26
		var span := bot - top
		var k: int = it.key & 0xFF
		var grip := top + (1 - v[i]) * span
		if it.pressed and (it.my < grip - 8 or it.my > grip + 8):
			v[i] = clampf(1 - (it.my - top) / span, 0, 1)
		elif it.held:
			v[i] = clampf(v[i] - it.dy / span, 0, 1)
		if k == GoomeUI.K_UP or k == GoomeUI.K_DOWN:
			v[i] = clampf(v[i] + (0.05 if k == GoomeUI.K_UP else -0.05), 0, 1)
		if it.double_clicked:
			v[i] = 0.5
		var cx: float = it.x + it.w / 2
		var ty := top + (1 - v[i]) * span
		var mid := top + span / 2
		ui.draw_rect(Rect2(cx - 2, top, 4, span), fade(text, 0.09), 2.0)
		ui.draw_rect(Rect2(cx - 6, mid - 0.5, 12, 1), fade(text, 0.2))
		ui.draw_rect(Rect2(cx - 2, minf(ty, mid), 4, absf(mid - ty)), accent, 2.0)
		var lit: bool = it.hovered or it.held
		ui.draw_shadow(Rect2(cx - 11, ty - 3, 22, 10), Color(0, 0, 0, 0.44), 5, 6)
		ui.draw_gradient(Rect2(cx - 11, ty - 5, 22, 10), Color.WHITE if lit else Color("e4e7ec"), Color("dde1e7") if lit else Color("b9bec7"), 5.0)
		ui.draw_rect(Rect2(cx - 5, ty - 0.5, 10, 1), fade(accent, 0.9))
		text_at(ui, 11, cx, it.y + it.h - 9, HZ[i], text if lit else ui.theme_get("text_dim"), true)
	ui.store_set("eq", v)


func xy_pad(ui: GoomeUI) -> void:
	var v := ui.store("xy", 3)
	if not v[2]:
		v[0] = 0.62
		v[1] = 0.38
		v[2] = 1
	var it := ui.custom("xy", 0, 138, GoomeUI.ITEM_DRAG_X | GoomeUI.ITEM_DRAG_Y)
	if it.held:
		v[0] = clampf((it.mx - it.x - 8) / (it.w - 16), 0, 1)
		v[1] = clampf((it.my - it.y - 8) / (it.h - 16), 0, 1)
	ui.store_set("xy", v)
	var accent: Color = ui.theme_get("accent")
	var r: Rect2 = it.rect
	var p := r.position + Vector2(8 + v[0] * (r.size.x - 16), 8 + v[1] * (r.size.y - 16))
	ui.draw_rect(r, Color("1b1e23"), 12.0)
	ui.draw_gradient(r, fade(accent, 0), fade(accent, 0.1), 12.0, true)
	for i in range(1, 4):
		ui.draw_rect(Rect2(r.position.x + r.size.x * i / 4, r.position.y + 6, 1, r.size.y - 12), Color(1, 1, 1, 0.06))
		ui.draw_rect(Rect2(r.position.x + 6, r.position.y + r.size.y * i / 4, r.size.x - 12, 1), Color(1, 1, 1, 0.06))
	ui.draw_rect(Rect2(p.x - 0.5, r.position.y + 4, 1, r.size.y - 8), fade(accent, 0.35))
	ui.draw_rect(Rect2(r.position.x + 4, p.y - 0.5, r.size.x - 8, 1), fade(accent, 0.35))
	ui.draw_circle(p, 18 if it.held else 14, fade(accent, 0.22 if it.held else 0.14))
	ui.draw_circle(p, 7, accent)
	ui.draw_ring(p, 7, Color.WHITE, 2)
	ui.draw_text(r.position + Vector2(12, 9), "cutoff %.2f  res %.2f" % [v[0], 1 - v[1]], ui.theme_get("text_dim"), 11.5)
	ui.draw_outline(r, fade(accent, 0.6) if it.hovered or it.held else ui.theme_get("border"), 1, 12.0)


func node_box(ui: GoomeUI, d: Dictionary, it: Dictionary, at: Vector2, top: bool) -> void:
	var accent: Color = ui.theme_get("accent")
	var r := Rect2(at, it.rect.size)
	var col: Color = d.color
	var base := Color("2a2e36")
	ui.draw_shadow(Rect2(at + Vector2(0, 10 if top else 5), r.size), Color(0, 0, 0, 0.56 if top else 0.44), 12, 22 if top else 12)
	ui.draw_rect(r, base, 12.0)
	ui.draw_gradient(Rect2(at, Vector2(r.size.x, 26)), col.lerp(base, 0.25), col.lerp(base, 0.55), Vector4(12, 12, 0, 0), true)
	ui.draw_circle(at + Vector2(15, 13), 4, Color.WHITE)
	ui.draw_text(at + Vector2(26, 5), d.name, Color.WHITE, 13.5)
	ui.draw_text(at + Vector2(12, 35), d.info, ui.theme_get("text_dim"), 12)
	var lit: bool = it.held or it.hovered
	ui.draw_outline(r, accent.lerp(Color.WHITE, 0.2 if it.held else 0.0) if lit else Color(1, 1, 1, 0.13), 1.5 if lit else 1.0, 12.0)
	for side in [0.0, r.size.x]:
		ui.draw_circle(at + Vector2(side, 41), 5.5, Color("1a1c21"))
		ui.draw_ring(at + Vector2(side, 41), 5, col, 2)


func wire(ui: GoomeUI, a: Vector2, b: Vector2, from: Color, to: Color) -> void:
	var last := a
	var pull := clampf((b.x - a.x) * 0.5, 40, 200)
	for i in range(1, 33):
		var t := i / 32.0
		var p := a.bezier_interpolate(a + Vector2(pull, 0), b - Vector2(pull, 0), b, t)
		ui.draw_line(last, p, from.lerp(to, t - 1 / 64.0), 2.5)
		last = p


func node_area(ui: GoomeUI, o: Vector2) -> void:
	var ar := ui.custom("area", 0, 232, GoomeUI.ITEM_NO_HIT)
	var area: Rect2 = ar.rect
	var spots := []
	var items := []
	var top := -1
	for i in NODES.size():
		var d: Dictionary = NODES[i]
		var p := ui.store(d.name, 3)
		if not p[2]:
			p[0] = d.at.x
			p[1] = d.at.y
			p[2] = 1
		ui.pos(area.position.x - o.x + p[0], area.position.y - o.y + p[1])
		var it := ui.custom(d.name, 150, 58, GoomeUI.ITEM_DRAG_X | GoomeUI.ITEM_DRAG_Y)
		if it.held:
			p[0] += it.dx
			p[1] += it.dy
			top = i
		p[0] = clampf(p[0], 8, area.size.x - it.w - 8)
		p[1] = clampf(p[1], 8, area.size.y - it.h - 8)
		if it.released:
			p[0] = clampf(snap(p[0], 20), 0, area.size.x - it.w)
			p[1] = clampf(snap(p[1], 20), 0, area.size.y - it.h)
		ui.store_set(d.name, p)
		spots.append(Vector2(p[0], p[1]))
		items.append(it)
	var accent: Color = ui.theme_get("accent")
	ui.draw_rect(area, Color("17191e"), 12.0)
	ui.push_clip(area.grow(-1))
	for y in range(20, int(area.size.y), 20):
		for x in range(20, int(area.size.x), 20):
			ui.draw_rect(Rect2(area.position + Vector2(x - 1, y - 1), Vector2(2, 2)), Color(1, 1, 1, 0.086))
	if top >= 0:
		var size: Vector2 = items[top].rect.size
		var s := Vector2(clampf(snap(spots[top].x, 20), 0, area.size.x - size.x), clampf(snap(spots[top].y, 20), 0, area.size.y - size.y))
		ui.draw_rect(Rect2(area.position + s, size), fade(accent, 0.08), 12.0)
		ui.draw_outline(Rect2(area.position + s, size), fade(accent, 0.45), 1.5, 12.0)
	for i in NODES.size() - 1:
		var a: Vector2 = area.position + spots[i] + Vector2(items[i].w, 41)
		var b: Vector2 = area.position + spots[i + 1] + Vector2(0, 41)
		wire(ui, a, b, NODES[i].color, NODES[i + 1].color)
	for i in NODES.size():
		if i != top:
			node_box(ui, NODES[i], items[i], area.position + spots[i], false)
	if top >= 0:
		node_box(ui, NODES[top], items[top], area.position + spots[top], true)
	ui.draw_text(area.position + Vector2(14, area.size.y - 24), "drag the boxes, they snap to the grid when you let go", fade(ui.theme_get("text_dim"), 0.8), 12)
	ui.pop_clip()
	ui.draw_outline(area, ui.theme_get("border"), 1, 12.0)


func button_rows(ui: GoomeUI) -> void:
	ui.heading("Buttons your way")
	ui.wrapped("push a radius, a color or per corner radii, then pop it. only the widgets in between change.")
	ui.button("Normal")
	ui.same_line()
	ui.push_num(GoomeUI.S_RADIUS, 0)
	ui.button("Square")
	ui.pop(1)
	ui.same_line()
	ui.push_num(GoomeUI.S_RADIUS, 999)
	ui.button("Pill")
	ui.pop(1)
	ui.same_line()
	ui.push_radii(16, 3, 16, 3)
	ui.button("Leaf")
	ui.pop(1)

	ui.push_num(GoomeUI.S_GAP, 1)
	var names := ["Day", "Week", "Month", "Year"]
	for i in 4:
		if i:
			ui.same_line()
		var left := 9.0 if i == 0 else 2.0
		var right := 9.0 if i == 3 else 2.0
		ui.push_radii(left, right, right, left)
		if seg == i:
			ui.push_style({"input_bg": Color("3b4250"), "border": Color(1, 1, 1, 0.25)})
		else:
			ui.push_style({"text": Color("969ca6"), "border": Color(0, 0, 0, 0)})
		ui.size(74, 0)
		if ui.button(names[i]):
			seg = i
		ui.pop(3)
	ui.pop(1)
	ui.space(8)

	var n := ui.push_style({"input_bg": Color("6fdc5f"), "hover": Color("86e878"), "active": Color("58c24a"), "text": Color("0f1d0c"), "border": Color(0, 0, 0, 0), "radius": 999.0})
	ui.button("Save it")
	ui.pop(n)
	ui.same_line()
	n = ui.push_style({"input_bg": Color(0, 0, 0, 0), "hover": Color("e5534b", 0.14), "active": Color("e5534b", 0.25), "text": Color("ff7a72"), "border": Color("e5534b", 0.5), "border_width": 1.5})
	ui.button("Delete")
	ui.pop(n)
	ui.same_line()
	ui.push_num(GoomeUI.S_FONT_SIZE, 19)
	ui.push_num(GoomeUI.S_PAD, 14)
	ui.push_radii(4, 18, 4, 18)
	ui.button("Big one")
	ui.pop(3)

	ui.push_radii(12, 12, 0, 0)
	ui.push_num(GoomeUI.S_GAP, 2)
	var tabs := ["Files", "Search", "Git"]
	for i in 3:
		if i:
			ui.same_line()
		var here := tab == i
		ui.push_color(GoomeUI.S_INPUT_BG, Color("2f333c") if here else Color("1f2228"))
		ui.push_color(GoomeUI.S_BORDER, Color(1, 1, 1, 0.19) if here else Color(0, 0, 0, 0))
		ui.push_color(GoomeUI.S_TEXT, Color("e9ebee") if here else Color("8c929c"))
		if ui.button(tabs[i]):
			tab = i
		ui.pop(3)
	ui.pop(2)


func more_widgets(ui: GoomeUI) -> void:
	ui.heading("Every widget reads it")
	ui.push_color(GoomeUI.S_ACCENT, Color("ff6fa8"))
	ui.push_color(GoomeUI.S_ACCENT_TEXT, Color("2a0614"))
	ui.row(2)
	pink = ui.checkbox("Pink", pink)
	pink = ui.toggle("Also pink", pink)
	amount = ui.slider("Amount", amount, 0.0, 1.0, "%.2f")
	ui.pop(2)
	ui.push_num(GoomeUI.S_RADIUS, 999)
	find = ui.input("##find", find, 63)
	ui.pop(1)
	ui.push_num(GoomeUI.S_RADIUS, 0)
	ui.push_color(GoomeUI.S_ACCENT, Color("4fb3f7"))
	ui.push_color(GoomeUI.S_ACCENT_TEXT, Color("06202e"))
	ui.progress(amount, "square corners")
	ui.pop(3)


func light_card(ui: GoomeUI) -> void:
	var spot := ui.custom("light-spot", 0, 262, GoomeUI.ITEM_NO_HIT)
	ui.push_theme("light", {"radius": 8.0, "shadow": Color(0, 0, 0, 0.38)})
	var green: Color = ui.theme_get("accent")
	ui.set_window_rect("A whole other theme", spot.rect)
	if ui.window("A whole other theme", spot.rect, 0):
		on = ui.toggle("Wifi", on)
		vol = ui.slider("Volume", vol, 0.0, 1.0, "%.2f")
		ui.row(2)
		pick = ui.radio("Small", pick, 0)
		pick = ui.radio("Large", pick, 1)
		room = ui.input("##name", room, 63)
		ui.push_num(GoomeUI.S_RADIUS, 999)
		ui.button("Cancel")
		ui.same_line()
		ui.push_style({"input_bg": green, "hover": Color("55c944"), "active": Color("3aa12b"), "text": Color.WHITE, "border": Color(0, 0, 0, 0)})
		ui.button("Apply")
		ui.pop(6)
	ui.window_end()
	ui.pop(1)


func _goome_frame(ui: GoomeUI) -> void:
	ui.app_begin("Custom", GoomeUI.APP_STATUSBAR | GoomeUI.APP_NO_PAD)
	ui.split_begin("main", false, 0.4)
	button_rows(ui)
	ui.space(4)
	more_widgets(ui)
	ui.space(6)
	light_card(ui)
	ui.split_next()

	ui.pos(0, 0)
	var o: Vector2 = ui.custom("##origin", 1, 1, GoomeUI.ITEM_NO_HIT).rect.position
	ui.push_color(GoomeUI.S_ACCENT, SWATCHES[sel])
	ui.heading("Custom widgets")
	ui.pos(176, 3)
	ui.badge("pinned with ui.pos", Color(1, 1, 1, 0.15))
	ui.space(2)
	knob(ui, "Gain", 0.62, 12, "%.1f dB")
	ui.same_line()
	knob(ui, "Tone", 0.35, 100, "%.0f%%")
	ui.same_line()
	knob(ui, "Mix", 0.8, 100, "%.0f%%")
	ui.same_line()
	knob(ui, "Width", 0.5, 200, "%.0f%%")
	ui.space(4)
	timeline(ui, o)
	ui.space(4)
	swatch_strip(ui)
	ui.space(4)
	node_area(ui, o)
	ui.space(4)
	faders(ui)
	ui.same_line()
	xy_pad(ui)
	var gain := ui.store("Gain", 2)
	var head := ui.store("playhead", 2)
	ui.pop(1)
	ui.split_end()
	ui.statusbar("gain %.1f dB   playhead %.1fs" % [gain[0] * 12, head[0]])
	ui.app_end()
