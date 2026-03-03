# Devkit board (e.g. NUCLEO-F446RE)

This folder holds **STM32CubeMX-generated** init code for your dev board, so you can build and flash the same app on the devkit while your custom PCB is not yet assembled.

## One-time setup

1. **Generate code in STM32CubeMX for your devkit**
   - Select your dev board (e.g. NUCLEO-F446RE with STM32F446).
   - Configure clocks, pins, peripherals (and FreeRTOS if you use it) to match what you need for development.
   - In *Project Manager → Code Generator*, choose "Copy only necessary library files" and keep the same IDE/toolchain layout (e.g. MDK-ARM or "STM32CubeIDE" — we only need the generated `Core/`).
   - Generate the project into a **temporary folder** (e.g. `devkit_cubemx/`).

2. **Copy the generated files here**
   - From the generated project, copy:
     - **`Core/Inc/*`** → **`board/devkit/Inc/`**
     - **`Core/Src/*`** → **`board/devkit/Src/`**
   - If your devkit uses the **same MCU** (STM32F446), you can leave the startup and linker script in the project root; the build will use `project/startup_stm32f446xx.s` and `project/STM32F446XX_FLASH.ld`.  
     If the devkit uses a different MCU, add the correct `startup_*.s` (and optionally linker script) in this folder or adjust the toolchain for the devkit build.

3. **Call your app from devkit `main.c`**
   - In the generated `main.c`, after your `MX_*_Init()` calls, add a call to your application init (e.g. `app_init();`) so the devkit app runs. Use the same pattern as in `board/custom/Src/main.c` (include `app.h` and call `app_init()` before `osKernelStart()` if you use FreeRTOS). Application code is board-specific: this build uses `app/devkit/`, not `app/custom/`.

## Build for devkit

From the **project** directory:

```bash
# Configure for devkit (once)
cmake --preset devkit

# Build
cmake --build build/devkit -j
```

From the **repo root**:

```bash
cd project && cmake --preset devkit   # once
cmake --build project/build/devkit -j
```

## Build for custom PCB (default)

```bash
cmake --preset custom
cmake --build build/custom -j
```

Both builds live side by side: `build/custom/` (custom) and `build/devkit/` (devkit).
