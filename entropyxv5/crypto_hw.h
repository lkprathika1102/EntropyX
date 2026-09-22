#ifndef CRYPTO_HW_H
#define CRYPTO_HW_H

#include <stdint.h>
#include <stdbool.h>

void crypto_hw_init(void);
bool crypto_hw_sha256(const uint8_t *input, uint16_t in_len, uint8_t *out_hash_32bytes);

#endif
