<p align="center">
  <img src="https://raw.githubusercontent.com/goome-ui/.github/main/profile/logo.png" alt="goome" width="150">
</p>

<h1 align="center">🤖 godot-goome 🤖</h1>

<p align="center">
<b>goome's immediate mode ui inside Godot. One Control node, you write the ui from GDScript every frame.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Godot-4.4%2B-6fdc5f?style=flat-square&logo=godotengine&logoColor=1b1d22&labelColor=6fdc5f" alt="Godot 4.4+">
  <img src="https://img.shields.io/badge/GDExtension-plain%20C-1b1d22?style=flat-square&labelColor=6fdc5f" alt="GDExtension in plain C">
  <img src="https://img.shields.io/badge/Linux-x86__64-1b1d22?style=flat-square&logo=linux&logoColor=white" alt="Linux">
  <img src="https://img.shields.io/badge/Windows-x86__64-1b1d22?style=flat-square&logo=windows11&logoColor=white" alt="Windows">
  <img src="https://img.shields.io/badge/macOS-arm64%20%2B%20x86__64-1b1d22?style=flat-square&logo=apple&logoColor=white" alt="macOS">
  <img src="https://img.shields.io/badge/license-GPL--3.0-1b1d22?style=flat-square" alt="GPL-3.0">
</p>

<p align="center">
  <a href="https://goome-ui.github.io/docs/">Docs</a> &nbsp;·&nbsp;
  <a href="https://goome-ui.github.io/docs/builder/">Builder</a> &nbsp;·&nbsp;
  <a href="https://github.com/goome-ui/examples">Examples</a> &nbsp;·&nbsp;
  <a href="https://github.com/goome-ui/c-goome">C</a> &nbsp;·&nbsp;
  <a href="https://github.com/goome-ui/csharp-goome">C#</a> &nbsp;·&nbsp;
  <a href="https://github.com/goome-ui/python-goome">Python</a> &nbsp;·&nbsp;
  <a href="https://github.com/goome-ui/java-goome">Java</a>
</p>

<p align="center">
  This is <a href="https://github.com/goome-ui/c-goome">goome</a> as a Godot addon. You drop a <code>GoomeView</code> into your scene, write <code>_goome_frame(ui)</code>, and you get goome's whole ui kit on top of your game, windows and sliders and text boxes and tables.<br>
  goome draws offscreen into a texture, so it works with every renderer (Forward+, Mobile, Compatibility). Godot's mouse and keys get fed into it, and while the ui wants them your game doesnt see them. Clicks that miss the ui go straight through to the game.<br>
  It's a GDExtension written in plain C against Godot's own <code>gdextension_interface.h</code>, no godot-cpp, no C# needed.<br> <br>
  If you want a see-through overlay over the whole desktop, a normal app window or a mod for a game you didnt make, thats the other goome repos. <a href="https://github.com/goome-ui/c-goome">c-goome</a> is the main one, <a href="https://github.com/goome-ui/csharp-goome">csharp-goome</a> has the Unity mod templates and <a href="https://github.com/goome-ui/java-goome">java-goome</a> does Minecraft mods.<br>
  The <a href="https://goome-ui.github.io/docs/builder/">ui builder</a> lets you click a ui together in the browser before you write any of it.
</p>

<h3 align="center">🎮 A menu in 20 lines 🎮</h3>

```gdscript
extends GoomeView

var volume := 0.5
var player := "goome"
var god_mode := false


func _goome_frame(ui: GoomeUI) -> void:
	if ui.window("Settings", Rect2(40, 40, 300, 0)):
		volume = ui.slider("Volume", volume, 0.0, 1.0, "%.2f")
		player = ui.input("Name", player)
		god_mode = ui.toggle("God mode", god_mode)
		if ui.button("Respawn"):
			get_tree().reload_current_scene()
	ui.window_end()
```

<p align="center">
Put that script on a <code>GoomeView</code> that fills the screen (anchors Full Rect, under a CanvasLayer if your game has a camera) and that's it.
</p>

<details>
<summary align="center"><b>⬇️ Get It ⬇️</b></summary>

<br>

<p align="center">
The easy way: download <code>godot-goome-0.3.0.zip</code> from the <a href="https://github.com/goome-ui/godot-goome/releases">releases page</a> and unzip it into your project. You end up with <code>res://addons/goome/</code> with the libs for every OS already in <code>bin/</code>. Open the project and <code>GoomeView</code> shows up in the Add Node list.<br> <br>
Every version gets its own zip on there, so for another one just pick its release. Building a version yourself is <code>git clone -b v0.3.0 https://github.com/goome-ui/godot-goome</code> and the steps below.<br> <br>
Or build it yourself, you only need a C compiler (gcc or clang, mingw gcc for windows):
</p>

