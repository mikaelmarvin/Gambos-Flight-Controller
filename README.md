# gambos-sw
Software part of the Gambos project.

## Build, flash & debug

**From the IDE**
- **Build:** `Ctrl+Shift+B` or **Terminal → Run Task → Build**
- **Clean:** **Terminal → Run Task → Clean**
- **Flash:** **Terminal → Run Task → Flash** (requires ST-Link connected)
- **Build and Flash:** **Terminal → Run Task → Build and Flash**
- **Debug:** **Run and Debug** (F5) → choose **Debug (OpenOCD)**. Builds first, then attaches to the target.

**From the terminal** (from repo root):
```bash
./project/scripts/build.sh    # build
./project/scripts/clean.sh    # clean
./project/scripts/flash.sh    # flash (build first if needed)
```

Uses ST-Link and OpenOCD; change `interface/stlink.cfg` in `.vscode/launch.json` or in the scripts if you use another probe (e.g. CMSIS-DAP).