#ifndef BOOT_FACTORY_PARAMS_H
#define BOOT_FACTORY_PARAMS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BOOT_FACTORY_PARAMS_MAGIC        0x54434146U
#define BOOT_FACTORY_PARAMS_HEADER_WORDS 6U
#define BOOT_FACTORY_PARAMS_SECTOR_SIZE  4096U

struct boot_factory_params {
    uint32_t magic;
    uint32_t header_size;
    uint32_t header_crc;
    uint32_t boot_addr;
    uint32_t payload_addr;
    uint32_t payload_size;
} __attribute__((packed));

uint32_t boot_factory_params_crc32(const void *data, size_t size);
bool boot_factory_params_validate(const struct boot_factory_params *params);
uint32_t boot_factory_app_addr_get(uint32_t default_app_addr);

#endif /* BOOT_FACTORY_PARAMS_H */
