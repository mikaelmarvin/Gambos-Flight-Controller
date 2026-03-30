# Board support

## F446 boards (`custom`, `devkit`)

Same folder layout; shared `Drivers/` and `Middlewares/` at the project root.

```
board/
  custom/          # Custom PCB
    Inc/            # CubeMX Core/Inc
    Src/            # CubeMX Core/Src
    ...
  devkit/           # e.g. NUCLEO-F446RE
    Inc/
    Src/
```

- Copy CubeMX `Core/Inc` → `board/<name>/Inc`, `Core/Src` → `board/<name>/Src`.
- **startup_*.s** – Optional; else the project root startup is used (F446).
- **\*.ld** – Optional; else `STM32F446XX_FLASH.ld` at project root.

Build: `cmake --preset custom` or `cmake --preset devkit`.

## Blue Pill (`bluepill`) — STM32F103

Different MCU: self-contained CubeMX tree under `board/bluepill/` (`Core/`, `Drivers/`, `Middlewares/` including FreeRTOS, `ARM_CM3` port). Linker script `board/bluepill/STM32F103XX_FLASH.ld`.

Build from `project/` only: `cmake --preset bluepill` (do not use a second `CMakeLists.txt` only under `board/bluepill`).

After regenerating from CubeMX, update source lists in `cmake/stm32cubemx/stm32f1_bluepill_freertos.cmake` if files were added or removed (keep it in sync with `board/bluepill/cmake/stm32cubemx/CMakeLists.txt` from CubeMX, or delete that duplicate once integrated).

## Application code

Per board under `app/<board>/` — `app_init()` before `osKernelStart`, `app_run()` typically from the default FreeRTOS task.

See `app/README.md`.

## Adding another board

- **Same MCU family as `custom`:** extend `cmake/stm32f4_freertos.cmake` (or split by name), add `cmake/boards/<name>.cmake`, presets, and `app/<name>/`.
- **Different MCU:** add `cmake/boards/<name>.cmake`, a `cmake/stm32cubemx/<variant>.cmake` with HAL/RTOS paths, wire it in `cmake/stm32cubemx/CMakeLists.txt`, and `app/<name>/`.
