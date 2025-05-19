#include "extwalk.h"
#include <string.h>

#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <storage/storage_sd_api.h>

#include <imageviewer_icons.h>

typedef enum {
    EXTWALK_OK,
    EXTWALK_ERROR,
} ExtwalkStatus;

typedef enum {
    EXTWALK_FILE,
    EXTWALK_DIRECTORY,
} ExtwalkType;

typedef enum {
    EXTWALK_SORT_NONE,
    EXTWALK_SORT_NAME,
    EXTWALK_SORT_SIZE,
    EXTWALK_SORT_DATE,
} ExtwalkSort;

typedef enum {
    EXTWALK_ASCENDING,
    EXTWALK_DESCENDING,
} ExtwalkOrder;

typedef enum {
    EXTWALK_FILTER_NONE,
    EXTWALK_FILTER_IMAGE,
    EXTWALK_FILTER_VIDEO,
    EXTWALK_FILTER_AUDIO,
} ExtwalkFilter;

typedef enum {
    EXTWALK_ACTION_NONE,
    EXTWALK_ACTION_OPEN,
    EXTWALK_ACTION_DELETE,
    EXTWALK_ACTION_RENAME,
} ExtwalkAction;

typedef enum {
    EXTWALK_STATUS_IDLE,
    EXTWALK_STATUS_LOADING,
    EXTWALK_STATUS_ERROR,
} ExtwalkStatusType;

typedef enum {
    EXTWALK_EVENT_NONE,
    EXTWALK_EVENT_FILE_SELECTED,
    EXTWALK_EVENT_DIRECTORY_SELECTED,
    EXTWALK_EVENT_ACTION_PERFORMED,
} ExtwalkEventType;

typedef enum {
    EXTWALK_EVENT_OK,
    EXTWALK_EVENT_CANCEL,
} ExtwalkEventResult;

typedef enum {
    EXTWALK_EVENT_FILE_SELECTED_OK,
    EXTWALK_EVENT_FILE_SELECTED_CANCEL,
} ExtwalkEventFileSelectedResult;

typedef enum {
    EXTWALK_EVENT_DIRECTORY_SELECTED_OK,
    EXTWALK_EVENT_DIRECTORY_SELECTED_CANCEL,
} ExtwalkEventDirectorySelectedResult;

typedef enum {
    EXTWALK_EVENT_ACTION_PERFORMED_OK,
    EXTWALK_EVENT_ACTION_PERFORMED_CANCEL,
} ExtwalkEventActionPerformedResult;

typedef enum {
    EXTWALK_EVENT_ERROR_OK,
    EXTWALK_EVENT_ERROR_CANCEL,
} ExtwalkEventErrorResult;

typedef struct {
    const char* path;
    const char* extension;
} ExtwalkFile;

typedef struct {
    ExtwalkFile* files;
    size_t file_count;
} ExtwalkFileList;

typedef struct {
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
    ExtwalkStatusType status;
    ExtwalkEventType event_type;
    ExtwalkEventResult event_result;
    ExtwalkEventFileSelectedResult event_file_selected_result;
    ExtwalkEventDirectorySelectedResult event_directory_selected_result;
    ExtwalkEventActionPerformedResult event_action_performed_result;
    ExtwalkEventErrorResult event_error_result;
} ExtwalkContext;

typedef struct {
    const char* path;
    const char* filename;
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
} ExtwalkFileInfo;

typedef struct {
    const char* path;
    const char* filename;
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
} ExtwalkDirectoryInfo;

typedef struct {
    const char* path;
    const char* filename;
    size_t count; // Use this instead of file_count for consistency
    ExtwalkFile* items; // Use this instead of files for consistency
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
} ExtwalkDirectoryInfoList;

typedef struct {
    const char* path;
    const char* filename;
    size_t file_count;
    ExtwalkFile* files;
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
} ExtwalkFileInfoList;

typedef struct {
    const char* path;
    const char* filename;
    size_t file_count;
    ExtwalkFile* files;
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
} ExtwalkFileInfoListItem;

