#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Image conversion result enum
typedef enum {
    ImageConverterOK,
    ImageConverterError,
    ImageConverterUnsupported
} ImageConverterResult;

// Convert file to 1-bit bitmap for Flipper display
ImageConverterResult image_convert_to_bitmap(
    const char* filename,
    uint8_t* bitmap,
    uint16_t* width,
    uint16_t* height);

// Converts grayscale or RGB data to 1-bit 128x64 bitmap
void image_convert_to_bitmap128x64(
    const uint8_t* input_data,
    size_t input_width,
    size_t input_height,
    uint8_t* output_bitmap);

#ifdef __cplusplus
}
#endif
