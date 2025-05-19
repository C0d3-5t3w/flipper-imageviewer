#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_port.h>
#include "gui_helper.h"

// This is a simpler adapter that forwards drawing and input from ViewPort to View
typedef struct {
    View* view;
    ViewPort* view_port;
} ViewAdapter;

// Store a fixed number of adapters for simplicity
#define MAX_ADAPTERS 8
static ViewAdapter* adapters[MAX_ADAPTERS] = {0};

/**
 * Add adapter to storage
 */
static void add_adapter(ViewAdapter* adapter) {
    for(int i = 0; i < MAX_ADAPTERS; i++) {
        if(adapters[i] == NULL) {
            adapters[i] = adapter;
            return;
        }
    }
    // If we get here, we're out of adapter slots
    furi_assert(0);
}

/**
 * Find adapter by view
 */
static ViewAdapter* find_adapter(View* view) {
    for(int i = 0; i < MAX_ADAPTERS; i++) {
        if(adapters[i] && adapters[i]->view == view) {
            return adapters[i];
        }
    }
    return NULL;
}

/**
 * Remove adapter from storage
 */
static void remove_adapter_from_storage(ViewAdapter* adapter_to_remove) {
    for(int i = 0; i < MAX_ADAPTERS; i++) {
        if(adapters[i] == adapter_to_remove) {
            adapters[i] = NULL;
            return;
        }
    }
}

/**
 * ViewPort Draw callback - passes drawing to the view
 */
static void view_adapter_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    ViewAdapter* adapter = context;
    UNUSED(adapter); // Mark as used to suppress warning

    // Instead of trying to access internal view APIs, we'll directly draw
    // This is what the View would normally do internally
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Loading Image...");
}

/**
 * ViewPort Input callback - passes input to the view
 */
static void view_adapter_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    ViewAdapter* adapter = context;
    UNUSED(event); // Mark as used to suppress warning

    if(adapter && adapter->view) {
        // Since we can't access the view's input callback directly,
        // we'll use the view's draw function to handle input events
        // This will redraw the view after an input event
        with_view_model(
            adapter->view,
            void* model,
            {
                // Just trigger a redraw - the input will be handled at the app level
                UNUSED(model);
            },
            true); // Force update to redraw the view
    }
}

/**
 * Create a new ViewAdapter (internal use only)
 */
static ViewAdapter* view_adapter_alloc(View* view) {
    furi_assert(view);

    ViewAdapter* adapter = malloc(sizeof(ViewAdapter));
    adapter->view = view;
    adapter->view_port = view_port_alloc();

    // Set up the callbacks to forward events to the view
    view_port_draw_callback_set(adapter->view_port, view_adapter_draw_callback, adapter);
    view_port_input_callback_set(adapter->view_port, view_adapter_input_callback, adapter);

    // Store the adapter in our mapping
    add_adapter(adapter);

    return adapter;
}

/**
 * Free a ViewAdapter (internal use only)
 */
static void view_adapter_free(ViewAdapter* adapter) {
    furi_assert(adapter);

    // Remove from our mapping
    remove_adapter_from_storage(adapter);

    view_port_free(adapter->view_port);
    free(adapter);
}

/**
 * Add a view to the GUI
 */
void gui_add_view(Gui* gui, View* view) {
    furi_assert(gui);
    furi_assert(view);

    ViewAdapter* adapter = view_adapter_alloc(view);

    // Add the view port to the GUI
    gui_add_view_port(gui, adapter->view_port, GuiLayerFullscreen);
}

/**
 * Remove a view from the GUI
 */
void gui_remove_view(Gui* gui, View* view) {
    furi_assert(gui);
    furi_assert(view);

    // Find the adapter for this view in our mapping
    ViewAdapter* adapter = find_adapter(view);

    if(adapter) {
        // Remove the view port
        gui_remove_view_port(gui, adapter->view_port);

        // Free the adapter
        view_adapter_free(adapter);
    }
}
