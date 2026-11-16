extends GoomeView

var on := false
var dropped := false
var box := 0.0
var sp := 0.0
var col := Color(0, 0, 0, 0)
var last := 0.0
var n := 0
var log := []
var final_last := -1.0


func _init() -> void:
	redraw_every_frame = true
	sync_clipboard = false
	pixel_scale = 1
	fixed_step = true


func argb_fnv(img: Image) -> int:
	var d := img.get_data()
	var h := 2166136261
	var i := 0
	while i < d.size():
		h = ((h ^ d[i + 2]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i + 1]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i]) * 16777619) & 0xFFFFFFFF
		h = ((h ^ d[i + 3]) * 16777619) & 0xFFFFFFFF
		i += 4
	return h


func _goome_frame(ui: GoomeUI) -> void:
	if n > 0 and n < 70:
		log[n - 1].hash = argb_fnv(get_image())
		log[n - 1].animating = ui.animating()
	on = n >= 2 and n != 62 and n != 63 and n < 1665
	dropped = n >= 65 and n < 1665
	box = ui.anim("box", 200.0 if on else 10.0, 160, GoomeUI.EASE_OUT)
	sp = ui.anim("spring", 1.0 if on else 0.0, 320, GoomeUI.EASE_SPRING)
	col = ui.anim_color("col", Color(0x20D060FF) if on else Color(0, 0, 0, 0), 200, GoomeUI.EASE_LINEAR)
	if not dropped:
		last = ui.anim("gone", 5.0 if on else 0.0, 100, GoomeUI.EASE_LINEAR)
	ui.draw_rect(Rect2(box, 20, 60, 40), Color(0xE0A030FF))
	ui.draw_rect(Rect2(20 + sp * 180, 90, 50, 30), Color(0x4FB3F7FF), 15.0)
	ui.draw_rect(Rect2(20, 140, 260, 40), col)
	if n == 1665:
		final_last = last
	if n < 70:
		log.append({"box": box, "sp": sp, "col": col, "last": last, "hash": 0, "animating": false})
	n += 1
