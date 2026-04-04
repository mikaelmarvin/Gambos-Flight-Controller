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

## Application code

Per board under `app/<board>/` — `app_init()` before `osKernelStart`, `app_run()` typically from the default FreeRTOS task.

See `app/README.md`.

## Adding another board

- **Same MCU family as `custom`:** extend `cmake/stm32f4_freertos.cmake` (or split by name), add `cmake/boards/<name>.cmake`, presets, and `app/<name>/`.
- **Different MCU:** add `cmake/boards/<name>.cmake`, a `cmake/stm32cubemx/<variant>.cmake` with HAL/RTOS paths, wire it in `cmake/stm32cubemx/CMakeLists.txt`, and `app/<name>/`.
