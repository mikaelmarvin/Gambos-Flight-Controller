# Board support

## Gambos PCB (`gambos-pcb`)

`board/` is the **committed STM32CubeMX export** used by CMake.

| Path | Role |
|------|------|
| `board/` | CubeMX project (`board.ioc`, `Core/`, HAL, FreeRTOS, generated CMake) |
| `app/` | Application code |
| `custom_drivers/` | Hardware drivers |

### CubeMX regeneration

1. Open `board/board.ioc` in CubeMX.
2. Generate code into the same `board/` directory (overwrite). The `.ioc` must keep the folder's name (`board.ioc` in `board/`), or CubeMX generates into a new sibling folder.
3. Review `git diff` — keep project hooks inside `USER CODE` blocks.
4. Build:

```bash
./project/scripts/build.sh
```

Put pins, clocks, timers, NVIC, and FreeRTOS parameters in the `.ioc`. Put application glue only in Cube `USER CODE` sections (for example `app_init()` in `Core/Src/main.c`).

`board/CMakeLists.txt` is **CubeIDE-only**. The firmware build uses `board/cmake/stm32cubemx/` via the top-level `CMakeLists.txt`.
