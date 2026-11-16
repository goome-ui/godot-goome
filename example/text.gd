extends GoomeView

const LINES := [
	"goome 😀 🎉 👍🏽 👨‍👩‍👧 🇩🇪 #️⃣ ❤️ 🐱",
	"السلام عليكم، كيف حالك؟",
	"שלום עולם, מה שלומך?",
	"goome says مرحبا (123) to שלום 👋",
	"لا لأن محمد كتب 2024 [ok]",
]

var dir := GoomeUI.DIR_AUTO
var name_text := "العربية abc 😀"
var notes := "שלום! this line starts in hebrew\nmixed: abc عربي 42 👍🏽\nمرحبا بكم"


func _goome_frame(ui: GoomeUI) -> void:
	ui.text_dir(dir)
	if ui.app_begin("goome text"):
		ui.heading("Emoji, Arabic, Hebrew")
		for line in LINES:
			ui.label(line)
		ui.separator()
		ui.row(3)
		dir = ui.radio("Auto", dir, GoomeUI.DIR_AUTO)
		dir = ui.radio("Left to right", dir, GoomeUI.DIR_LTR)
		dir = ui.radio("Right to left", dir, GoomeUI.DIR_RTL)
		ui.row(1)
		name_text = ui.input("Name", name_text, 120)
		notes = ui.text_edit("notes", notes, 120, GoomeUI.EDIT_WRAP)
		ui.app_end()
