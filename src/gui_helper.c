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

/**
 * ViewPort Draw callback - forwards to the View
 */
static void view_adapter_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    ViewAdapter* adapter = context;

    // Call the view's draw function directly
    // This works because view_draw will take care of getting the model,
    // calling the callback with appropriate context, and releasing the model
    view_draw(adapter->view, canvas);
}

/**
 * ViewPort Input callback - forwards to the View
 */
static void view_adapter_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    ViewAdapter* adapter = context;

    // Call the view's input function directly
    // This works because view_input will take care of getting the model,
    // calling the callback with appropriate context, and releasing the model
    view_input(adapter->view, event);
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

    return adapter;
}

/**
 * Free a ViewAdapter (internal use only)
 */
static void view_adapter_free(ViewAdapter* adapter) {
    furi_assert(adapter);

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

    // Store the adapter pointer as the view's data
    // This way we can retrieve it later when removing the view
    view_set_previous_callback(view, (ViewNavigationCallback)adapter);

    // Add the view port to the GUI
    gui_add_view_port(gui, adapter->view_port, GuiLayerFullscreen);
}

/**
 * Remove a view from the GUI
 */
void gui_remove_view(Gui* gui, View* view) {
    furi_assert(gui);
    furi_assert(view);

    // Get back the adapter pointer that we stored earlier
    ViewAdapter* adapter = (ViewAdapter*)view_get_previous_callback(view);

    if(adapter) {
        // Remove the view port
        gui_remove_view_port(gui, adapter->view_port);

        // Free the adapter
        view_adapter_free(adapter);

        // Clear the reference
        view_set_previous_callback(view, NULL);
    }
}
