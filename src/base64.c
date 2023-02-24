/*base64.c*/
#include "base64.h"
char *lfs_base64_encode(const char *str, uint64_t len)
{
    uint64_t length = CEIL_POS(len * 4 / 3) + 1;
    char *base64Chars = (char *)malloc(sizeof(char) * length);
    uint64_t idx = 0;

    for (uint64_t i = 0; i < len; i += 3)
    {
        uint32_t byte1 = (uint8_t)str[i];
        uint16_t byte2 = (i + 1 < len) ? (uint8_t)str[i + 1] : 0;
        uint8_t byte3 = (i + 2 < len) ? (uint8_t)str[i + 2] : 0;

        uint32_t triplet = (byte1 << 16) | (byte2 << 8) | byte3;

        for (uint64_t j = 0; (j < 4) && (i + j * 0.75 < len); j++)
        {
            base64Chars[idx] = BASE64_MAP[(triplet >> (6 * (3 - j))) & 0x3f];
            idx++;
        }
    }

    char paddingChar = BASE64_MAP[64];
    if (paddingChar)
    {
        while (idx % 4)
        {
            base64Chars[idx] = paddingChar;
            idx++;
        }
    }
    base64Chars[idx] = 0;
    return base64Chars;
}

char *lfs_base64_decode(const char *base64Str, uint64_t len)
{
    while (base64Str[len - 1] == BASE64_MAP[64])
    {
        len--;
    }
    uint64_t length = CEIL_POS(len * 3 / 4) + 1;
    char *str = (char *)malloc(sizeof(char) * length);
    uint64_t idx = 0;

    for (uint64_t i = 0; i < len; i += 4)
    {
        uint32_t triplet = 0;
        for (uint8_t j = 0; j < 4; ++j)
        {
            if (i + j < len)
                triplet = (triplet << 6) | ((uint8_t)BASE64_REVERSE_MAP[base64Str[i + j]] & 0x3f);
            else
                triplet = triplet << 6;
        }
        for (uint8_t j = 0; (j < 3); ++j)
        {
            str[idx] = (triplet >> (8 * (2 - j))) & 0xff;
            idx++;
        }
    }
    str[idx] = 0;
    return str;
}