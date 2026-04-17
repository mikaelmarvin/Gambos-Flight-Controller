# gambos-sw

Software part of the Gambos project — STM32 firmware built with CMake, developed in a **Dev Container** (Docker).

## Prerequisites

| Requirement | Notes |
|-------------|--------|
| **Git** | Clone this repository. |
| **Docker** + **Docker Compose** | Used by the dev container. |
| **Cursor** or **VS Code** | With the **Dev Containers** extension (`ms-vscode-remote.remote-containers`). |
| **Host OS** | **Linux** recommended for **USB** (ST-Link, serial) into the container. On macOS/Windows, builds work; USB passthrough needs extra host setup. |

## First-time setup (after clone)

1. **Open the repo in the editor** and choose **“Reopen in Container”** (or **Dev Containers: Reopen in Container**).
2. Wait for the image to build and **post-create** to finish. The container runs `.devcontainer/setup.sh`, which runs **`./project/scripts/build.sh`** for **both** `custom` and `devkit` (so `.clangd` has a database for each tree) and fixes `compile_commands.json` paths for clangd.
3. **Open a new terminal** so the shell prompt (Starship) and `PATH` are correct.

If anything fails, run **“Dev Containers: Rebuild Container”** once.

## Build firmware

**Use the build script** (from the **repository root**):

```bash
./project/scripts/build.sh              # default preset: devkit
./project/scripts/build.sh custom       # F446 custom PCB
./project/scripts/build.sh devkit       # F446 dev kit (e.g. NUCLEO)
```

Presets:

| Preset | Board |
|--------|--------|
| `custom` | F446 custom PCB |
| `devkit` | F446 dev kit (e.g. NUCLEO) |

Artifacts: `project/build/<preset>/gambos.elf`.

**ARM GCC** (`arm-none-eabi-gcc`) is installed in the container image.

If you configure manually (equivalent to what `build.sh` does), run from **`project/`**:

```bash
cd project
cmake --preset devkit
cmake --build build/devkit --parallel
```

### Other scripts (run from repo root)

| Script | Purpose |
|--------|---------|
| `./project/scripts/build.sh [preset]` | Configure (if needed) + build (default preset: `devkit`) |
| `./project/scripts/clean.sh [preset]` | CMake `clean` for that preset’s `build/<preset>/` |
| `./project/scripts/pristine.sh [preset\|all]` | Delete `build/<preset>/`, or `all` to remove all of `project/build/` |
| `./project/scripts/probe.sh [preset]` | OpenOCD: ST-Link + MCU (STM32F4 for `custom`/`devkit`) |
| `./project/scripts/flash.sh [preset]` | OpenOCD: program `build/<preset>/gambos.elf` (build first) |

Presets: `custom`, `devkit` (same as CMake).

## Editor / clangd (IntelliSense)

- `compile_commands.json` is generated under `project/build/<preset>/` when you configure that preset.
- The repo **`.clangd`** file picks the right compilation database per folder (`custom` default, override for `devkit` paths).  
- If you work on **devkit**, run `./project/scripts/build.sh devkit` once so the matching `compile_commands.json` exists, then **restart clangd** (command palette: **clangd: Restart language server**).

## ST-Link / USB (Linux host)

The compose file passes **`/dev/bus/usb`** and runs the service **`privileged: true`** so OpenOCD can use the ST-Link from inside the container.

On **Ubuntu**, add your user to **`dialout`** on the host if you use USB serial adapters (log out and back in):

```bash
sudo usermod -aG dialout $USER
```

After changing `docker-compose.yml`, recreate the stack:

```bash
docker compose down && docker compose up -d --build
```

Then **reopen the Dev Container**.

## Flash with OpenOCD (from the container)

After a successful build, from the **repo root**:

```bash
./project/scripts/flash.sh custom      # or devkit
```

Equivalent manual `openocd` lines (paths depend on your workspace location):

**F446 (`custom` / `devkit`):** `interface/stlink.cfg` + `target/stm32f4x.cfg` + `program …/build/<preset>/gambos.elf verify reset exit`

Probe only: `./project/scripts/probe.sh [preset]`

## Repo layout (short)

| Path | Purpose |
|------|---------|
| `project/CMakeLists.txt` | Top-level CMake; `BOARD` selects the target. |
| `project/cmake/boards/` | Per-board CPU flags and linker scripts. |
| `project/cmake/stm32cubemx/` | CubeMX / HAL / FreeRTOS wiring. |
| `project/board/<name>/` | Board-specific CubeMX output. |
| `project/app/<name>/` | Board-specific application (`app_init` / `app_run`). |
| `.devcontainer/` | Dev Container definition and setup script. |
| `.clangd` | clangd compilation-database routing per board folder. |

More detail: `project/board/README.md`, `project/app/README.md`.

## What to commit so the next clone “just works”

Keep these in the repo (already tracked):

- **`.devcontainer/`** — container definition and `setup.sh`
- **`Dockerfile`**, **`docker-compose.yml`** — image and USB settings
- **`project/CMakePresets.json`**, **`project/cmake/`** — build system
- **`.clangd`** — editor support for multiple boards

Avoid committing **`project/build/`** or editor caches — add them to `.gitignore` if you use local builds outside the container.

## Troubleshooting

| Issue | What to try |
|--------|-------------|
| clangd errors in `app/devkit` | `./project/scripts/build.sh devkit` (or `cd project && cmake --preset devkit`), restart clangd. |
| `compile_commands` paths wrong in container | `docker compose` runs `setup.sh --post-start` to rewrite paths. |
| OpenOCD cannot see ST-Link | Host: `lsusb`; Linux + Docker: recreate container after compose changes; check `privileged` and `/dev/bus/usb` mount. |
| CMake cannot find preset | Prefer `./project/scripts/build.sh` from repo root, or run `cmake` from **`project/`**. |
