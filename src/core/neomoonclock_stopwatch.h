#pragma once

#include "neomoonclock.h"

void stopwatch_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void* stopwatch_run(void* _arg);
void stopwatch_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void stopwatch_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void stopwatch_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void stopwatch_update_gui(neomoonclock_t* _neomoonclock);
