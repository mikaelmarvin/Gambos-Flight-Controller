# Board support (symmetric layout)

All boards use the **same folder layout**. Each board has its own CubeMX-generated init and optional extras; application code is separate per board under `app/<board>/`.

## Layout (per board)

```
board/
  custom/          # Custom PCB
    Inc/            # CubeMX Core/Inc
    Src/            # CubeMX Core/Src
    README.md       # optional
    startup_*.s     # optional, if different from project root
    *.ld            # optional, if different linker script
  devkit/           # Dev board (e.g. NUCLEO-F446RE)
    Inc/
    Src/
    README.md
    ...
```

- **Inc/, Src/** – Required. STM32CubeMX-generated code: copy from generated `Core/Inc` → `board/<name>/Inc`, `Core/Src` → `board/<name>/Src`.
- **startup_*.s** – Optional. If omitted, the build uses the one in the project root (same MCU).
- **\*.ld** – Optional. Board-specific linker script if needed (otherwise toolchain uses project root).
- **README.md** – Optional. Board notes, pinout, how to regenerate from CubeMX.

## Application code (not shared)

Application code is **board-specific** in `app/`:

- `app/custom/` – Used when `BOARD=custom` (product application).
- `app/devkit/` – Used when `BOARD=devkit` (test/validation code).

See `app/README.md`.

## Build

- **Custom PCB:** `cmake --preset custom` → `build/custom/`
- **Devkit:** `cmake --preset devkit` → `build/devkit/`

## Adding a new board

1. Create `board/<name>/Inc/` and `board/<name>/Src/`.
2. Generate code in STM32CubeMX for that board and copy `Core/Inc` → `board/<name>/Inc`, `Core/Src` → `board/<name>/Src`.
3. Add `app/<name>/` with `app.c` and `app.h` (same `app_init()` / `app_run()` interface).
4. In the top-level `CMakeLists.txt`, extend the `BOARD` check to allow `<name>` and add a preset if you want.
