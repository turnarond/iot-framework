# Modbus Client 工具使用文档

## 1. 工具简介

Modbus Client 是一个用于测试 Modbus TCP 服务器的命令行工具，支持读取和写入各种类型的寄存器值，并可以按表格格式列出批量寄存器值。

## 2. 支持的命令

| 命令 | 描述 |
|------|------|
| `read` | 读取寄存器值并以简单列表形式显示 |
| `write` | 写入单个寄存器值 |
| `list` | 按表格格式列出批量寄存器值 |

## 3. 支持的选项

| 选项 | 长选项 | 描述 | 默认值 |
|------|--------|------|--------|
| `-h` | `--host` | Modbus 服务器 IP 地址 | 127.0.0.1 |
| `-p` | `--port` | Modbus 服务器端口 | 502 |
| `-s` | `--slave` | 从站 ID | 1 |
| `-t` | `--type` | 寄存器类型 | 无（必须指定） |
| `-a` | `--address` | 起始地址 | 无（必须指定） |
| `-n` | `--number` | 读取的寄存器数量（用于 read/list 命令） | 无（必须指定） |
| `-v` | `--value` | 写入的值（用于 write 命令） | 无（必须指定） |

## 4. 支持的寄存器类型

| 类型 | 描述 | 支持的操作 |
|------|------|------------|
| `coil` | 线圈（数字输出） | 读取、写入 |
| `discrete` | 离散输入（数字输入） | 读取 |
| `holding` | 保持寄存器（模拟输出） | 读取、写入 |
| `input` | 输入寄存器（模拟输入） | 读取 |

## 5. 使用示例

### 5.1 按表格格式列出批量寄存器值

**功能**：以表格形式按行列出从指定地址开始的多个寄存器值，清晰易读。

**命令格式**：
```bash
modbus_client list -h <host> -p <port> -s <slave> -t <type> -a <address> -n <number>
```

**示例**：
```bash
# 列出从地址 0 开始的 10 个保持寄存器值
modbus_client list -h 127.0.0.1 -p 1502 -s 1 -t holding -a 0 -n 10
```

**输出示例**：
```
Address Value
------- -----
0       1234
1       0
2       0
3       0
4       0
5       0
6       0
7       0
8       0
9       0
```

### 5.2 读取寄存器值

**功能**：读取从指定地址开始的多个寄存器值，并以简单列表形式显示。

**命令格式**：
```bash
modbus_client read -h <host> -p <port> -s <slave> -t <type> -a <address> -n <number>
```

**示例**：
```bash
# 读取从地址 0 开始的 5 个保持寄存器值
modbus_client read -h 127.0.0.1 -p 1502 -s 1 -t holding -a 0 -n 5
```

**输出示例**：
```
Read 5 registers from holding register starting at address 0
Values: 1234, 0, 0, 0, 0
```

### 5.3 写入寄存器值

**功能**：向指定地址的寄存器写入一个值。

**命令格式**：
```bash
modbus_client write -h <host> -p <port> -s <slave> -t <type> -a <address> -v <value>
```

**示例**：
```bash
# 向地址 0 的保持寄存器写入值 1234
modbus_client write -h 127.0.0.1 -p 1502 -s 1 -t holding -a 0 -v 1234
```

**输出示例**：
```
Successfully wrote value 1234 to holding register at address 0
```

## 6. 常见问题

### 6.1 连接失败

**错误信息**：`Error: Failed to connect: Connection refused`

**可能原因**：
- Modbus 服务器未运行
- 服务器 IP 地址或端口错误
- 网络连接问题

**解决方案**：
- 确保 Modbus 服务器正在运行
- 检查服务器 IP 地址和端口是否正确
- 检查网络连接是否正常

### 6.2 寄存器地址错误

**错误信息**：`Error: Exception code 2`

**可能原因**：
- 寄存器地址超出了服务器支持的范围

**解决方案**：
- 检查服务器配置，确认支持的寄存器地址范围
- 使用有效的寄存器地址

### 6.3 写入失败

**错误信息**：`Error: Exception code 3`

**可能原因**：
- 写入的值超出了寄存器支持的范围
- 尝试写入只读寄存器

**解决方案**：
- 确保写入的值在寄存器支持的范围内
- 确认寄存器类型支持写入操作

## 7. 构建和安装

### 7.1 构建

```bash
cd /home/yanchaodong/work/acoinfo/edge-framework/src/tools/modbus_client
mkdir -p build
cd build
cmake ..
make -j4
```

### 7.2 运行

```bash
# 直接运行
./modbus_client <command> [options]

# 或使用完整路径
/home/yanchaodong/work/acoinfo/edge-framework/src/tools/modbus_client/build/modbus_client <command> [options]
```

## 8. 注意事项

1. **寄存器类型**：`discrete` 和 `input` 类型的寄存器只支持读取操作，不支持写入操作。

2. **线圈值**：写入线圈时，值 0 表示关闭，非零值表示打开。

3. **地址范围**：不同的 Modbus 服务器可能支持不同的寄存器地址范围，请根据服务器配置使用有效的地址。

4. **端口设置**：默认端口为 502，这是 Modbus TCP 的标准端口。如果服务器使用了其他端口，请使用 `-p` 选项指定。

5. **从站 ID**：默认从站 ID 为 1。如果服务器配置了不同的从站 ID，请使用 `-s` 选项指定。

## 9. 联系信息

如有任何问题或建议，请联系工具开发者。
