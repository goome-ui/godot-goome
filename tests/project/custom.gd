extends GoomeView

var styled := 0
var leave_pushed := false
var pushed := 0
var origin := Vector2.ZERO
var spot := {}
var free_box := {}
var x_box := {}
var knob := {}
var free_moved := Vector2.ZERO
var free_drag := Vector2.ZERO
var free_released := 0
var x_moved := Vector2.ZERO
var x_drag := Vector2.ZERO
var keys: Array[int] = []
var knob_clicks := 0
var presses := 0
var fresh := PackedFloat64Array()
var text_width := 0.0
var text_box := Vector2.ZERO
var skin := Image.create_empty(8, 8, false, Image.FORMAT_RGBA8)


func _ready() -> void:
	skin.fill(Color(1, 0, 0))


func _goome_frame(ui: GoomeUI) -> void:
	ui.pop(3)
	ui.pos(0, 0)
	origin = ui.custom("origin", 1, 1, GoomeUI.ITEM_NO_HIT).rect.position

	if styled == 1:
		ui.push_num(GoomeUI.S_RADIUS, 0)
		ui.push_color(GoomeUI.S_INPUT_BG, Color(1, 0, 0))
	elif styled == 2:
		pushed = ui.push_style({"radius": 0.0, "input_bg": Color(1, 0, 0)})
	elif styled == 3:
		ui.push_theme("light", {"radius": 0.0, "input_bg": Color(1, 0, 0)})
		ui.push_font(null)
	elif styled == 4:
		ui.push_skin(GoomeUI.SKIN_BUTTON, skin, 2, 2, 2, 2)
		ui.push_font(null)
	ui.pos(20, 20)
	ui.size(120, 40)
	ui.button("One")
	if styled:
		ui.pop(2)
	ui.pos(20, 80)
	ui.size(120, 40)
	ui.button("Two")

	ui.pos(200, 30)
	spot = ui.custom("spot", 50, 30)
	ui.draw_rect(spot.rect, Color(0, 1, 0))

	ui.pos(300, 100)
	free_box = ui.custom("free", 60, 60, GoomeUI.ITEM_DRAG_X | GoomeUI.ITEM_DRAG_Y)
	ui.draw_rect(free_box.rect, Color(0.3, 0.3, 0.8), 8.0)
	var count := ui.store("presses", 1)
	if free_box.pressed:
		count[0] += 1
		ui.store_set("presses", count)
	presses = int(count[0])
	if free_box.held:
		free_moved += Vector2(free_box.dx, free_box.dy)
		free_drag = Vector2(free_box.drag_x, free_box.drag_y)
	if free_box.released:
		free_released += 1

	ui.pos(300, 200)
	x_box = ui.custom("x", 60, 40, GoomeUI.ITEM_DRAG_X)
	ui.draw_rect(x_box.rect, Color(0.8, 0.3, 0.3), Vector4(8, 0, 8, 0))
	if x_box.held:
		x_moved += Vector2(x_box.dx, x_box.dy)
		x_drag = Vector2(x_box.drag_x, x_box.drag_y)

	ui.pos(420, 100)
	knob = ui.custom("knob", 60, 60, GoomeUI.ITEM_FOCUSABLE)
	if knob.key:
		keys.append(knob.key & 0xFF)
	if knob.clicked:
		knob_clicks += 1
	var center := Vector2(knob.x + 30, knob.y + 30)
	ui.draw_circle(center, 24, Color(0.2, 0.2, 0.2))
	ui.draw_ring(center, 24, Color.WHITE if knob.focused else Color.GRAY, 2)
	ui.draw_arc(center, 18, 2.4, 7.0, Color.ORANGE, 4)
	ui.draw_line(center, center + Vector2(0, -16), Color.WHITE, 2)

	fresh = ui.store("never", 3)

	ui.pos(20, 300)
	var area := ui.custom("area", 200, 100, GoomeUI.ITEM_NO_HIT)
	ui.push_clip(area.rect)
	ui.draw_shadow(area.rect, Color(0, 0, 0, 0.5), 10, 12)
	ui.draw_gradient(area.rect, Color.DARK_SLATE_GRAY, Color.BLACK, 10.0, true)
	ui.draw_outline(area.rect, Color.WHITE, 1.5, 10.0)
	ui.draw_polygon(PackedVector2Array([area.rect.position + Vector2(10, 90), area.rect.position + Vector2(40, 60), area.rect.position + Vector2(70, 90)]), Color.YELLOW)
	text_width = ui.draw_text(area.rect.position + Vector2(10, 10), "hello", Color.WHITE)
	text_box = ui.text_size("hello")
	ui.pop_clip()

	if leave_pushed:
		ui.push_color(GoomeUI.S_INPUT_BG, Color(0, 0, 1))
