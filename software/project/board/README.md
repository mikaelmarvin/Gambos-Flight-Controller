# Board support

## F446 devkit

CubeMX output lives under **`board/devkit/`**: **`Core/`** (or `Inc`/`Src`), **`Drivers/`**, **`Middlewares/`**, `.ioc`, and the **Cube-generated CMake** tree (`CMakeLists.txt`, **`cmake/`**).

### Two CMake entry points (both valid)

| Where you configure | Purpose |
|---------------------|--------|
| **`software/project/`** (this repo) | Builds firmware target **`gambos`**. `scripts/gen-board-sources.sh` reads Cube-generated files under `board/devkit/cmake/` and writes generated inputs under `build/devkit/generated/` (`toolchain.cmake`, `cubemx_paths.cmake`). Top `CMakeLists.txt` then consumes those generated files plus `app/${BOARD}/` and `custom_drivers`. |
| **`board/devkit/`** as CMake source root | STM32CubeIDE / Cube “generated project” flow: uses **`board/devkit/CMakeLists.txt`** and **`board/devkit/cmake/`** as shipped by Cube. |

Do not hand-edit Cube’s **`board/devkit/CMakeLists.txt`** or **`board/devkit/cmake/`** for the `gambos` build; they are treated as generated input.

- **startup** — optional in `board/devkit/`; otherwise `software/project/startup_stm32f446xx.s` is used.
- **Linker script** — `board/devkit/STM32F446XX_FLASH.ld` (path is parsed from Cube toolchain and emitted into `build/devkit/generated/toolchain.cmake`).

Build **gambos** from `software/project/`:

```bash
./scripts/build.sh devkit
```

Direct `cmake --preset devkit` works only after generated inputs already exist under `build/devkit/generated/`.

HAL/RTOS source list is parsed from Cube-generated `board/devkit/cmake/stm32cubemx/CMakeLists.txt`.

## Application code

Firmware app code: `app/devkit/` — see `app/README.md`.
