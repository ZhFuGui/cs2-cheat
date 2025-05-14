# CS2作弊程序教程

**语言**： [English](README.md) | [中文(简体)](#)

## 概述
本项目通过开发《反恐精英2》（CS2）作弊程序探索C/C++编程。项目包含多种版本（内部和外部），提供源代码和预编译可执行文件，你将掌握核心编程概念，同时探索游戏修改技术，寓学于乐。

**注意**：仅限教育使用。在CS2线上游戏中使用可能违反服务条款，导致封号。

## 仓库结构
- **`docs/`**：配置指南（`setup.md`）、贡献说明（`contributing.md`）。
- **`src/`**：作弊版本的源代码和可执行文件。
  - 示例：`src/external/demo-xray/` 包含：
    - `bin/xray.exe`：预编译可执行文件。
    - `code/`：源代码（`.hpp`、`.cpp`）。
    - `demo-xray.md`：项目指南。
  - 其他项目结构相同。
- **`tools/`**：`cs2-dumper.exe`，用于提取CS2内存偏移。
- **根目录**：`.gitignore`、 `LICENSE`、 `README.md`、 `README_CN.md`。

## 使用方法
1. **克隆仓库**：
   ```bash
   git clone <repository-url>
   ```
2. **运行偏移提取工具**：
   - 在CS2运行时（建议本地环境）启动`tools/cs2-dumper.exe`。
3. **运行作弊程序**：
   - 进入项目`bin/`目录（例如`src/external/demo-xray/bin/`）。
   - 运行可执行文件（例如`xray.exe`）。
   - 查看项目`.md`文件（例如`demo-xray.md`）。
4. **探索源代码**（可选）：
   - 使用Visual Studio修改或编译`code/`中的文件。
   - 配置方法见`docs/setup.md`。

## 贡献
请参阅`docs/contributing.md`，了解如何分享代码或文档。

---

如有问题，请在GitHub提交Issue。