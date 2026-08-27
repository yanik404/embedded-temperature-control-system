import json
from pathlib import Path


tasks_path = Path(".vscode/tasks.json")
settings_path = Path(".vscode/settings.json")
pico_script = Path("tools/pico.ps1").read_text(encoding="utf-8")
setup_script = Path("tools/setup_windows.ps1").read_text(encoding="utf-8")
readme = Path("README.md").read_text(encoding="utf-8")

tasks = json.loads(tasks_path.read_text(encoding="utf-8"))
json.loads(settings_path.read_text(encoding="utf-8"))
labels = {task["label"] for task in tasks["tasks"]}

assert {"Laptop einrichten", "Build", "Flash", "Build & Flash"} <= labels
assert "${workspaceFolder}" in tasks_path.read_text(encoding="utf-8")
for content in (tasks_path.read_text(encoding="utf-8"), pico_script, setup_script):
    assert "C:\\Users\\" not in content
    assert "C:/Users/" not in content

assert '.tools/' in Path(".gitignore").read_text(encoding="utf-8")
assert "Pico SDK 2.1.1" in setup_script
assert "RPI-RP2" in pico_script and "picotool.exe" in pico_script
assert "Terminal → Run Task → Laptop einrichten" in readme

print("portable Windows setup and VS Code task checks passed")
