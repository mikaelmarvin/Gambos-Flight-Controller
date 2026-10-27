# Gambos firmware

STM32 firmware for the Gambos project — built with CMake, developed in a **Dev Container** (Docker) at the **repository root** (see the top-level `README.md`).

## Prerequisites


| Requirement                     | Notes                                                                         |
| ---------------------------------| -------------------------------------------------------------------------------|
| **Git**                         | Clone this repository.                                                        |
| **Docker** + **Docker Compose** | Used by the dev container.                                                    |
| **Cursor** or **VS Code**       | With the **Dev Containers** extension (`ms-vscode-remote.remote-containers`). |
| **Host OS**                     | **Linux** recommended for **USB** (J-Link, serial) into the container.       |


## First-time setup (after clone)

1. **Open the repo in the editor** and choose **“Reopen in Container”** (or **Dev Containers: Reopen in Container**).
2. Wait for the image to build and **post-create** to finish. The container runs `.devcontainer/setup.sh`, which runs `./project/scripts/build.sh`.
3. **Open a new terminal** so the shell prompt (Starship) and `PATH` are correct.

## Build, flash, probe, clean, and pristine

Run from the **repository root** via `project/scripts/`.


| Argument | Hardware   | Debugger            |
| -------- | ---------- | ------------------- |
| `gambos-pcb` (default) | Gambos PCB | **SEGGER J-Link** (`JLinkExe`) |



| Script                            | Role                                              |
| -----------------------------------| ---------------------------------------------------|
| `build.sh`                        | Configure + compile → `build/gambos-pcb/gambos.elf` |
| `clean.sh`                        | CMake `clean`                                     |
| `pristine.sh` / `pristine.sh all` | Delete entire `build/`                            |
| `pristine.sh gambos-pcb`          | Delete only `build/gambos-pcb/`                   |
| `flash.sh`                        | Program the MCU (build first)                     |
| `probe.sh`                        | Verify debugger connection                        |


```bash
./project/scripts/build.sh
./project/scripts/clean.sh
./project/scripts/pristine.sh          # or pristine.sh gambos-pcb | all
./project/scripts/flash.sh
./project/scripts/probe.sh             # optional before flash
```

## Editor / clangd (IntelliSense)

- `compile_commands.json` is generated under `project/build/gambos-pcb/` when you build. A symlink at `project/compile_commands.json` points at that directory.
- **clangd path mappings** in `.devcontainer/devcontainer.json` and `.vscode/settings.json` translate host paths (`/home/mikael/gambos`) and container paths (`/workspace/gambos`) so IntelliSense works whether you built on the host or in the Dev Container — no file rewriting.
- CMSIS-SVD for register view in Cortex-Debug: `STM32F446.svd` at the repo root (referenced from `.vscode/launch.json`).
- `.clangd` (repo root) and `project/.clangd` point clangd at `project/compile_commands.json`.
- If clangd is stale, run `./project/scripts/build.sh`, then **restart clangd** (command palette: **clangd: Restart language server**).

## Debugging

**Run and Debug (F5):** **Debug** or **Attach gambos-pcb (J-Link)** — see `.vscode/launch.json`. Symbols from `build/gambos-pcb/gambos.elf`; build (and flash if needed) first.

**Before F5**

1. In a terminal: `./project/scripts/build.sh` so the ELF exists and matches your code.
2. Optionally `flash.sh` / `probe.sh` if you want to verify outside the debugger.
