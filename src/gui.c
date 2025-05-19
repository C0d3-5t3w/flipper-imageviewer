#include <furi.h>
#include <furi_hal.h>
#include <gui/canvas.h>
#include <gui/modules/validators.h>
#include <gui/view.h>
#include <gui/view_port.h>
#include <gui/modules/file_browser.h>
#include "gui.h"
#include "convert.h"

#define TAG           "ImageViewer"
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

typedef enum {
    IMAGEVIEWER_ICON_SIZE_DEFAULT,
    IMAGEVIEWER_ICON_SIZE_LARGE,
    IMAGEVIEWER_ICON_SIZE_MEDIUM,
} ImageViewerIconSize;

#define IMAGEVIEWER_ICON_SIZE_MEDIUM  10
#define IMAGEVIEWER_ICON_SIZE_LARGE   16
#define IMAGEVIEWER_ICON_SIZE_DEFAULT 8

typedef enum {
    IMAGEVIEWER_EVENT_NONE,
    IMAGEVIEWER_EVENT_FILE_SELECTED,
    IMAGEVIEWER_EVENT_DIRECTORY_SELECTED,
    IMAGEVIEWER_EVENT_ACTION_PERFORMED,
} ImageViewerEvent;

typedef enum {
    IMAGEVIEWER_EVENT_OK,
    IMAGEVIEWER_EVENT_CANCEL,
} ImageViewerEventResult;

typedef enum {
    FILE_BROWSER_EVENT_NONE,
    FILE_BROWSER_EVENT_OPEN,
    FILE_BROWSER_EVENT_CLOSE,
} FileBrowserEvent;

typedef enum {
    FILE_BROWSER,
    FILE_BROWSER_WORKER,
} FileBrowserWorker;

typedef struct {
    View* view;
    ViewPort* view_port;
    Canvas* canvas;
    FileBrowser* file_browser;
    FileBrowserWorker* file_browser_worker;
    Loading* loading;
    Submenu* submenu;
    Popup* popup;
} ImageViewerContext;

#if 0
static void imageviewer_set_icon_size(ImageViewerContext* context, ImageViewerIconSize icon_size) {
    if(context->canvas) {
        // Handle icon size setting
        UNUSED(icon_size);
    }
}

static void file_browser_callback(FileBrowser* file_browser, FileBrowserEvent event) {
    if(file_browser == NULL) {
        return;
    }

    switch(event) {
    case FILE_BROWSER_EVENT_OPEN:
        // Handle open event
        break;
    case FILE_BROWSER_EVENT_CLOSE:
        // Handle close event
        break;
    default:
        break;
    }
}
#endif

static void draw_callback(Canvas* canvas, void* ctx) {
    ImageViewer* app = ctx;
    if(app->bitmap) {
        canvas_draw_bitmap(canvas, 0, 0, app->width, app->height, app->bitmap);
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "No Image");
    }
}

static bool input_callback(InputEvent* event, void* ctx) {
    ImageViewer* app = ctx;
    bool handled = false;

    if(event->type == InputTypeShort) {
        char next_file[256];
        switch(event->key) {
        case InputKeyRight:
            if(extwalk_get_next_image(app->current_file, next_file, sizeof(next_file))) {
                image_viewer_set_file(app, next_file);
            }
            handled = true;
            break;
        case InputKeyLeft:
            if(extwalk_get_prev_image(app->current_file, next_file, sizeof(next_file))) {
                image_viewer_set_file(app, next_file);
            }
            handled = true;
            break;
        default:
            break;
        }
    }

    return handled;
}

ImageViewer* image_viewer_alloc() {
    ImageViewer* app = malloc(sizeof(ImageViewer));
    app->view = view_alloc();
    app->bitmap = NULL;
    app->current_file[0] = '\0';

    view_set_context(app->view, app);
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(ImageViewer));
    view_set_draw_callback(app->view, draw_callback);
    view_set_input_callback(app->view, input_callback);

    return app;
}

void image_viewer_free(ImageViewer* app) {
    if(app->bitmap) free(app->bitmap);
    view_free(app->view);
    free(app);
}

View* image_viewer_get_view(ImageViewer* app) {
    return app->view;
}

void image_viewer_set_file(ImageViewer* app, const char* path) {
    if(app->bitmap) {
        free(app->bitmap);
        app->bitmap = NULL;
    }

    strncpy(app->current_file, path, sizeof(app->current_file) - 1);

    // Allocate bitmap buffer
    app->bitmap = malloc(128 * 64 / 8);
    if(!app->bitmap) {
        FURI_LOG_E(TAG, "Failed to allocate bitmap");
        return;
    }

    // Convert image to bitmap
    ImageConverterResult result =
        image_convert_to_bitmap(path, app->bitmap, &app->width, &app->height);

    if(result != ImageConverterOK) {
        FURI_LOG_E(TAG, "Failed to convert image");
        free(app->bitmap);
        app->bitmap = NULL;
        return;
    }

    view_commit_model(app->view, true);
}
