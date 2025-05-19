#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_port.h>
#include <gui/modules/dialog_ex.h>
#include <input/input.h>
#include <storage/storage.h>
#include "src/gui.h"
#include "src/extwalk.h"
#include "src/gui_helper.h" // Added header for gui_add_view/remove_view

// Event flags for button input
#define EVENT_MASK_BUTTONS (1U << 0U)

// Event types
typedef enum {
    EventTypeKey,
    EventTypeBack
} EventType;

// Main input queue
static FuriMessageQueue* event_queue;

static void file_selected_callback(const char* filename, void* context) {
    ImageViewer* app = context;
    image_viewer_set_file(app, filename);
}

int32_t imageviewer_app(void* p) {
    UNUSED(p);

    // Initialize modules
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Gui* gui = furi_record_open(RECORD_GUI);

    // Allocate viewer
    ImageViewer* app = image_viewer_alloc();
    View* view = image_viewer_get_view(app);

    // Create message queue
    event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Initialize file walker
    extwalk_init(storage);

    // Register view with GUI
    // Using the proper APIs from gui.h
    gui_add_view(gui, view);

    // Initial scan for images
    extwalk_scan_dir("/ext", file_selected_callback, app);

    // Main event loop
    bool running = true;
    while(running) {
        InputEvent event;
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                switch(event.key) {
                case InputKeyBack:
                    running = false;
                    break;
                default:
                    break;
                }
            }
        }
    }

    // Cleanup
    furi_message_queue_free(event_queue);
    gui_remove_view(gui, view);
    image_viewer_free(app);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);

    return 0;
}
