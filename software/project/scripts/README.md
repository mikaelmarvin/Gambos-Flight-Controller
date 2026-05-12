# `software/project/scripts`

Run these from the **repository root**.

**`build.sh`** and **`clean.sh`** require **`devkit`** or **`custom`** — no default (same rule as **`flash.sh`** / **`probe.sh`**).

## Probe & flash (explicit board required)

You **must** pass **`devkit`** or **`custom`** — there is no default.

| Board argument | Hardware | Probe | Flash |
|----------------|----------|-------|-------|
| **`devkit`** | STM32 Nucleo-F446RE — **ST-Link** | `./software/project/scripts/probe.sh devkit` | `./software/project/scripts/flash.sh devkit` |
| **`custom`** | Gambos PCB — **SEGGER J-Link** | `./software/project/scripts/probe.sh custom` | `./software/project/scripts/flash.sh custom` |

Build first:

```bash
./software/project/scripts/build.sh devkit    # or build.sh custom
```

Optional env:

- **`GAMBOS_FLASH_ELF`** — explicit ELF for `flash.sh` (adapter still follows `devkit` vs `custom`).
- **`GAMBOS_JLINK_DEVICE`**, **`GAMBOS_JLINK_SPEED`** — J-Link only (`custom`).

## Other scripts

| Script | Role |
|--------|------|
| `build.sh devkit` / `build.sh custom` | Configure + compile (**argument required**) → `build/<preset>/gambos.elf` |
| `clean.sh devkit` / `clean.sh custom` | CMake `clean` (**argument required**) |
| `pristine.sh` / `pristine.sh all` | Delete entire `build/` |
| `pristine.sh devkit` / `pristine.sh custom` | Delete only `build/devkit/` or `build/custom/` |
| `gen-board-sources.sh` | Called by `build.sh` |
| `fix-compile-commands-for-container.sh` | Dev Container path fix for `compile_commands.json` |

USB debug probes need **`/dev/bus/usb`** (see repo `docker-compose.yml`). For **`custom`**, install SEGGER in the image (repo `Dockerfile`) and rebuild the Dev Container.
