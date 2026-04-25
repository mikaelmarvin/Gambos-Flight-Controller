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
2. Wait for the image to build and **post-create** to finish. The container runs `.devcontainer/setup.sh`, which runs **`./software/project/scripts/build.sh devkit`** and fixes `compile_commands.json` paths for clangd.
3. **Open a new terminal** so the shell prompt (Starship) and `PATH` are correct.

If anything fails, run **“Dev Containers: Rebuild Container”** once.

## Build firmware

**Use the build script** (from the **repository root**):

```bash
./software/project/scripts/build.sh              # devkit (default)
./software/project/scripts/build.sh devkit     # same
```

From this directory (`software/`) you can instead run:

```bash
./project/scripts/build.sh
```

Artifacts: `software/project/build/devkit/gambos.elf`.

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
| `./software/project/scripts/build.sh [devkit]` | Configure (if needed) + build |
| `./software/project/scripts/clean.sh [devkit]` | CMake `clean` for `build/devkit/` |
| `./software/project/scripts/pristine.sh [devkit\|all]` | Delete `build/devkit/`, or `all` to remove all of `software/project/build/` |
| `./software/project/scripts/probe.sh` | OpenOCD: ST-Link + MCU (STM32F4) |
| `./software/project/scripts/flash.sh` | OpenOCD: program `build/devkit/gambos.elf` (build first) |

## Editor / clangd (IntelliSense)

- `compile_commands.json` is generated under `software/project/build/devkit/` when you configure the devkit preset.
- CMSIS-SVD for register view in Cortex-Debug: **`software/STM32F446.svd`** (referenced from `.vscode/launch.json`).
- **`software/.clangd`** points firmware sources at the devkit compilation database.
- If clangd is stale, run `./software/project/scripts/build.sh` once, then **restart clangd** (command palette: **clangd: Restart language server**).

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
./software/project/scripts/flash.sh
```

Equivalent manual `openocd` lines (paths depend on your workspace location):

**F446 devkit:** `interface/stlink.cfg` + `target/stm32f4x.cfg` + `program …/software/project/build/devkit/gambos.elf verify reset exit`

Probe only: `./software/project/scripts/probe.sh`

## Repo layout (short)

| Path | Purpose |
|------|---------|
| `software/project/CMakeLists.txt` | Top-level CMake |
| `software/project/scripts/gen-board-sources.sh` | Generates build inputs from Cube CMake (`build/<preset>/generated/*.cmake`) |
| `software/project/board/devkit/` | Board-specific CubeMX output |
| `software/project/app/devkit/` | Devkit application |
| `.devcontainer/` (repo root) | Dev Container definition and setup script |
| `software/.clangd` | clangd compilation database routing |
| `software/STM32F446.svd` | CMSIS-SVD for STM32F446 (peripherals in the debugger) |

More detail: `software/project/board/README.md`, `software/project/app/README.md`.

## What to commit so the next clone “just works”

Keep these in the repo (already tracked):

- **`.devcontainer/`** — container definition and `setup.sh`
- **`Dockerfile`**, **`docker-compose.yml`** — image and USB settings
- **`software/project/CMakePresets.json`**, **`software/project/CMakeLists.txt`**, **`software/project/scripts/`** — build system
- **`software/.clangd`**, **`software/.clang-format`** — editor / formatter for firmware

Avoid committing **`software/project/build/`** or editor caches — they belong in `.gitignore`.

## Troubleshooting

| Issue | What to try |
|--------|-------------|
| clangd errors in `app/devkit` | `./software/project/scripts/build.sh` (or `cd software/project && cmake --preset devkit`), restart clangd. |
| `compile_commands` paths wrong in container | `docker compose` runs `setup.sh --post-start` to rewrite paths. |
| OpenOCD cannot see ST-Link | Host: `lsusb`; Linux + Docker: recreate container after compose changes; check `privileged` and `/dev/bus/usb` mount. |
| CMake cannot find preset | Prefer `./software/project/scripts/build.sh` from repo root, or run `cmake` from **`software/project/`**. |
