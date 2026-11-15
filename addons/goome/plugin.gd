@tool
extends EditorPlugin


func _enter_tree() -> void:
	if not ClassDB.class_exists("GoomeView"):
		push_warning("goome: the native lib didnt load, check addons/goome/bin has one for this os")
