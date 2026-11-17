#include "boot_factory_params.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static struct boot_factory_params valid_params(void)
{
    struct boot_factory_params params = {
        .magic = BOOT_FACTORY_PARAMS_MAGIC,
        .header_size = BOOT_FACTORY_PARAMS_HEADER_WORDS,
        .header_crc = 0U,
        .boot_addr = CONFIG_BOOT_FACTORY_TEST_OFFSET,
        .payload_addr = BOOT_FACTORY_PARAMS_HEADER_WORDS,
        .payload_size = 128U,
    };

    params.header_crc = boot_factory_params_crc32(&params, sizeof(params));
    return params;
}

int main(void)
{
    static const char crc_input[] = "123456789";
    struct boot_factory_params params = valid_params();

    assert(boot_factory_params_crc32(crc_input, strlen(crc_input)) == 0xCBF43926U);
    assert(boot_factory_params_validate(&params));

    params.boot_addr = 0x600000U;
    assert(!boot_factory_params_validate(&params));

    params = valid_params();
    params.header_crc ^= 1U;
    assert(!boot_factory_params_validate(&params));

    params = valid_params();
    params.payload_size = BOOT_FACTORY_PARAMS_SECTOR_SIZE;
    params.header_crc = 0U;
    params.header_crc = boot_factory_params_crc32(&params, sizeof(params));
    assert(!boot_factory_params_validate(&params));

    return 0;
}
