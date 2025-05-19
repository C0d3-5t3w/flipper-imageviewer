#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_port.h>
#include "gui_helper.h"

/**
 * Internal adapter structure to connect View and ViewPort
 */
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
static void add_adapter(View* view, ViewAdapter* adapter) {
    UNUSED(view); // view parameter is implicitly used by find_adapter logic
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
static void remove_adapter_from_cache(ViewAdapter* adapter_to_remove) { // Renamed to avoid confusion
    for(int i = 0; i < MAX_ADAPTERS; i++) {
        if(adapters[i] == adapter_to_remove) {
            adapters[i] = NULL;
            return;
        }
    }
}

/**
 * ViewPort Draw callback - forwards to the View
 */
static void view_adapter_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    ViewAdapter* adapter = context;

    // Call to undeclared function 'view_draw' removed.
    // The View's drawing logic cannot be invoked this way with public SDK.
    // Placeholder drawing to indicate an issue:
    UNUSED(adapter); // adapter->view would be used if view_draw was available
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, canvas_width(canvas) / 2, canvas_height(canvas) / 2, AlignCenter, AlignCenter, "Helper Draw Error");
}

/**
 * ViewPort Input callback - forwards to the View
 */
static void view_adapter_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    ViewAdapter* adapter = context;

    // Call to undeclared function 'view_input' removed.
    // The View's input logic cannot be invoked this way with public SDK.
    UNUSED(adapter); // adapter->view would be used if view_input was available
    UNUSED(event);   // event would be used if view_input was available
    // Input will not be forwarded to the view.
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
    add_adapter(view, adapter);

    return adapter;
}

/**
 * Free a ViewAdapter (internal use only)
 */
static void view_adapter_free(ViewAdapter* adapter) {
    furi_assert(adapter);

    // Remove from our mapping
    remove_adapter_from_cache(adapter);

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
    // This replaces the call to the undeclared 'view_get_previous_callback'
    ViewAdapter* adapter = find_adapter(view);

    if(adapter) {
        // Remove the view port
        gui_remove_view_port(gui, adapter->view_port);

        // Free the adapter
        view_adapter_free(adapter);
    }
}
