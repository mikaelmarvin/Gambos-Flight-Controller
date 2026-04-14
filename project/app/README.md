# Application code (board-specific)

Application code is **not shared** between boards. Each board has its own app so you can run production logic on the custom PCB and test/validation code on the devkit.

| Folder       | Used when   | Purpose                    |
|-------------|-------------|----------------------------|
| `app/custom/` | `BOARD=custom` | Your product application (custom PCB) |
| `app/devkit/` | `BOARD=devkit` | Devkit test code, validation, demos   |

All use `app_init()` (before the RTOS scheduler starts) and `app_run()` (e.g. from the default task). Add more `.c` files as needed and list them in the top-level `CMakeLists.txt` under `target_sources` for `gambos`.
