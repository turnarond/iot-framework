#include "../include/mb_rt.h"
#include "mb_config.h"
#include <string.h>

// CRC16 计算（Modbus RTU 用，TCP 可省略，但保留兼容性）
static uint16_t mb_crc16(const uint8_t *data, size_t len) {
    // 标准 Modbus CRC16 实现（略）
    return 0;
}

// 构造读保持寄存器请求 PDU
static void mb_build_read_holding_req(uint8_t *pdu, uint16_t start, uint16_t count) {
    pdu[0] = 0x03;               // 功能码
    pdu[1] = (start >> 8) & 0xFF;
    pdu[2] = start & 0xFF;
    pdu[3] = (count >> 8) & 0xFF;
    pdu[4] = count & 0xFF;
}

// 解析响应 PDU
static mb_err_t mb_parse_read_holding_resp(
    const uint8_t *pdu, size_t pdu_len,
    uint16_t expected_count,
    uint16_t *out_values
) {
    if (pdu_len < 3) return MB_ERR_ILLEGAL_DATA;
    if (pdu[0] != 0x03) return MB_ERR_ILLEGAL_DATA;

    uint8_t byte_count = pdu[1];
    if (byte_count != 2 * expected_count) return MB_ERR_ILLEGAL_DATA;

    for (int i = 0; i < expected_count; i++) {
        out_values[i] = (pdu[2 + 2*i] << 8) | pdu[2 + 2*i + 1];
    }
    return MB_OK;
}