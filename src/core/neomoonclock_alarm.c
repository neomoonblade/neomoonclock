#include "neomoonclock_alarm.h"

#include "util/notification.h"

void alarm_enabled_check_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(_neomoonclock->gui.alarm_enabled_check_button))) {
		gtk_widget_set_sensitive(_neomoonclock->gui.alarm_hour_spin_button, FALSE);
		gtk_widget_set_sensitive(_neomoonclock->gui.alarm_minute_spin_button, FALSE);
		
		_neomoonclock->alarm_running = true;
		_neomoonclock->alarm_5min_reminded = false;
		
		pthread_create(&_neomoonclock->alarm_thread, NULL, alarm_run, _neomoonclock);
		pthread_detach(_neomoonclock->alarm_thread);
	} else {
		_neomoonclock->alarm_running = false;
		gtk_widget_set_sensitive(_neomoonclock->gui.alarm_hour_spin_button, TRUE);
		gtk_widget_set_sensitive(_neomoonclock->gui.alarm_minute_spin_button, TRUE);
	}
}

void* alarm_run(void* _arg) {
	neomoonclock_t* neomoonclock = (neomoonclock_t*) _arg;
	
	struct timespec duration = {1, 0}, 
					remaining;
	
	int alarm_hour = gtk_spin_button_get_value(GTK_SPIN_BUTTON(neomoonclock->gui.alarm_hour_spin_button));
	int alarm_minute = gtk_spin_button_get_value(GTK_SPIN_BUTTON(neomoonclock->gui.alarm_minute_spin_button));
	int alarm_total_minute = alarm_hour * 60 + alarm_minute;
	
	while(neomoonclock->alarm_running) {
		time_t current_time = time(NULL);
		struct tm* current_local_time = localtime(&current_time);
		
		int current_hour =  current_local_time->tm_hour;
		int current_minute = current_local_time->tm_min;
		int current_total_minute = current_hour * 60 + current_minute;
		
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(neomoonclock->gui.alarm_5min_reminder_check_button)) &&
			current_total_minute == alarm_total_minute - 5 &&
			neomoonclock->alarm_5min_reminded == false) {
			neomoonclock->alarm_5min_reminded = true;
			notification_notify("neomoonclock alarm", "Heads up! It's 5 minutes before the alarm rings");
		}
		
		if(alarm_total_minute == current_total_minute) {
			notification_notify("neomoonclock alarm", "Alarm!");
			neomoonclock->alarm_running = false;
			gtk_widget_set_sensitive(neomoonclock->gui.alarm_hour_spin_button, TRUE);
			gtk_widget_set_sensitive(neomoonclock->gui.alarm_minute_spin_button, TRUE);
		}
		
		nanosleep(&duration, &remaining);
	}
	
	return NULL;
}

