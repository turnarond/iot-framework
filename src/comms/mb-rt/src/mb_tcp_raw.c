#include "../include/mb_rt.h"
#include "mb_config.h"
#include <lwip/tcp.h>
#include <lwip/ip_addr.h>
#include <api/sys_check_timeouts.h>
#include <arch/sys_arch.h>  // for sys_now()

#if MB_ENABLE_GPIO_PROFILING
#include <driver/gpio.h>
static inline void mb_gpio_high(void) { gpio_write(MB_GPIO_PIN, 1); }
static inline void mb_gpio_low(void)  { gpio_write(MB_GPIO_PIN, 0); }
#else
#define mb_gpio_high()
#define mb_gpio_low()
#endif

// 内部主站结构
struct mb_master_s {
    ip_addr_t remote_ip;
    uint16_t  remote_port;
    struct tcp_pcb *pcb;
    uint8_t   tx_buf[MB_BUF_SIZE];
    uint8_t   rx_buf[MB_BUF_SIZE];
    size_t    rx_len;
    int       response_ready;
    uint32_t  timeout_us;
};

// TCP 接收回调
static err_t mb_tcp_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    mb_master_t master = (mb_master_t)arg;
    if (p == NULL) {
        master->response_ready = -1; // 连接关闭
        return ERR_OK;
    }

    if (master->rx_len + p->len <= MB_BUF_SIZE) {
        memcpy(master->rx_buf + master->rx_len, p->payload, p->len);
        master->rx_len += p->len;
        master->response_ready = 1;
    }
    pbuf_free(p);
    return ERR_OK;
}

// TCP 连接回调
static err_t mb_tcp_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err) {
    return ERR_OK;
}

// 创建主站
mb_master_t mb_master_create(const char *ip_str, uint16_t port) {
    mb_master_t master = (mb_master_t)malloc(sizeof(struct mb_master_s));
    if (!master) return NULL;

    memset(master, 0, sizeof(*master));
    ipaddr_aton(ip_str, &master->remote_ip);
    master->remote_port = port;
    master->timeout_us = 100000; // 默认 100ms

    master->pcb = tcp_new();
    if (!master->pcb) {
        free(master);
        return NULL;
    }

    tcp_arg(master->pcb, master);
    tcp_recv(master->pcb, mb_tcp_recv_cb);
    tcp_connect(master->pcb, &master->remote_ip, master->remote_port, mb_tcp_connected_cb);

    // 等待连接建立（简化版，实际可加超时）
    while (master->pcb->state != ESTABLISHED) {
        sys_check_timeouts();
    }

    return master;
}

void mb_master_destroy(mb_master_t master) {
    if (master) {
        if (master->pcb) {
            tcp_close(master->pcb);
        }
        free(master);
    }
}

mb_err_t mb_read_holding_registers(
    mb_master_t master,
    uint8_t unit_id,
    uint16_t start_addr,
    uint16_t num_regs,
    uint16_t *out_values
) {
    if (!master || !out_values || num_regs == 0 || num_regs > MB_MAX_REGS) {
        return MB_ERR_ILLEGAL_DATA;
    }

    // 1. 构造 MBAP + PDU
    uint8_t *buf = master->tx_buf;
    uint16_t trans_id = 1; // 简化：固定事务 ID
    buf[0] = (trans_id >> 8) & 0xFF;
    buf[1] = trans_id & 0xFF;
    buf[2] = 0; // Protocol ID high
    buf[3] = 0; // Protocol ID low
    // 长度字段稍后填
    buf[6] = unit_id;
    mb_build_read_holding_req(&buf[7], start_addr, num_regs);
    uint16_t pdu_len = 5;
    buf[4] = (pdu_len + 1) >> 8; // length high
    buf[5] = (pdu_len + 1) & 0xFF; // length low

    // 2. 清空接收状态
    master->rx_len = 0;
    master->response_ready = 0;

    // 3. 【关键】开始打桩 + 发送
    mb_gpio_high();
    tcp_write(master->pcb, buf, 7 + pdu_len, TCP_WRITE_FLAG_COPY);
    tcp_output(master->pcb);

    // 4. 轮询等待响应（关键路径：禁用任务切换）
    uint32_t start = sys_now(); // lwIP 时间戳（ms 级，仅作 fallback）
    while (!master->response_ready) {
        sys_check_timeouts(); // 处理 lwIP 定时器

        // 【可选】加入极短 delay 避免总线饥饿
        // sys_msleep(0); // 或 NOP 循环

        // 超时保护（微秒级需高精度 timer，此处简化）
        if ((sys_now() - start) > (master->timeout_us / 1000)) {
            mb_gpio_low();
            return MB_ERR_TIMEOUT;
        }
    }

    mb_gpio_low();

    if (master->response_ready < 0) {
        return MB_ERR_CONNECT;
    }

    // 5. 解析响应（跳过 MBAP 头 6 字节）
    if (master->rx_len < 9) return MB_ERR_ILLEGAL_DATA;
    return mb_parse_read_holding_resp(&master->rx_buf[7], master->rx_len - 7, num_regs, out_values);
}

void mb_set_timeout_us(mb_master_t master, uint32_t timeout_us) {
    if (master) master->timeout_us = timeout_us;
}