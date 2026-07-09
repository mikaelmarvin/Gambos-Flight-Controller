# Board patches (Gambos PCB)

Unified diffs applied on top of `gambos-pcb_upstream/` to produce `gambos-pcb/`.

| Patch | Purpose |
|-------|---------|
| `001-main-app-hooks.patch` | Boot UART banner, `app_init()` |
| `002-freertos-default-task.patch` | Default task stack + idle behavior |
| `003-gpio-button-exti.patch` | User button EXTI + NVIC |
| `004-stm32f4xx-it-exti0.patch` | `EXTI0_IRQHandler` |
| `005-tim-pwm-tuning.patch` | TIM2/TIM3 prescaler and period |

## After CubeMX regen

1. Update `gambos-pcb_upstream/` with the new Cube export.
2. `./software/project/scripts/sync-cubemx-gambos-pcb.sh`
3. Fix any patch conflicts, then update the failing `.patch` file(s).
4. `./software/project/scripts/build.sh`

## Capture new customizations

If you intentionally changed `gambos-pcb/` and want to refresh patches:

```bash
./software/project/scripts/regenerate-board-patches.sh
```

Review the git diff before committing.
