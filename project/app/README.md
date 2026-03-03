# Application code (board-specific)

Application code is **not shared** between boards. Each board has its own app so you can run production logic on the custom PCB and test/validation code on the devkit.

| Folder       | Used when   | Purpose                    |
|-------------|-------------|----------------------------|
| `app/custom/` | `BOARD=custom` | Your product application (custom PCB) |
| `app/devkit/` | `BOARD=devkit` | Devkit test code, validation, demos   |

Both provide the same interface (`app_init()`, `app_run()`) so each board’s `main.c` can call them the same way. Add more sources under `app/custom/` or `app/devkit/` as needed; list them in `CMakeLists.txt` under the board-specific `target_sources` section.
