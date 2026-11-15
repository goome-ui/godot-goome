<h1 align="center">🛠 Contributing to godot-goome 🛠</h1>

<p align="center">
Small PRs that fix one thing are way easier for me to check, so those get merged a lot faster.<br>
This repo is only the Godot side. If the bug is in goome itself (drawing, the ui kit, a widget acting weird) it goes to <a href="https://github.com/goome-ui/c-goome">c-goome</a>. Docs fixes go to the <a href="https://goome-ui.github.io/docs/">docs</a> and new example projects to <a href="https://github.com/goome-ui/examples">goome-ui/examples</a>.
</p>

<details>
<summary align="center"><b>📋 The short version 📋</b></summary>

<p align="center">
• One thing per PR. One fix or one feature, if it does two things it gets closed<br>
• Open your PR against the <b>dev</b> branch, not main<br>
• Change the C in <code>src/</code>, the GDScript in <code>example/</code> and <code>tests/</code>, never anything in <code>native/</code><br>
• Dont touch the protected files, a bot closes the PR if you do<br>
• Test it and say how you tested it, on which OS and which Godot version<br>
• CI has to pass<br>
• AI is fine, but say you used it, and you need to know what every line does
</p>
</details>

<details>
<summary align="center"><b>💬 Before you start 💬</b></summary>

<p align="center">
Small fixes can go straight to a PR. For a new feature or anything big, <a href="https://github.com/goome-ui/godot-goome/issues">open an issue</a> first so we can talk about it.<br> <br>
Same for new methods, properties or signals on <code>GoomeView</code> and <code>GoomeUI</code>, always an issue first. Once peoples scripts call them they cant really change anymore, so Id rather get the name and shape right before its in.
</p>
</details>

<details>
<summary align="center"><b>🌿 Branches 🌿</b></summary>

<p align="center">
You only ever PR into <b>dev</b>, I move stuff up to beta and main myself once its been around for a bit.
</p>
</details>

<details>
<summary align="center"><b>🚫 What gets closed automatically 🚫</b></summary>

<p align="center">
A bot checks every PR and closes it with a comment if it hits any of these. Dont take it personally, fix it and open a new one.<br> <br>
• PRs that arent against <b>dev</b><br>
• Anything in <code>.github/</code><br>
• Anything in <code>native/</code>. <code>goome.h</code> comes from c-goome and <code>gdextension/gdextension_interface.h</code> comes from Godot, both get copied in by the scripts in <code>tools/</code><br>
• <code>build.sh</code>, <code>tools/</code>, any <code>.gdextension</code> file, <code>addons/goome/plugin.gd</code>, <code>addons/goome/plugin.cfg</code> and anything in <code>addons/goome/bin/</code>. Those run on peoples computers the second they build or open the project in the editor, before anyone even looks at the code. You cant review a binary in a PR either, the libs only come from my CI runs<br>
• <code>LICENSE</code> and the pictures (<code>docs/logo*</code>, <code>docs/screenshot.png</code>)<br>
• Over 3000 changed files<br> <br>
Renaming or moving any of those counts too. Need a build change? <a href="https://github.com/goome-ui/godot-goome/issues">Open an issue</a> and Ill do it.
</p>
</details>

<details>
<summary align="center"><b>📄 native/ 📄</b></summary>

- `native/goome.h` is c-goome's single `goome.h`, copied as is, and `native/VERSION` is the c-goome commit it came from
- updating it means `tools/update_native.sh ../c-goome` (or a path to any committed `goome.h`, like one in a c-goome worktree), then fixing whatever changed in `src/`
- `native/gdextension/gdextension_interface.h` is dumped from a real Godot with `--dump-gdextension-interface`, `native/gdextension/VERSION` says which one
- updating that means `tools/update_gdextension.sh 4.7.2-stable` on linux, it downloads that Godot, dumps the header and grabs Godot's license next to it
- the extension asks for its functions by name when it loads, so a newer header only matters when you want something new from it. Keep `compatibility_minimum` in `addons/goome/goome.gdextension` honest when you do
</details>

<details>
<summary align="center"><b>🤖 Using AI 🤖</b></summary>

<p align="center">
Using AI to help write your PR is allowed, just say in the PR that it helped.<br> <br>
You still have to know what your code does. If I ask about a line you should be able to answer without going back to the AI.<br> <br>
And test it yourself, untested AI dumps get closed.
</p>
</details>

<details>
<summary align="center"><b>🧪 Testing 🧪</b></summary>

<p align="center">
Build it, then run the headless test with any Godot 4.4 or newer:
</p>

```sh
./build.sh
tests/run.sh /path/to/godot

# windows, from git bash with mingw gcc on PATH
./build.sh
tests/run.sh /path/to/Godot_v4.7.2-stable_win64_console.exe

# the example, in the editor
cp -r addons/goome example/addons/
godot -e --path example
```

<p align="center">
<code>tests/run.sh</code> copies the addon into <code>tests/project</code>, imports it and runs <code>test.tscn</code>. It prints one line per check and fails if any of them fail.<br> <br>
The first import of a fresh project can crash on the way out after the extension got loaded for the first time. Thats Godot, the import itself is fine, so <code>run.sh</code> ignores it and runs the real test after.<br> <br>
If you change how input gets in or out, add a check to <code>tests/project/test.gd</code>. The test sends real Godot input events through <code>Input.parse_input_event</code>, so it goes through the same path as a real mouse.<br> <br>
In the PR write a few lines on what you ran, on which OS and with which Godot. Changes you can see need a before and after screenshot.<br> <br>
CI runs on linux, windows and mac when you open the PR. It has to be green before I merge.
</p>
</details>

<details>
<summary align="center"><b>✏️ Code style ✏️</b></summary>

<p align="center">
• Plain C99, no godot-cpp, no C++. It builds with gcc, clang and mingw gcc with no extra link flags, keep it that way<br>
• Everything Godot goes through <code>src/gd.c</code>. Engine methods get called by name with <code>gd_call</code>, so there are no method hashes to break between Godot versions<br>
• New <code>GoomeUI</code> methods go in the table at the bottom of <code>src/ui.c</code>, the arg types and defaults there are what GDScript sees<br>
• Values in, new value out. GDScript cant pass by reference, so widgets give back the new value instead of changing one<br>
• Free what you make. Every <code>gd_var</code> gets a <code>gd.destroy</code>, every <code>gd_as_cstr</code> a <code>free</code><br>
• Match the code around it, same naming and formatting. No comments, the names should say it<br>
• Dont reformat whole files, it makes the diff impossible to read
</p>
</details>

<details>
<summary align="center"><b>💬 Discussions and first issues 💬</b></summary>
<p align="center">
Questions and stuff you made with goome go in <a href="https://github.com/goome-ui/c-goome/discussions">Discussions</a>.<br>
If you want something to work on, the <a href="https://github.com/goome-ui/godot-goome/labels/good%20first%20issue">good first issue</a> ones are small. Comment on one before you start.
</p>
</details>

<details>
<summary align="center"><b>🛡️ Security 🛡️</b></summary>

<p align="center">
Found a security hole? Dont open a public issue or PR for it, read <a href="https://github.com/goome-ui/.github/blob/main/SECURITY.md">SECURITY.md</a> and tell me privately first.
</p>
</details>

<p align="center">
By sending a PR you agree your code goes out under the same <a href="LICENSE">GPL-3.0 license</a> as the rest of godot-goome.
</p>
