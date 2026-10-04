# Board support

## Gambos PCB (`gambos-pcb`)

`board/gambos-pcb/` is the **committed STM32CubeMX export** used by CMake.

| Path | Role |
|------|------|
| `board/gambos-pcb/` | CubeMX project (`.ioc`, `Core/`, HAL, FreeRTOS, generated CMake) |
| `app/gambos-pcb/` | Application code |
| `custom_drivers/` | Hardware drivers |

### CubeMX regeneration

1. Open `board/gambos-pcb/gambos-pcb.ioc` in CubeMX.
2. Generate code into the same `board/gambos-pcb/` directory (overwrite).
3. Review `git diff` — keep project hooks inside `USER CODE` blocks.
4. Build:

```bash
./software/project/scripts/build.sh
```

Put pins, clocks, timers, NVIC, and FreeRTOS parameters in the `.ioc`. Put application glue only in Cube `USER CODE` sections (for example `app_init()` in `Core/Src/main.c`).

`board/gambos-pcb/CMakeLists.txt` is **CubeIDE-only**. The firmware build uses `board/gambos-pcb/cmake/stm32cubemx/` via the top-level `CMakeLists.txt`.
