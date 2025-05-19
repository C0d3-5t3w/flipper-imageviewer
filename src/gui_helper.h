#pragma once

#include <gui/gui.h>
#include <gui/view.h>

/**
 * @brief Add a view to the GUI
 * 
 * Creates a view port and attaches the view to it
 * 
 * @param gui Gui instance
 * @param view View to add
 */
void gui_add_view(Gui* gui, View* view);

/**
 * @brief Remove a view from the GUI
 * 
 * @param gui Gui instance
 * @param view View to remove
 */
void gui_remove_view(Gui* gui, View* view);
