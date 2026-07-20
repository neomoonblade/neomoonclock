#pragma once

#include "neomoonclock.h"

void alarm_enabled_check_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
void* alarm_run(void* _arg);
void alarm_finished(neomoonclock_t* _neomoonclock);