typedef struct {
    const char* path;
    const char* filename;
    size_t file_count;
    ExtwalkFile* files;
    ExtwalkType type;
    ExtwalkSort sort;
    ExtwalkOrder order;
    ExtwalkFilter filter;
    ExtwalkAction action;
} ExtwalkDirectoryInfoListItem;

static Storage* storage_ptr;

void extwalk_init(Storage* storage) {
    storage_ptr = storage;
}

static bool is_image_file(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if(!ext) return false;
    return strstr(IMAGE_EXTENSIONS, ext) != NULL;
}

void extwalk_scan_dir(const char* path, FileFoundCallback callback, void* context) {
    File* dir = storage_file_alloc(storage_ptr);
    if(storage_dir_open(dir, path)) {
        char filename[256];
        while(storage_dir_read(dir, NULL, filename, sizeof(filename))) {
            if(is_image_file(filename)) {
                callback(filename, context);
            }
        }
    }
    storage_dir_close(dir);
    storage_file_free(dir);
}

bool extwalk_get_next_image(const char* current, char* next, size_t size) {
    if(!current || !next) return false;

    // Get directory path
    char dir_path[256];
    strncpy(dir_path, current, sizeof(dir_path));
    char* last_slash = strrchr(dir_path, '/');
    if(last_slash) *last_slash = '\0';

    // Open directory
    File* dir = storage_file_alloc(storage_ptr);
    if(!storage_dir_open(dir, dir_path)) {
        storage_file_free(dir);
        return false;
    }

    char filename[256];
    bool found_current = false;
    bool found_next = false;

    // Scan directory
    while(storage_dir_read(dir, NULL, filename, sizeof(filename))) {
        if(!found_current) {
            // Look for current file
            if(strstr(current, filename)) {
                found_current = true;
            }
        } else {
            // Look for next image file
            if(is_image_file(filename)) {
                snprintf(next, size, "%s/%s", dir_path, filename);
                found_next = true;
                break;
            }
        }
    }

    storage_dir_close(dir);
    storage_file_free(dir);

    return found_next;
}

bool extwalk_get_prev_image(const char* current, char* prev, size_t size) {
    UNUSED(current);
    UNUSED(prev);
    UNUSED(size);
    // TODO: Implement finding previous image
    return false;
}

const char* extwalk_get_extension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if(ext && ext != filename) {
        return ext + 1;
    }
    return NULL;
}
const char* extwalk_get_mime_type(const char* filename) {
    const char* ext = extwalk_get_extension(filename);
    if(ext) {
        if(strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
            return "image/jpeg";
        } else if(strcmp(ext, "png") == 0) {
            return "image/png";
        } else if(strcmp(ext, "gif") == 0) {
            return "image/gif";
        } else if(strcmp(ext, "bmp") == 0) {
            return "image/bmp";
        }
    }
    return NULL;
}

void extwalk_get_file_info(const char* path, ExtwalkFileInfo* file_info) {
    if(path == NULL || file_info == NULL) {
        return;
    }
    // Example: Let's say we found a file with the following properties
    file_info->path = path;
    file_info->filename = "example.jpg";
    file_info->type = EXTWALK_FILE;
    file_info->sort = EXTWALK_SORT_NAME;
    file_info->order = EXTWALK_ASCENDING;
    file_info->filter = EXTWALK_FILTER_IMAGE;
    file_info->action = EXTWALK_ACTION_OPEN;
}

void extwalk_get_directory_info(const char* path, ExtwalkDirectoryInfo* dir_info) {
    if(path == NULL || dir_info == NULL) {
        return;
    }
    // Example: Let's say we found a directory with the following properties
    dir_info->path = path;
    dir_info->filename = "example_directory";
    dir_info->type = EXTWALK_DIRECTORY;
    dir_info->sort = EXTWALK_SORT_NAME;
    dir_info->order = EXTWALK_ASCENDING;
    dir_info->filter = EXTWALK_FILTER_NONE;
    dir_info->action = EXTWALK_ACTION_NONE;
}

