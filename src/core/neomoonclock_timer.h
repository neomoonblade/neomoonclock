#pragma once

#include "neomoonclock.h"

void timer_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void* timer_run(void* _arg);
void timer_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void timer_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void timer_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void timer_update_gui(neomoonclock_t* _neomoonclock);
void timer_finished(neomoonclock_t* _neomoonclock);
