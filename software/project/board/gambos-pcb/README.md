# Gambos PCB — CubeMX board tree

Committed STM32CubeMX export for the Gambos PCB. CMake builds this directory directly.

- Edit hardware config in `gambos-pcb.ioc`, then regenerate into this folder.
- Keep project-specific C hooks inside `USER CODE` blocks only.
- Application code lives in `app/gambos-pcb/`; drivers in `custom_drivers/`.

See [`../README.md`](../README.md) for the regeneration workflow.
