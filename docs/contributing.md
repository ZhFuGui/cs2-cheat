# 贡献指南

欢迎为CS2作弊程序教程项目贡献代码或文档！本项目提供了一种实用的方式，通过开发《反恐精英2》（CS2）作弊程序探索C/C++编程。你的贡献一定可以帮助其他读者。


## 1. 贡献流程

1. **Fork仓库**：
   - 在GitHub上Fork本仓库到你的账户。
2. **克隆并创建分支**：
   ```bash
   git clone https://github.com/ZhFuGui/cs2-cheat
   cd cs2-cheat
   git checkout -b feature/your-feature-name        #此处需要修改为自命的分支名
   ```
3. **修改代码或文档**：
   - 添加或修改`src/`中的代码（例如`src/external/demo-xray/code/`）。
   - 更新对应的`.md`文件（例如`src/external/demo-xray/demo-xray.md`）。
4. **测试更改**：
   - 确保代码可编译（参考`docs/setup.md`）。
   - 验证可执行文件在Windows 10/11上运行正常。
5. **提交更改**：
   ```bash
   git add .
   git commit -m "添加功能：简述你的更改"
   git push origin feature/your-feature-name        #你的分支
   ```
6. **创建拉取请求**：
   - 在GitHub提交PR，描述更改内容。
   - 回应审稿人反馈。

## 2. 编码规范

- **语言**：使用C++14及以上，代码简洁易读。
- **命名**：
  - 变量和函数：`snake_case`（例如`read_memory`）。
  - 类和结构体：`PascalCase`（例如`MemoryReader`）。
- **缩进**：4个空格，无制表符。
- **注释**：为关键函数和逻辑添加清晰注释（中文或英文）。
- **文件结构**：头文件（`.hpp`）和源文件（`.cpp`）存放于`code/`。

### 代码示例

以下是一个简单的内存读取函数，展示编码风格：

```cpp
// code/memory_utils.hpp
#pragma once
#include <windows.h>

// 读取指定地址的内存值
template <typename T>
bool read_memory(HANDLE process, uintptr_t address, T& value) {
    return ReadProcessMemory(process, reinterpret_cast<LPCVOID>(address),
                            &value, sizeof(T), nullptr) != 0;
}
```

```cpp
// code/memory_utils.cpp
#include "memory_utils.hpp"
#include <stdexcept>

// 示例：读取玩家坐标
float get_player_position(HANDLE game_process, uintptr_t pos_address) {
    float position = 0.0f;
    if (!read_memory(game_process, pos_address, position)) {
        throw std::runtime_error("无法读取玩家坐标");
    }
    return position;
}
```

- **说明**：
  - 头文件用于定义，类型安全。
  - 源文件实现具体功能，包含错误处理。
  - 命名遵循`snake_case`，注释清晰。

## 3. 提交规范

- **提交信息**：简明扼要，例如：
  ```
  添加外部XRAY功能：实现玩家坐标读取
  ```
- **文档更新**：修改`src/external/demo-xray/code/`时，同步更新`demo-xray.md`。
- **测试**：确保代码在Windows上可编译，功能与项目`.md`描述一致。

## 4. 注意事项

- **代码质量**：避免硬编码偏移，优先使用`cs2-dumper.exe`生成的偏移。
- **沟通**：在Issue或PR中说明贡献目的，保持清晰交流。

---

感谢你的贡献！如有疑问，请在GitHub提交Issue。