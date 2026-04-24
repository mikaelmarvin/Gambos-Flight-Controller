# Gambos firmware

STM32 firmware for the Gambos project — built with CMake, developed in a **Dev Container** (Docker) at the **repository root** (see the top-level `README.md`).

## Prerequisites

| Requirement | Notes |
|-------------|--------|
| **Git** | Clone this repository. |
| **Docker** + **Docker Compose** | Used by the dev container. |
| **Cursor** or **VS Code** | With the **Dev Containers** extension (`ms-vscode-remote.remote-containers`). |
| **Host OS** | **Linux** recommended for **USB** (ST-Link, serial) into the container. On macOS/Windows, builds work; USB passthrough needs extra host setup. |

## First-time setup (after clone)

1. **Open the repo in the editor** and choose **“Reopen in Container”** (or **Dev Containers: Reopen in Container**).
2. Wait for the image to build and **post-create** to finish. The container runs `.devcontainer/setup.sh`, which runs **`./software/project/scripts/build.sh`** for **both** `custom` and `devkit` (so **`software/.clangd`** has a database for each tree) and fixes `compile_commands.json` paths for clangd.
3. **Open a new terminal** so the shell prompt (Starship) and `PATH` are correct.

If anything fails, run **“Dev Containers: Rebuild Container”** once.

## Build firmware

**Use the build script** (from the **repository root**):

```bash
./software/project/scripts/build.sh              # default preset: devkit
./software/project/scripts/build.sh custom       # F446 custom PCB
./software/project/scripts/build.sh devkit       # F446 dev kit (e.g. NUCLEO)
```

From this directory (`software/`) you can instead run:

```bash
./project/scripts/build.sh
```

Presets:

| Preset | Board |
|--------|--------|
| `custom` | F446 custom PCB |
| `devkit` | F446 dev kit (e.g. NUCLEO) |

Artifacts: `software/project/build/<preset>/gambos.elf`.

**ARM GCC** (`arm-none-eabi-gcc`) is installed in the container image.

If you configure manually (equivalent to what `build.sh` does), run from **`software/project/`**:

```bash
cd software/project
cmake --preset devkit
cmake --build build/devkit --parallel
```

### Other scripts (run from repo root)

| Script | Purpose |
|--------|---------|
| `./software/project/scripts/build.sh [preset]` | Configure (if needed) + build (default preset: `devkit`) |
| `./software/project/scripts/clean.sh [preset]` | CMake `clean` for that preset’s `build/<preset>/` |
| `./software/project/scripts/pristine.sh [preset\|all]` | Delete `build/<preset>/`, or `all` to remove all of `software/project/build/` |
| `./software/project/scripts/probe.sh [preset]` | OpenOCD: ST-Link + MCU (STM32F4 for `custom`/`devkit`) |
| `./software/project/scripts/flash.sh [preset]` | OpenOCD: program `build/<preset>/gambos.elf` (build first) |

Presets: `custom`, `devkit` (same as CMake).

## Editor / clangd (IntelliSense)

- `compile_commands.json` is generated under `software/project/build/<preset>/` when you configure that preset.
- CMSIS-SVD for register view in Cortex-Debug: **`software/STM32F446.svd`** (referenced from `.vscode/launch.json`).
- **`software/.clangd`** picks the right compilation database per folder (`custom` default, override for `devkit` paths).  
- If you work on **devkit**, run `./software/project/scripts/build.sh devkit` once so the matching `compile_commands.json` exists, then **restart clangd** (command palette: **clangd: Restart language server**).

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
./software/project/scripts/flash.sh custom      # or devkit
```

Equivalent manual `openocd` lines (paths depend on your workspace location):

**F446 (`custom` / `devkit`):** `interface/stlink.cfg` + `target/stm32f4x.cfg` + `program …/software/project/build/<preset>/gambos.elf verify reset exit`

Probe only: `./software/project/scripts/probe.sh [preset]`

## Repo layout (short)

| Path | Purpose |
|------|---------|
| `software/project/CMakeLists.txt` | Top-level CMake; `BOARD` selects the target. |
| `software/project/cmake/boards/` | Per-board CPU flags and linker scripts. |
| `software/project/cmake/stm32cubemx/` | CubeMX / HAL / FreeRTOS wiring. |
| `software/project/board/<name>/` | Board-specific CubeMX output. |
| `software/project/app/<name>/` | Board-specific application (`app_init` / `app_run`). |
| `.devcontainer/` (repo root) | Dev Container definition and setup script. |
| `software/.clangd` | clangd compilation-database routing per board folder. |
| `software/STM32F446.svd` | CMSIS-SVD for STM32F446 (peripherals in the debugger). |

More detail: `software/project/board/README.md`, `software/project/app/README.md`.

## What to commit so the next clone “just works”

Keep these in the repo (already tracked):

- **`.devcontainer/`** — container definition and `setup.sh`
- **`Dockerfile`**, **`docker-compose.yml`** — image and USB settings
- **`software/project/CMakePresets.json`**, **`software/project/cmake/`** — build system
- **`software/.clangd`**, **`software/.clang-format`** — editor / formatter for firmware

Avoid committing **`software/project/build/`** or editor caches — they belong in `.gitignore`.

## Troubleshooting

| Issue | What to try |
|--------|-------------|
| clangd errors in `app/devkit` | `./software/project/scripts/build.sh devkit` (or `cd software/project && cmake --preset devkit`), restart clangd. |
| `compile_commands` paths wrong in container | `docker compose` runs `setup.sh --post-start` to rewrite paths. |
| OpenOCD cannot see ST-Link | Host: `lsusb`; Linux + Docker: recreate container after compose changes; check `privileged` and `/dev/bus/usb` mount. |
| CMake cannot find preset | Prefer `./software/project/scripts/build.sh` from repo root, or run `cmake` from **`software/project/`**. |
