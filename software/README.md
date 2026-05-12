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

**`build.sh`** requires **`devkit`** or **`custom`** as its argument (no default).

**Use the build script** (from the **repository root**):

```bash
./software/project/scripts/build.sh devkit
./software/project/scripts/build.sh custom
```

From this directory (`software/`) you can instead run:

```bash
./project/scripts/build.sh devkit    # or custom
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

Full table (debugger × board): **`software/project/scripts/README.md`**.

| Script | Purpose |
|--------|---------|
| `./software/project/scripts/build.sh devkit\|custom` | Configure (if needed) + build (**argument required**) |
| `./software/project/scripts/clean.sh devkit\|custom` | CMake `clean` for `build/<preset>/` (**argument required**) |
| `./software/project/scripts/pristine.sh [devkit\|custom\|all]` | With **no argument** or **`all`**: delete all of `software/project/build/`. With **`devkit`** or **`custom`**: delete only that preset folder. |
| `./software/project/scripts/flash.sh devkit\|custom` | Program firmware (**required:** `devkit` = ST-Link, `custom` = J-Link) |
| `./software/project/scripts/probe.sh devkit\|custom` | Probe SWD (**required:** same arguments as `flash.sh`) |

## Editor / clangd (IntelliSense)

- `compile_commands.json` is generated under `software/project/build/devkit/` when you configure the devkit preset.
- CMSIS-SVD for register view in Cortex-Debug: **`software/STM32F446.svd`** (referenced from `.vscode/launch.json`).
- **`software/.clangd`** points firmware sources at the devkit compilation database.
- If clangd is stale, run `./software/project/scripts/build.sh devkit` once (or `custom` if you use that preset), then **restart clangd** (command palette: **clangd: Restart language server**).

## Debugging (VS Code / Cortex-Debug)

Build, clean, pristine, flash, and probe are done only via **`software/project/scripts/`** (terminal). **`.vscode/tasks.json`** is intentionally empty so the editor does not duplicate those workflows.

**`.vscode/launch.json`** exists **only** for **Cortex-Debug**: Run and Debug (**F5**). Install **Cortex-Debug** (`marus25.cortex-debug`) — it is listed in `.devcontainer/devcontainer.json`.

| Configuration | Preset | Probe | Purpose |
|---------------|--------|-------|---------|
| **Debug devkit (ST-Link / OpenOCD)** | devkit | Nucleo ST-Link | Starts OpenOCD + GDB; loads symbols from `build/devkit/gambos.elf`; stops at `main`. |
| **Attach devkit (ST-Link / OpenOCD)** | devkit | ST-Link | Attaches to the running MCU (build/flash in terminal first if firmware changed). |
| **Debug custom (J-Link)** | custom | SEGGER J-Link | SEGGER GDB Server + GDB; symbols from `build/custom/gambos.elf`. |
| **Attach custom (J-Link)** | custom | J-Link | Attach only. |

**Before F5**

1. In a terminal: **`./software/project/scripts/build.sh devkit`** or **`build.sh custom`** so the ELF exists and matches your code.
2. Optionally **`flash.sh`** / **`probe.sh`** with the same preset if you want to verify outside the debugger.
3. USB probe visible in the container (`lsusb`; compose mounts `/dev/bus/usb`).
4. For **J-Link**, the container needs SEGGER tools (**`JLinkGDBServerCLExe`** for debug). Close anything else using the probe (**`JLinkExe`**, another debug session).

If Cortex-Debug reports **`jlink: GDB Server Quit Unexpectedly`**, open the **Terminal** panel and select the **gdb-server** (or SEGGER) terminal — the real reason is printed there. When the log shows **J-Link is connected** but **Could not connect to target**, the probe works over USB; fix **target power**, **SWD wiring** (SWDIO, SWCLK, GND), and **reset**/**BOOT** before changing IDE settings. If wiring is good but the link is flaky, add J-Link **`serverArgs`** in **`.vscode/launch.json`** (see [SEGGER command-line options](https://wiki.segger.com/J-Link_GDB_Server)), for example a slower clock: `"serverArgs": ["-speed", "1000"]`.

**Symbols / peripherals**

- **`software/STM32F446.svd`** — peripheral registers in Cortex-Debug (paths are set in `launch.json`).

## ST-Link / USB (Linux host)

The compose file passes **`/dev/bus/usb`** and runs the service **`privileged: true`** so OpenOCD or SEGGER tools can use USB debug probes from inside the container.

On **Ubuntu**, add your user to **`dialout`** on the host if you use USB serial adapters (log out and back in):

```bash
sudo usermod -aG dialout $USER
```

After changing `docker-compose.yml`, recreate the stack:

```bash
docker compose down && docker compose up -d --build
```

Then **reopen the Dev Container**.

## Flash / probe (from the container)

Use **`software/project/scripts/README.md`** for the full matrix. Short version:

**`flash.sh`** and **`probe.sh`** require **`devkit`** or **`custom`** as the first argument (no default).

| Argument | Hardware | Debugger |
|----------|----------|----------|
| **`devkit`** | Nucleo-F446RE | On-board **ST-Link** (OpenOCD) |
| **`custom`** | Gambos PCB | **SEGGER J-Link** (`JLinkExe`, OpenOCD fallback) |

Build the matching preset first (`build.sh devkit` or `build.sh custom`).

### Devkit (OpenOCD + ST-Link)

After `./software/project/scripts/build.sh devkit`:

```bash
./software/project/scripts/probe.sh devkit    # optional
./software/project/scripts/flash.sh devkit
```

### Custom board (J-Link)

After `./software/project/scripts/build.sh custom`:

```bash
./software/project/scripts/probe.sh custom    # optional
./software/project/scripts/flash.sh custom
```

Optional environment variables: `GAMBOS_JLINK_DEVICE`, `GAMBOS_JLINK_SPEED`, `GAMBOS_FLASH_ELF`.

If `JLinkExe` is missing, **`flash.sh custom`** / **`probe.sh custom`** fall back to OpenOCD with `software/project/openocd/interface-jlink-swd.cfg`.

**Rebuild the Dev Container** after Dockerfile changes so SEGGER installs.

## Repo layout (short)

| Path | Purpose |
|------|---------|
| `software/project/CMakeLists.txt` | Top-level CMake |
| `software/project/scripts/gen-board-sources.sh` | Generates build inputs from Cube CMake (`build/<preset>/generated/*.cmake`) |
| `software/project/board/devkit/` | Board-specific CubeMX output |
| `software/project/app/devkit/` | Devkit application |
| `.vscode/launch.json` | Cortex-Debug only — Run/Debug (**F5**); no build/flash tasks |
| `.vscode/tasks.json` | Empty — build/flash/clean use **`software/project/scripts/`** in the terminal |
| `.devcontainer/` (repo root) | Dev Container definition and setup script |
| `software/.clangd` | clangd compilation database routing |
| `software/STM32F446.svd` | CMSIS-SVD for STM32F446 (peripherals in the debugger) |
| `software/project/scripts/README.md` | Probe/flash matrix: devkit ST-Link vs custom J-Link |
| `software/project/openocd/` | OpenOCD snippets (J-Link + SWD) used when **`flash.sh custom`** uses OpenOCD fallback |

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
| clangd errors in `app/devkit` | `./software/project/scripts/build.sh devkit` (or `cmake --preset devkit` from `software/project/`), restart clangd. |
| `compile_commands` paths wrong in container | `docker compose` runs `setup.sh --post-start` to rewrite paths. |
| OpenOCD cannot see ST-Link | Host: `lsusb`; Linux + Docker: recreate container after compose changes; check `privileged` and `/dev/bus/usb` mount. |
| J-Link / `flash.sh custom` fails after pulling updates | **Rebuild Container** so SEGGER installs from the Dockerfile; close other tools using J-Link (`JLinkExe`, Ozone). Only one client may attach to the probe at a time. |
| CMake cannot find preset | Run `./software/project/scripts/build.sh devkit` or `build.sh custom` from repo root, or run `cmake` from **`software/project/`**. |
