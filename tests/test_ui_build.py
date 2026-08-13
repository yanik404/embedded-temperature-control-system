import subprocess
import sys


result = subprocess.run([sys.executable, "tools/build_ui.py", "--check"], check=False)
assert result.returncode == 0, "generated UI assets are out of date"
print("UI build reproducibility check passed")
