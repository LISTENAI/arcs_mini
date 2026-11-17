#include "boot_factory_params.h"

#include <stddef.h>
#include <stdint.h>

#define BOOT_FACTORY_HEADER_OFFSET 0x140U
#define BOOT_FACTORY_LOAD_ADDR_OFFSET 8U

#define __boot_factory_ramcode__ __attribute__((section(".boot_f.ramcode")))

_Static_assert(sizeof(struct boot_factory_params) == 24U,
               "factory parameter header must contain six uint32_t words");

__boot_factory_ramcode__ uint32_t boot_factory_params_crc32(const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFU;

    while (size-- > 0U) {
        crc ^= *bytes++;
        for (unsigned int bit = 0; bit < 8U; ++bit) {
            uint32_t mask = 0U - (crc & 1U);
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }

    return crc ^ 0xFFFFFFFFU;
}

__boot_factory_ramcode__ static uint32_t factory_payload_offset_bytes(
    const struct boot_factory_params *params)
{
    if (params->header_size == BOOT_FACTORY_PARAMS_HEADER_WORDS) {
        return params->payload_addr * sizeof(uint32_t);
    }

    return params->payload_addr;
}

__boot_factory_ramcode__ bool boot_factory_params_validate(
    const struct boot_factory_params *params)
{
    struct boot_factory_params copy = *params;
    uint32_t saved_crc = copy.header_crc;
    uint32_t payload_offset;

    if (copy.magic != BOOT_FACTORY_PARAMS_MAGIC) {
        return false;
    }

    if (copy.header_size != BOOT_FACTORY_PARAMS_HEADER_WORDS &&
        copy.header_size != sizeof(copy)) {
        return false;
    }

    if (copy.boot_addr != CONFIG_BOOT_FACTORY_TEST_OFFSET) {
        return false;
    }

    payload_offset = factory_payload_offset_bytes(&copy);
    if (payload_offset < sizeof(copy) ||
        payload_offset > BOOT_FACTORY_PARAMS_SECTOR_SIZE ||
        copy.payload_size > BOOT_FACTORY_PARAMS_SECTOR_SIZE - payload_offset) {
        return false;
    }

    copy.header_crc = 0U;
    return boot_factory_params_crc32(&copy, sizeof(copy)) == saved_crc;
}

__boot_factory_ramcode__ static bool factory_image_is_valid(uint32_t app_addr)
{
    const volatile uint8_t *header =
        (const volatile uint8_t *)(uintptr_t)(app_addr + BOOT_FACTORY_HEADER_OFFSET);
    uint32_t load_addr = 0U;

    if (header[0] != 'H' || header[1] != 'r') {
        return false;
    }

    for (size_t i = 0; i < sizeof(load_addr); ++i) {
        load_addr |= (uint32_t)header[BOOT_FACTORY_LOAD_ADDR_OFFSET + i] << (i * 8U);
    }

    return load_addr == app_addr;
}

__boot_factory_ramcode__ uint32_t boot_factory_app_addr_get(uint32_t default_app_addr)
{
    const volatile uint32_t *source = (const volatile uint32_t *)(uintptr_t)(
        CONFIG_MEM_FLASH_BASE + CONFIG_BOOT_FACTORY_PARAMS_OFFSET);
    struct boot_factory_params params = {
        .magic = source[0],
        .header_size = source[1],
        .header_crc = source[2],
        .boot_addr = source[3],
        .payload_addr = source[4],
        .payload_size = source[5],
    };

    if (!boot_factory_params_validate(&params)) {
        return default_app_addr;
    }

    uint32_t factory_app_addr = CONFIG_MEM_FLASH_BASE + params.boot_addr;
    return factory_image_is_valid(factory_app_addr) ? factory_app_addr : default_app_addr;
}