```sh
git clone https://github.com/goome-ui/godot-goome
cd godot-goome
./build.sh              # the lib for this machine into addons/goome/bin/
./build.sh win          # goome.windows.x86_64.dll with mingw, works from linux too
./build.sh universal    # on a mac, one dylib for arm64 and x86_64
cp -r addons/goome /path/to/your/project/addons/
```

<p align="center">
Turning the plugin on in Project Settings > Plugins is optional, the node works without it. The plugin only tells you when the lib for your OS is missing.<br> <br>
On a mac, if you downloaded the zip, macOS might put it in quarantine. <code>xattr -dr com.apple.quarantine addons/goome</code> fixes that.
</p>
</details>

<details>
<summary align="center"><b>🚀 Quick Start 🚀</b></summary>

<br>

<p align="center">
There are two ways to write the ui. Extend <code>GoomeView</code> and write <code>_goome_frame</code> like above, or keep your own script somewhere else and connect to the <code>goome_frame</code> signal:
</p>

```gdscript
extends Node2D

var hp := 100

func _ready() -> void:
	$UI/GoomeView.goome_frame.connect(draw_ui)

func draw_ui(ui: GoomeUI) -> void:
	if ui.window("Player", Rect2(20, 20, 220, 0), GoomeUI.WIN_MOVE | GoomeUI.WIN_COLLAPSE):
		hp = ui.slider_int("HP", hp, 0, 100)
		ui.progress(hp / 100.0, "%d hp" % hp)
	ui.window_end()
```

<p align="center">
Its immediate mode, so the values live in your variables, not in the widgets. Every widget takes the current value and gives back the new one. Change <code>hp</code> from anywhere else and the slider just shows it next frame.<br> <br>
<code>example/</code> is a whole little project: a spinning square you control from a goome window, with tabs, a color picker, a table, a modal plus a text editor. Copy the addon in and open it:
</p>

```sh
./build.sh
cp -r addons/goome example/addons/
godot -e --path example
```
</details>

<details>
<summary align="center"><b>🎛️ UI Kit 🎛️</b></summary>

<br>

<p align="center">
Everything on <code>GoomeUI</code>, the thing you get in <code>_goome_frame(ui)</code>:
</p>

| | |
|:--|:--|
| `window(title, rect, flags)` / `window_end()` | a window. `rect` is only the start spot and size (h 0 = fit the content). Always call `window_end`, even when it returned false. `get_window_rect`, `set_window_rect`, `is_window_open`, `set_window_open` save and restore the layout |
| `button` `checkbox` `toggle` `radio` | `if ui.button("Hi"):`, `on = ui.checkbox("On", on)`, `mode = ui.radio("Big", mode, 2)` |
| `slider` `slider_int` `drag_float` `drag_int` `number` | number widgets, they give back the new value |
| `input(label, text, max_length)` / `text_edit(id, text, height, flags)` | one line and multiline text, with `GoomeUI.EDIT_*` flags |
| `input_ex(label, text, max_length, flags, hint)` / `password(label, text, max_length, hint)` | input with extras. `hint` is the grey text while its empty, flags are `GoomeUI.INPUT_PASSWORD`, `INPUT_READONLY`, `INPUT_NUMBER`, `INPUT_NO_SELECT_ON_FOCUS` and `INPUT_ENTER_RETURNS`. `entered()` is true when enter got hit, `password` turns that on for you |
| `combo(label, selected, items)` / `tabs(id, names, selected)` | take a `PackedStringArray`, give back the index |
| `color(label, color)` | a color picker, takes a Color and gives one back |
| `label` `heading` `wrapped` `link` `badge` `progress` `spinner` | text and small stuff |
| `row(cols)` `same_line()` `space(px)` `separator()` `width(w)` | layout |
| `collapsing(label, open)` / `tree(label, open)` + `tree_end()` | sections that open and close, `open` is just how it starts and goome remembers it after that |
| `scroll_begin(id, h)` / `scroll_end()` | scroll area, always call the end |
| `menubar_begin` `menu_begin` `menu_item` `menu_check` `menu_separator` | menus, the ends only when the begin returned true |
| `context_begin(id)` / `popup_open(id)` `popup_begin(id)` `popup_close()` | right click menus and popups |
| `modal_open(title)` / `modal_begin(title)` + `modal_end()` | a centered dialog. Open it from a button, esc or the X closes it |
| `table_begin` `table_column` `table_sort` `table_row` `table_next` `table_end` | tables, `GoomeUI.TABLE_*` makes them sortable or resizable |
| `list_clip(count, row_h)` | for huge lists, gives back a `Vector2i` of the rows to draw |
| `split_begin(id, vertical, fraction)` `split_next` `split_end` | split panes |
| `plot_lines` / `plot_bars` | graphs from a `PackedFloat32Array` |
| `app_begin(title, flags)` / `app_end()` | a root panel filling the whole view, for full screen tools |
| `toast(text, ms)` / `statusbar(text)` | little popups and the status line |
| `tooltip` `hovered` `focused` `set_focus` `push_id` `pop_id` | for the widget right before it, and ids for loops |
| `changed()` | if the last widget changed its value this frame |
| `theme_dark()` `theme_light()` `theme_set(name, value)` `theme_get(name)` | `ui.theme_set("accent", Color.ORANGE)`, `ui.theme_set("radius", 4.0)`. Names are the goome_theme fields: bg, panel, border, text, text_dim, accent, accent_text, hover, active, input_bg, shadow, radius, pad, gap, border_width, font_size, shadow_blur and anim_ms, the <a href="https://goome-ui.github.io/docs/">docs</a> have the rest |

