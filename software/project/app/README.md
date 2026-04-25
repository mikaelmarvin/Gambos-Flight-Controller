# Application code

The devkit application lives in `app/devkit/`. Sources are listed in `app/devkit/CMakeLists.txt` (included from the top-level `CMakeLists.txt`).

Use `app_init()` (before the RTOS scheduler starts) and `app_run()` (for example from the default FreeRTOS task).