void extwalk_get_file_list(const char* path, ExtwalkFileList* file_list) {
    if(path == NULL || file_list == NULL) {
        return;
    }
    // Example: Let's say we found 3 files
    file_list->file_count = 3;
    file_list->files = malloc(file_list->file_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < file_list->file_count; i++) {
        file_list->files[i].path = path;
        file_list->files[i].extension = "jpg";
    }
}
void extwalk_get_directory_list(const char* path, ExtwalkDirectoryInfoList* dir_list) {
    if(path == NULL || dir_list == NULL) {
        return;
    }
    dir_list->count = 2;
    dir_list->items = malloc(dir_list->count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < dir_list->count; i++) {
        dir_list->items[i].path = path;
        dir_list->items[i].extension = "dir";
    }
}

void extwalk_free_directory_list(ExtwalkDirectoryInfoList* dir_list) {
    if(dir_list == NULL) {
        return;
    }
    free(dir_list->items);
    dir_list->items = NULL;
    dir_list->count = 0;
}

void extwalk_free_file_list(ExtwalkFileList* file_list) {
    if(file_list == NULL) {
        return;
    }
    free(file_list->files);
    file_list->files = NULL;
    file_list->file_count = 0;
}

void extwalk_set_file_info(ExtwalkFileInfo* file_info, const char* path, const char* filename) {
    if(file_info == NULL || path == NULL || filename == NULL) {
        return;
    }
    file_info->path = path;
    file_info->filename = filename;
}

void extwalk_set_directory_info(
    ExtwalkDirectoryInfo* dir_info,
    const char* path,
    const char* filename) {
    if(dir_info == NULL || path == NULL || filename == NULL) {
        return;
    }
    dir_info->path = path;
    dir_info->filename = filename;
}

void extwalk_set_file_list(ExtwalkFileList* file_list, const char* path, size_t file_count) {
    if(file_list == NULL || path == NULL) {
        return;
    }
    file_list->file_count = file_count;
    file_list->files = malloc(file_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < file_count; i++) {
        file_list->files[i].path = path;
        file_list->files[i].extension = "jpg";
    }
}

void extwalk_set_directory_list(
    ExtwalkDirectoryInfoList* dir_list,
    const char* path,
    size_t dir_count) {
    if(dir_list == NULL || path == NULL) {
        return;
    }
    dir_list->count = dir_count;
    dir_list->items = malloc(dir_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < dir_count; i++) {
        dir_list->items[i].path = path;
        dir_list->items[i].extension = "dir";
    }
}

void extwalk_set_file_info_list(
    ExtwalkFileInfoList* file_info_list,
    const char* path,
    size_t file_count) {
    if(file_info_list == NULL || path == NULL) {
        return;
    }
    file_info_list->file_count = file_count;
    file_info_list->files = malloc(file_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < file_count; i++) {
        file_info_list->files[i].path = path;
        file_info_list->files[i].extension = "jpg";
    }
}

void extwalk_set_directory_info_list(
    ExtwalkDirectoryInfoList* dir_info_list,
    const char* path,
    size_t dir_count) {
    if(dir_info_list == NULL || path == NULL) {
        return;
    }
    dir_info_list->count = dir_count;
    dir_info_list->items = malloc(dir_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < dir_count; i++) {
        dir_info_list->items[i].path = path;
        dir_info_list->items[i].extension = "dir";
    }
}

void extwalk_set_file_info_list_item(
    ExtwalkFileInfoListItem* file_info_list_item,
    const char* path,
    size_t file_count) {
    if(file_info_list_item == NULL || path == NULL) {
        return;
    }
    file_info_list_item->file_count = file_count;
    file_info_list_item->files = malloc(file_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < file_count; i++) {
        file_info_list_item->files[i].path = path;
        file_info_list_item->files[i].extension = "jpg";
    }
}

void extwalk_set_directory_info_list_item(
    ExtwalkDirectoryInfoListItem* dir_info_list_item,
    const char* path,
    size_t file_count) {
    if(dir_info_list_item == NULL || path == NULL) {
        return;
    }
    dir_info_list_item->file_count = file_count;
    dir_info_list_item->files = malloc(file_count * sizeof(ExtwalkFile));
    for(size_t i = 0; i < file_count; i++) {
        dir_info_list_item->files[i].path = path;
        dir_info_list_item->files[i].extension = "dir";
    }
}
