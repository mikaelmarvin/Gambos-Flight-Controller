# CubeMX upstream (pristine)

This directory is the **untouched STM32CubeMX export** for the Gambos PCB.

- Do **not** hand-edit files here.
- After regenerating in CubeMX, replace this tree with the new export.
- Then run:

```bash
./software/project/scripts/sync-cubemx-gambos-pcb.sh
./software/project/scripts/build.sh
```

Customizations live in `../gambos-pcb_patches/` and are applied onto `../gambos-pcb/` by the sync script.
