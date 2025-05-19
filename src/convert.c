#include <furi.h>
#include <furi/furi.h>
#include <furi/core/core_defines.h>
#include <furi/core/event_loop.h>
#include <furi/core/event_loop_timer.h>
#include <furi/core/event_flag.h>
#include <furi/core/kernel.h>
#include <furi/core/log.h>
#include <furi/core/memmgr.h>
#include <furi/core/memmgr_heap.h>
#include <furi/core/message_queue.h>
#include <furi/core/mutex.h>
#include <furi/core/pubsub.h>
#include <furi/core/record.h>
#include <furi/core/semaphore.h>
#include <furi/core/thread.h>
#include <furi/core/thread_list.h>
#include <furi/core/timer.h>
#include <furi/core/string.h>
#include <furi/core/stream_buffer.h>
#include <furi_hal.h>
#include <furi_hal_rtc.h>
#include <lib/bit_lib/bit_lib.h>
#include <lib/mlIB/m-array.h>
#include <storage/storage.h>
#include "convert.h"

#define IMAGE_BUF_SIZE 1024 // 128x64 / 8 bits per byte

// Nearest-neighbor resizer and threshold-to-monochrome
void image_convert_to_bitmap128x64(
    const uint8_t* input_data,
    size_t input_width,
    size_t input_height,
    uint8_t* output_bitmap) {
    memset(output_bitmap, 0, 1024);

    for(size_t y = 0; y < 64; y++) {
        for(size_t x = 0; x < 128; x++) {
            // Compute nearest pixel in original image
            size_t src_x = x * input_width / 128;
            size_t src_y = y * input_height / 64;
            size_t src_index = src_y * input_width + src_x;

            // Assume 1 byte per pixel grayscale for simplicity
            uint8_t pixel = input_data[src_index];

            // Threshold: convert to 1-bit
            uint8_t bit = (pixel > 128) ? 1 : 0;

            // Set bit in output
            size_t byte_index = (y * 128 + x) / 8;
            size_t bit_index = x % 8;
            if(bit) {
                output_bitmap[byte_index] |= (1 << (7 - bit_index));
            }
        }
    }
}

ImageConverterResult image_convert_to_bitmap(
    const char* filename,
    uint8_t* bitmap,
    uint16_t* width,
    uint16_t* height) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool success = false;

    // Open file
    if(!storage_file_open(file, filename, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return ImageConverterError;
    }

    // Read first bytes to detect format
    uint8_t header[8];
    if(storage_file_read(file, header, sizeof(header)) != sizeof(header)) {
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return ImageConverterError;
    }

    // Simple format detection
    if(header[0] == 0x42 && header[1] == 0x4D) {
        // BMP file
        // Skip header
        storage_file_seek(file, 54, false);

        // Read image data directly to bitmap buffer
        // This assumes 1-bit BMP format - add proper BMP parsing for production
        storage_file_read(file, bitmap, IMAGE_BUF_SIZE);

        *width = 128;
        *height = 64;
        success = true;
    }
    // Add more format detection and conversion here

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success ? ImageConverterOK : ImageConverterError;
}