<p align="center">
Same labels in a loop need <code>ui.push_id(str(i))</code> first, same as in C. Begin and end rules are the C ones, the <a href="https://goome-ui.github.io/docs/">docs</a> list which ends you always need.<br> <br>
The ui only works inside <code>_goome_frame</code> or the signal. Call it anywhere else and you get one error and nothing happens.
</p>
</details>

<details>
<summary align="center"><b>🎨 Changing The Look 🎨</b></summary>

<br>

<p align="center">
Pick a preset once and the whole thing follows it. For one widget, push a style before it and pop it after:
</p>

```gdscript
var themed := false


func _goome_frame(ui: GoomeUI) -> void:
	if not themed:
		themed = ui.theme_preset(GoomeUI.PRESET_MIDNIGHT)
	var n := ui.push_style({"radius": 999.0, "accent": Color.HOT_PINK})
	if ui.button("Pink pill"):
		print("clicked")
	ui.pop(n)
```

<p align="center">
Fonts, icons, nine slice skins, your own widgets and the full list of theme names are all in the <a href="https://goome-ui.github.io/docs/">docs</a>. <code>example/custom.tscn</code>, <code>skinned.tscn</code> and <code>anything.tscn</code> are good ones to poke at too.
</p>
</details>

<details>
<summary align="center"><b>🖱️ Mouse And Keys 🖱️</b></summary>

<br>

<p align="center">
The view only counts as "there" where goome drew something that takes clicks. Godot asks it with <code>_has_point</code>, so a click next to a goome window goes to whatever is under it, your other Controls or <code>_unhandled_input</code> in your game.<br> <br>
When you start dragging a slider or a window, the view keeps the mouse until you let go, even when you move off the ui. Those moves are eaten with <code>accept_event()</code>, so a camera that looks around in <code>_unhandled_input</code> doesnt turn while you drag. Popups and menus take the whole screen until they close.<br> <br>
Keys only go to goome while a text box has focus (or a modal is open). Then theyre eaten too, so typing "w" in a name box doesn't walk your player forward. Click anywhere next to the ui and the keys go back to the game.<br> <br>
If your game polls <code>Input.is_action_pressed</code>, nothing gets eaten there, since Godot doesnt route that through the GUI. Ask the view instead:
</p>

```gdscript
if not $UI/GoomeView.wants_keyboard():
	move_player(delta)
```

<p align="center">
Dont override <code>_gui_input</code>, <code>_has_point</code> or <code>_input</code> in a script that extends <code>GoomeView</code>, thats where the view does its thing. Use another node for your own input.
</p>
</details>

<details>
<summary align="center"><b>🔧 The Node 🔧</b></summary>

<br>

| | |
|:--|:--|
| `redraw_every_frame` | default on. Off means it only draws when input came in, something animates or you call `redraw()`. Turn it off if your ui doesn't show live values |
| `take_keyboard` | default on. Off and goome never gets keys |
| `sync_clipboard` | default on. ctrl+v pastes from the system clipboard, ctrl+c / ctrl+x copy to it, through DisplayServer |
| `light_theme` | starts on the light theme |
| `pixel_scale` | texture px per ui px. 0 (default) follows the view's screen scale, so its sharp with stretch modes and hidpi |
| `wants_mouse()` / `wants_keyboard()` | what goome wants right now |
| `hit(position)` | does that local spot hit the ui |
| `get_texture()` / `get_image()` | the ImageTexture it draws, and the Image behind it, if you want the ui on a mesh or a screenshot |
| `push_text(text)` | type text into the focused box from code |
| `goome_frame(ui)` | the signal, every frame goome draws |

