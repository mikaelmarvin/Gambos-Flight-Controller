# `software/project/scripts`

Run these from the **repository root**.

## Probe & flash (Gambos PCB / J-Link)

| Command | Hardware | Probe | Flash |
|---------|----------|-------|-------|
| `./software/project/scripts/probe.sh` | Gambos PCB — **SEGGER J-Link** | `./software/project/scripts/probe.sh` | `./software/project/scripts/flash.sh` |

Build first:

```bash
./software/project/scripts/build.sh
```

Optional env:

- **`GAMBOS_FLASH_ELF`** — explicit ELF for `flash.sh`
- **`GAMBOS_JLINK_DEVICE`**, **`GAMBOS_JLINK_SPEED`** — J-Link tuning

## Other scripts

| Script | Role |
|--------|------|
| `build.sh` | Configure + compile → `build/gambos-pcb/gambos.elf` |
| `clean.sh` | CMake `clean` for `gambos-pcb` |
| `pristine.sh` / `pristine.sh all` | Delete entire `build/` |
| `pristine.sh gambos-pcb` | Delete only `build/gambos-pcb/` |
| `sync-cubemx-gambos-pcb.sh` | Upstream + patches → `board/gambos-pcb/` |
| `regenerate-board-patches.sh` | Recreate patches after intentional board edits |

USB debug probes need **`/dev/bus/usb`** (see repo `docker-compose.yml`). Install SEGGER in the image (repo `Dockerfile`) and rebuild the Dev Container.
