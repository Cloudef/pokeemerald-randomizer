#pragma once

#include <stdint.h>
#include <stddef.h>

size_t pokemon_text_to_utf8(const uint8_t *input, const size_t input_sz, char *out_buf, const size_t out_buf_sz);
size_t pokemon_utf8_to_text(const char *input, const size_t input_sz, uint8_t *out_buf, const size_t out_buf_sz);
