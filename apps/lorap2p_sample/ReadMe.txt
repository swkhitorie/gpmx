
BootLoader+App in STM32WLE5
flash map:
    namne      region
    boot       0x08000000 ~ 0x0800b7ff
    nvm        0x0800b800 ~ 0x0800bfff
    appa       0x0800c000 ~ 0x08025fff
    appb       0x08026000 ~ 0x0803ffff

nvm data:
    uint32_t boot_version
    uint32_t reversed1
    uint32_t app_a_load_flag;
    uint32_t app_a_version;
    uint32_t app_a_crc;
    uint32_t app_a_size;
    uint32_t app_b_load_flag;
    uint32_t app_b_version;
    uint32_t app_b_crc;
    uint32_t app_b_size;

ota firmware head:
    uint32_t app_type;
    uint32_t version;
    uint32_t crc;
    uint32_t size;

master app name suffix:
    xxx_12_b.bin -> appb and version v1.2

