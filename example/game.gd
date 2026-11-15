extends Node2D

var spin := 1.0
var size := 80.0
var color := Color("6fdc5f")
var angle := 0.0
var spot := Vector2(576, 324)
var dots: Array[Vector2] = []


func _process(delta: float) -> void:
	angle += spin * delta
	var dir := Vector2.ZERO
	if Input.is_physical_key_pressed(KEY_W):
		dir.y -= 1
	if Input.is_physical_key_pressed(KEY_S):
		dir.y += 1
	if Input.is_physical_key_pressed(KEY_A):
		dir.x -= 1
	if Input.is_physical_key_pressed(KEY_D):
		dir.x += 1
	if not $UI/Menu.wants_keyboard():
		spot += dir * 300 * delta
	queue_redraw()


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		dots.append(event.position)
		if dots.size() > 40:
			dots.pop_front()


func _draw() -> void:
	draw_rect(Rect2(Vector2.ZERO, get_viewport_rect().size), Color("1b1d22"))
	for d in dots:
		draw_circle(d, 6, Color(1, 1, 1, 0.5))
	draw_set_transform(spot, angle)
	draw_rect(Rect2(-size / 2, -size / 2, size, size), color)
	draw_set_transform(Vector2.ZERO)
