#ifndef __MB_CONFIG_H
#define __MB_CONFIG_H

/* 最大寄存器数量（决定缓冲区大小） */
#define MB_MAX_REGS             125   // Modbus 规范上限

/* 内部缓冲区大小（MBAP + PDU） */
#define MB_BUF_SIZE             (6 + 1 + 2 + 2*MB_MAX_REGS + 2)  // ≈ 260 bytes

/* 是否启用 GPIO 打桩（用于延迟测量） */
#define MB_ENABLE_GPIO_PROFILING 0

/* GPIO 引脚（若启用） */
#define MB_GPIO_PIN             12

#endif /* __MB_CONFIG_H */