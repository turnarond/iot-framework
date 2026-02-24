#ifndef __MB_RT_H
#define __MB_RT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 错误码 */
typedef enum {
    MB_OK = 0,
    MB_ERR_TIMEOUT,
    MB_ERR_CRC,
    MB_ERR_ILLEGAL_DATA,
    MB_ERR_CONNECT,
    MB_ERR_INTERNAL
} mb_err_t;

/* Modbus 主站句柄（不透明指针） */
typedef struct mb_master_s *mb_master_t;

/* 创建主站实例 */
mb_master_t mb_master_create(const char *ip_addr, uint16_t port);

/* 销毁主站 */
void mb_master_destroy(mb_master_t master);

/* 同步读保持寄存器（目标：≤200μs 端到端） */
mb_err_t mb_read_holding_registers(
    mb_master_t master,
    uint8_t unit_id,
    uint16_t start_addr,
    uint16_t num_regs,
    uint16_t *out_values  // 输出数组，需预分配
);

/* 设置通信超时（单位：微秒，仅用于非关键路径 fallback） */
void mb_set_timeout_us(mb_master_t master, uint32_t timeout_us);

/* 获取最后错误信息 */
const char* mb_get_error_string(mb_err_t err);

#ifdef __cplusplus
}
#endif

#endif /* __MB_RT_H */