```mermaid
graph TD
    A[c_cpp_properties.json] -->|提供智能感知| B(VS Code编辑器)
    B -->|编写代码| C[tasks.json]
    C -->|生成exe| D[launch.json]
    D -->|调试执行| E(终端输出)
```