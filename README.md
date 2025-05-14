# CS2 Cheat Program Tutorial

**Language**: [English](#) | [中文(简体)](README_CN.md)

## Overview
This tutorial helps beginners learn C/C++ by building cheat programs for Counter-Strike 2 (CS2). It offers multiple versions (internal and external) with source code and precompiled executables, guiding users to master programming concepts through game modification.

**Note**: For educational use only. Using these in online CS2 games may violate terms of service and lead to bans.

## Repository Structure
- **`docs/`**: Guides for setup (`setup.md`), contributions (`contributing.md`).
- **`src/`**: Source code and executables for cheat versions.
  - Example: `src/external/demo-xray/` includes:
    - `bin/xray.exe`: Precompiled executable.
    - `code/`: Source files (`.hpp`, `.cpp`).
    - `demo-xray.md`: Project guide.
  - Other projects follow the same structure.
- **`tools/`**: `cs2-dumper.exe` for extracting CS2 memory offsets.
- **Root**: `.gitignore`, `LICENSE`, `README.md`, `README_CN.md`.

## Usage
1. **Clone Repository**:
   ```bash
   git clone <repository-url>
   ```
2. **Run Offset Dumper**:
   - Launch `tools/cs2-dumper.exe` with CS2 running (local environment).
3. **Run Cheat Program**:
   - Navigate to a project’s `bin/` (e.g., `src/external/demo-xray/bin/`).
   - Run the executable (e.g., `xray.exe`).
   - Check the project’s `.md` (e.g., `demo-xray.md`).
4. **Explore Source** (Optional):
   - Modify or compile `code/` files using Visual Studio.
   - See `docs/setup.md`.

## Contributing
See `docs/contributing.md` for how to share code or docs.

---

Report issues on GitHub.