<p align="center">
The mouse cursor follows what goome asks for (text cursor over inputs, hand over links, resize arrows on window edges) through <code>mouse_default_cursor_shape</code>.<br> <br>
goome only redraws the part of the frame that changed and only that rect gets copied into the Image. Godot doesnt let you upload part of a texture, so the GPU upload is the whole texture, but only on frames where something changed.
</p>
</details>

<details>
<summary align="center"><b>🧩 Other Engines 🧩</b></summary>

<br>

<p align="center">
This addon is just goome's offscreen mode with Godot glued on. Any engine that can put pixels in a texture can do the same thing:
</p>

```c
goome_opts o = {.mode = GOOME_MODE_OFFSCREEN, .width = 800, .height = 600, .scale = 1};
goome *g = goome_open(&o, NULL);
goome_ui *u = goome_ui_new(g);

goome_push(g, &event);                        // your engine's mouse and keys
goome_ui_begin(u, 0); /* widgets */ goome_ui_end(u);
goome_rect r;
if (goome_frame_dirty(g, &r))                 // only what changed
    goome_frame_copy(g, pixels, pitch, GOOME_FMT_RGBA_PREMUL, &r);
if (goome_ui_wants(u) & GOOME_WANT_MOUSE) { } // don't hand the mouse to the game
goome_hit(g, x, y);                           // did that click land on the ui
goome_cursor_shape(g);                        // what cursor to show
```

<p align="center">
Then draw one premultiplied alpha quad. The <a href="https://github.com/goome-ui/examples">goome-ui/examples</a> repo has small ones for raylib, SDL or love2d, and the <a href="https://goome-ui.github.io/docs/">docs</a> have a page per engine.<br> <br>
If you dont want to write that glue yourself, c-goome has ready made <a href="https://github.com/goome-ui/c-goome/tree/main/backends">backends</a> for dx9 up to dx12, vulkan, metal or opengl. Input glue for win32, glfw or sdl is in there too.
</p>
</details>

<details>
<summary align="center"><b>🖥️ Where It Works 🖥️</b></summary>

<br>

<p align="center">
Godot 4.4 and newer, the header in <code>native/gdextension/</code> is from 4.7.2 and CI runs the test on 4.4.1 and 4.7.2.<br> <br>
The zip has x86_64 libs for linux and windows, plus a universal mac lib (arm64 and x86_64). Linux arm64 works too, run <code>./build.sh</code> on it. Android, iOS or web arent built yet.<br> <br>
Only normal single precision Godot builds, not the double precision ones.
</p>
</details>

<details>
<summary align="center"><b>🧪 Testing Without A Screen 🧪</b></summary>

<br>

<p align="center">
The test is a real Godot project in <code>tests/project</code> that runs headless. It clicks a button, drags a slider off the ui, types into a box and clicks next to the window, and checks the game got exactly the input it should have and the frame got drawn. It also draws a window full of custom icons and skins and checks the pixels hash the same as <code>tests/icons.c</code> does in plain C:
</p>

```sh
./build.sh
tests/run.sh /path/to/godot
```

<p align="center">
CI does that on all 3 desktops with the Godot 4.7.2 binary, plus linux again with 4.4.1, and runs the example for a bit to catch script errors.
</p>
</details>

<details>
<summary align="center"><b>🛠 Pull Requests 🛠</b></summary>

<br>

<p align="center">
Want to help with godot-goome? Make A pull request to <b>dev</b>!<br>
Small fixes are the easiest to merge, for anything bigger open an issue first so we can talk about it. Bugs in the ui kit itself go to <a href="https://github.com/goome-ui/c-goome">c-goome</a>.<br> <br>
As long as it works and doesnt add a dependency its very likely to get accepted!<br>
Read <a href="CONTRIBUTING.md">CONTRIBUTING.md</a> before you send one.
</p>
</details>

<details>
<summary align="center"><b>⚖️ License ⚖️</b></summary>

<br>

<p align="center">
godot-goome is under the <a href="LICENSE">GPL-3.0</a>.<br> <br>
<sub>
• You can use it, change it and share it<br>
• If you share something made with it, you have to share your source too, under the same license<br>
• Keep the credits
</sub><br> <br>
In short: play fair, keep it open, and give credit where it's due.<br> <br>
<code>native/gdextension/gdextension_interface.h</code> is Godot's, under the MIT license in <code>native/gdextension/LICENSE.txt</code>.
</p>
</details>

<details>
<summary align="center"><b>🩷 Donate 🩷</b></summary>

<br>

<p align="center">
<a href="https://sevvy-wevvy.com/kofi">Ko-fi</a> (Preferred)<br>
<a href="https://sevvy-wevvy.com/patreon">Patreon</a><br>
<a href="https://sevvy-wevvy.com/throne">Throne</a>
</p>
</details>
