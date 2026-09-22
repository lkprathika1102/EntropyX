#ifndef ENTROPY_MATH_H
#define ENTROPY_MATH_H

#include <stdint.h>
#include <stdbool.h>

#define ENTROPY_RAW_POOL_BITS 1024 
#define EXTRACTED_KEY_BYTES   32   

typedef struct {
    float min_entropy_per_bit;
    bool monobit_pass;
    bool runs_pass;
    bool serial_pass;
    bool passes_all_criteria;
} nist_stats_t;

void entropy_accumulate(const int16_t *samples, uint8_t count, uint8_t *bit_pool, uint16_t *bit_idx);
nist_stats_t entropy_run_nist_eval(const uint8_t *bit_pool, uint16_t total_bits);
void entropy_toeplitz_extract(const uint8_t *in_bits, uint16_t in_len, uint8_t *out_bytes, uint16_t out_len);

#endif
