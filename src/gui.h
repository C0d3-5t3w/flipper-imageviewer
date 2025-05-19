#pragma once

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/modules/file_browser.h>
#include <gui/modules/loading.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include "extwalk.h"

// Forward declare to prevent circular includes
typedef struct ImageViewer ImageViewer;

// Concrete struct definition
typedef struct ImageViewer {
    View* view;
    uint8_t* bitmap;
    uint16_t width;
    uint16_t height;
    char current_file[256];
} ImageViewer;

// Viewer API
ImageViewer* image_viewer_alloc();
void image_viewer_free(ImageViewer* app);
View* image_viewer_get_view(ImageViewer* app);
void image_viewer_set_file(ImageViewer* app, const char* path);
