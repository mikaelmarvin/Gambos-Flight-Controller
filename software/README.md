# Gambos firmware

STM32 firmware for the Gambos project — built with CMake, developed in a **Dev Container** (Docker) at the **repository root** (see the top-level `README.md`).

## Prerequisites


| Requirement                     | Notes                                                                         |
| ---------------------------------| -------------------------------------------------------------------------------|
| **Git**                         | Clone this repository.                                                        |
| **Docker** + **Docker Compose** | Used by the dev container.                                                    |
| **Cursor** or **VS Code**       | With the **Dev Containers** extension (`ms-vscode-remote.remote-containers`). |
| **Host OS**                     | **Linux** recommended for **USB** (ST-Link, serial) into the container.       |


## First-time setup (after clone)

1. **Open the repo in the editor** and choose **“Reopen in Container”** (or **Dev Containers: Reopen in Container**).
2. Wait for the image to build and **post-create** to finish. The container runs `.devcontainer/setup.sh`, which runs `./software/project/scripts/build.sh custom`.
3. **Open a new terminal** so the shell prompt (Starship) and `PATH` are correct.

## Build, flash, probe, clean, and pristine

Run from the **repository root** via `software/project/scripts/`. `build.sh`, `clean.sh`, `flash.sh`, and `probe.sh` require `devkit` or `custom` (no default). `pristine.sh` optionally takes a preset or removes all of `build/`.


| Argument | Hardware      | Debugger                       |
| ----------| ---------------| --------------------------------|
| `devkit` | Nucleo-F446RE | On-board **ST-Link** (OpenOCD) |
| `custom` | Gambos PCB    | **SEGGER J-Link** (`JLinkExe`) |



| Script                            | Role                                              |
| -----------------------------------| ---------------------------------------------------|
| `build.sh <preset>`               | Configure + compile → `build/<preset>/gambos.elf` |
| `clean.sh <preset>`               | CMake `clean` for one preset                      |
| `pristine.sh` / `pristine.sh all` | Delete entire `build/`                            |
| `pristine.sh <preset>`            | Delete only `build/devkit/` or `build/custom/`    |
| `flash.sh <preset>`               | Program the MCU (build matching preset first)     |
| `probe.sh <preset>`               | Verify debugger connection                        |


```bash
./software/project/scripts/build.sh devkit      # or custom
./software/project/scripts/clean.sh devkit
./software/project/scripts/pristine.sh          # or pristine.sh devkit | custom | all
./software/project/scripts/flash.sh devkit
./software/project/scripts/probe.sh custom      # optional before flash
```

## Editor / clangd (IntelliSense)

- `compile_commands.json` is generated under `software/project/build/<board>/` when you build a preset (`custom` or `devkit`). A symlink at `software/project/compile_commands.json` points at the active board (default: `custom`).
- **clangd path mappings** in `.devcontainer/devcontainer.json` and `.vscode/settings.json` translate host paths (`/home/mikael/gambos`) and container paths (`/workspace/gambos`) so IntelliSense works whether you built on the host or in the Dev Container — no file rewriting.
- CMSIS-SVD for register view in Cortex-Debug: `software/STM32F446.svd` (referenced from `.vscode/launch.json`).
- `software/.clangd` points firmware sources at the `board` compilation database.
- If clangd is stale, run `./software/project/scripts/build.sh <board>` once, then **restart clangd** (command palette: **clangd: Restart language server**).

## Debugging

**Run and Debug (F5):** **Debug** or **Attach** for `devkit` (OpenOCD + ST-Link) or `custom` (J-Link) — see `.vscode/launch.json`. Symbols from `build/<preset>/gambos.elf`; build (and flash if needed) that preset first.

**Before F5**

1. In a terminal: `./software/project/scripts/build.sh devkit` or `build.sh custom` so the ELF exists and matches your code.
2. Optionally `flash.sh` / `probe.sh` with the same preset if you want to verify outside the debugger.

