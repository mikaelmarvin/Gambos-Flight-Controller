# Board patches (Gambos PCB)

Unified diffs applied on top of `gambos-pcb_upstream/` to produce `gambos-pcb/`.

**Rule:** Prefer CubeMX for anything it can generate cleanly (GPIO EXTI, NVIC enables, IRQ stubs, clocks, pins). Patches are only for project glue Cube should not own.

| Patch | Purpose |
|-------|---------|
| `001-main-app-hooks.patch` | Boot UART banner, `app_init()` |
| `002-freertos-default-task.patch` | Default task stack + idle behavior |
| `005-tim-pwm-tuning.patch` | TIM2/TIM3 prescaler and period |
| `006-cubeide-cmake-note.patch` | Note that board `CMakeLists.txt` is CubeIDE-only |

Owned by Cube export (not patches):

- `USR_BTN` (PC0) EXTI + `EXTI0` NVIC + `EXTI0_IRQHandler`
- I2C1 EV/ER NVIC + `I2C1_EV/ER_IRQHandler` (needed for HAL I2C DMA)

## After CubeMX regen

1. Update `gambos-pcb_upstream/` with the new Cube export.
2. Confirm button + I2C interrupts are complete in upstream (see checklists below).
3. `./software/project/scripts/sync-cubemx-gambos-pcb.sh`
4. Fix any remaining patch conflicts, then update the failing `.patch` file(s).
5. `./software/project/scripts/build.sh`

### Button checklist (Cube-owned)

- PC0 = `GPIO_EXTI0`, user label `USR_BTN`
- GPIO mode: external interrupt, **Falling** edge if active-low (or Rising/Falling)
- Pull-up if the board has no external pull
- NVIC: enable **EXTI line 0 interrupt** (priority ≥ 5)

Upstream should include `HAL_NVIC_* (EXTI0_IRQn)` in `gpio.c` and `EXTI0_IRQHandler` in `stm32f4xx_it.c`.

App callback stays in `app/.../button_handler.cpp` (`HAL_GPIO_EXTI_Callback`).

### I2C1 checklist (Cube-owned)

- NVIC: enable **I2C1 event** and **I2C1 error** (priority ≥ 5)
- Upstream `i2c.c` MSP init enables those IRQs; `stm32f4xx_it.c` has `I2C1_EV/ER_IRQHandler`

## Capture new customizations

```bash
./software/project/scripts/regenerate-board-patches.sh
./software/project/scripts/regenerate-board-patches.sh 001
./software/project/scripts/regenerate-board-patches.sh 001-main-app-hooks 005
```

Review the git diff before committing. Unchanged patches are left alone.
