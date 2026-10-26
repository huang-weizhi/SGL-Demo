#include "CZ01.h"

CZ01_t scz01 =
    {
        .set_temp_c = 150,
        .set_temp_f = 302,

        .ch1_set_temp_c = 120,
        .ch1_set_temp_f = 248,
        .ch2_set_temp_c = 180,
        .ch2_set_temp_f = 356,
        .ch3_set_temp_c = 240,
        .ch3_set_temp_f = 464,
        .set_time = 30,
        //.function.speaker = ON,
};

static int parse_version_string(const char *str, uint8_t *major, uint8_t *minor, uint8_t *patch);
uint32_t convert_data(const uint8_t *input_array, uint32_t *output_array,
                      uint32_t start_index, uint32_t end_index);

void get_cz01_version(CZ01_t *this)
{
    parse_version_string(SOFTWARE_VERSTION, &this->sw_ver_major, &this->sw_ver_minor, &this->sw_ver_patch);
    parse_version_string(HARDWARE_VERSTION, &this->hw_ver_major, &this->hw_ver_minor, &this->hw_ver_patch);
	}

#define STORAGE_INFO 0x1FFFF7E0
#define STORAGE_INFO_SRAM
#define STORAGE_INFO_FLASH

#define UNIQUE_ID 0x1FFFF7E8

#define UNIQUE_ID_0_31 0x1FFFF7E8
#define UNIQUE_ID_32_63 0x1FFFF7EC
#define UNIQUE_ID_64_95 0x1FFFF7F0

uint32_t get_mcu_info(CZ01_t *this)
{
    this->mcu_size = *(uint32_t *)STORAGE_INFO;
    uint32_t uid = 0;
    uint32_t uid_crc_buf[12];
    for (uint8_t i = 0; i < 12; i++)
    {
        this->mcu_uid_buff[i] = *(uint8_t *)(UNIQUE_ID + i);
    }
    convert_data(this->mcu_uid_buff, uid_crc_buf, 0, 12);
    uid = crc_block_calculate(uid_crc_buf, 3);
    crc_data_reset();
    return uid;
}

static int parse_version_string(const char *str, uint8_t *major, uint8_t *minor, uint8_t *patch)
{
    if (str == NULL || str[0] != 'V')
    {
        return -1;
    }

    const char *p = str + 1;

    *major = 0;
    while (*p >= '0' && *p <= '9')
    {
        *major = *major * 10 + (*p - '0');
        p++;
    }
    if (*p != '.')
        return -1;
    p++;

    *minor = 0;
    while (*p >= '0' && *p <= '9')
    {
        *minor = *minor * 10 + (*p - '0');
        p++;
    }
    if (*p != '.')
        return -1;
    p++;

    *patch = 0;
    while (*p >= '0' && *p <= '9')
    {
        *patch = *patch * 10 + (*p - '0');
        p++;
    }

    return 0;
}

uint32_t convert_data(const uint8_t *input_array, uint32_t *output_array,
                      uint32_t start_index, uint32_t end_index)
{
    if (input_array == NULL || output_array == NULL)
    {
        return 0;
    }

    if (start_index > end_index)
    {
        return 0;
    }

    uint32_t byte_count = end_index - start_index + 1;
    const uint8_t *ptr = &input_array[start_index];
    uint32_t i;
    uint32_t output_index = 0;

    for (i = 0; i < byte_count / 4; i++)
    {
        output_array[output_index++] = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
        ptr += 4;
    }

    if (byte_count % 4 != 0)
    {
        uint32_t word = 0;
        uint8_t remaining = byte_count % 4;

        for (uint8_t j = 0; j < remaining; j++)
        {
            word |= ptr[j] << (24 - (j * 8));
        }

        output_array[output_index++] = word;
    }

    return output_index;
}