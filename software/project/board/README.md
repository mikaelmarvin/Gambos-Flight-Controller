# Board support

## Gambos PCB (`gambos-pcb`)

CubeMX output lives under **`board/gambos-pcb/`**: **`Core/`** (or `Inc`/`Src`), **`Drivers/`**, **`Middlewares/`**, `gambos-pcb.ioc`, and the **Cube-generated CMake** tree (`CMakeLists.txt`, **`cmake/`**).

### Two CMake entry points (both valid)

| Where you configure | Purpose |
|---------------------|--------|
| **`software/project/`** (this repo) | Builds firmware target **`gambos`**. `scripts/gen-board-sources.sh` reads Cube-generated files under `board/gambos-pcb/cmake/` and writes generated inputs under `build/gambos-pcb/generated/` (`toolchain.cmake`, `cubemx_paths.cmake`). Top `CMakeLists.txt` then consumes those generated files plus `app/gambos-pcb/` and `custom_drivers`. |
| **`board/gambos-pcb/`** as CMake source root | STM32CubeIDE / Cube “generated project” flow: uses **`board/gambos-pcb/CMakeLists.txt`** and **`board/gambos-pcb/cmake/`** as shipped by Cube. |

Do not hand-edit Cube’s **`board/gambos-pcb/CMakeLists.txt`** or **`board/gambos-pcb/cmake/`** for the `gambos` build; they are treated as generated input.

- **startup** — optional in `board/gambos-pcb/`; otherwise `software/project/startup_stm32f446xx.s` is used.
- **Linker script** — `board/gambos-pcb/STM32F446XX_FLASH.ld` (path is parsed from Cube toolchain and emitted into `build/gambos-pcb/generated/toolchain.cmake`).

Build **gambos** from `software/project/`:

```bash
./scripts/build.sh
```

Direct `cmake --preset gambos-pcb` works only after generated inputs already exist under `build/gambos-pcb/generated/`.

HAL/RTOS source list is parsed from Cube-generated `board/gambos-pcb/cmake/stm32cubemx/CMakeLists.txt`.

## Application code

Firmware app code: `app/gambos-pcb/` — see `app/README.md`.
