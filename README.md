# gambos

Monorepo for the Gambos project: **hardware** (schematics, PCBs, mechanical) and **software** (STM32 firmware).

| Directory | Contents |
|-----------|----------|
| [`hardware/`](hardware/) | Hardware design files (add your KiCad, STEP, etc. here). |
| [`software/`](software/) | Firmware, CMake project, and [software/README.md](software/README.md) for build and dev container setup. |

Development (Docker, ST-Link, build scripts) is unchanged in spirit; commands now live under `software/project/`. See **software/README.md** for prerequisites, `./software/project/scripts/build.sh`, and the Dev Container workflow.

The dev container `workspaceFolder` is name-agnostic (`/workspace/project/${localWorkspaceFolderBasename}`), so repository renames do not require manual edits.
