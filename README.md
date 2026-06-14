# Desk To Do

A lightweight desktop TODO reminder.

## Features

- **Persistent local history**  
  Your TODO list is automatically saved to local storage and restored on next launch.

- **Customizable theme via config file**  
  Modify `appdata/config.toml` to personalize colors, appearance, and visual style.

> **Note**  
> The feature that synchronizes the title bar color with the window background relies on Windows DWM APIs and is only available on **Windows 11 Insider Preview build 22000 or higher**. On older Windows versions the title bar will use the system default style.

## Build

### Requirements

- Qt6 SDK
- CMake 4.0+
- Compiler with C++20 support

> NOTE: Windows users should specify Qt SDK in `CMakeLists.txt`.

### Compilation

Run following commands to start compiling for executable:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

After a successful build, copy the `assets/appdata` directory into the same folder as the compiled executable.

## License

Licensed under the MIT license, check [LICENSE](LICENSE) for details.