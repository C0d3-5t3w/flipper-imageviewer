#pragma once

#include <storage/storage.h>

// Supported file extensions
#define IMAGE_EXTENSIONS ".bmp.png.jpg.jpeg"

// Directory info struct
typedef struct {
    size_t count;
    size_t capacity;
    char** items;
} DirectoryList;

typedef void (*FileFoundCallback)(const char* filename, void* context);

// File walker API
void extwalk_init(Storage* storage);
void extwalk_scan_dir(const char* path, FileFoundCallback callback, void* context);
bool extwalk_get_next_image(const char* current, char* next, size_t size);
bool extwalk_get_prev_image(const char* current, char* prev, size_t size);
