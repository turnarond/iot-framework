#include <stdio.h>
#include <SylixOS.h>
#include "mb_rt.h"

void modbus_test_task(void *arg) {
    mb_master_t master = mb_master_create("192.168.1.100", 502);
    if (!master) {
        printf("Failed to create master\n");
        return;
    }

    uint16_t values[10];
    mb_err_t err = mb_read_holding_registers(master, 1, 0, 10, values);
    if (err == MB_OK) {
        printf("Read success: %d %d %d ...\n", values[0], values[1], values[2]);
    } else {
        printf("Error: %s\n", mb_get_error_string(err));
    }

    mb_master_destroy(master);
}