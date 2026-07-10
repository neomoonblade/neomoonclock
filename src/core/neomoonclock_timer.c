#include "neomoonclock_timer.h"

#include "util/notification.h"

void timer_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	gdouble hour_duration = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_neomoonclock->gui.hour_spin_button)),
			minute_duration = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_neomoonclock->gui.minute_spin_button)),
			second_duration = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_neomoonclock->gui.second_spin_button));
	
	if(hour_duration == 0 && 
		minute_duration == 0 &&
		second_duration == 0) {
		GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(_neomoonclock->gui.window),
								GTK_DIALOG_MODAL,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_OK,
								"Invalid timer arguments");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	
	
	if(_neomoonclock->timer_running) {
		_neomoonclock->timer_running = false;
		gtk_widget_set_sensitive(_neomoonclock->gui.hour_spin_button, TRUE);
		gtk_widget_set_sensitive(_neomoonclock->gui.minute_spin_button, TRUE);
		gtk_widget_set_sensitive(_neomoonclock->gui.second_spin_button, TRUE);
	} else {
		_neomoonclock->timer_running = true;
		gtk_widget_set_sensitive(_neomoonclock->gui.hour_spin_button, FALSE);
		gtk_widget_set_sensitive(_neomoonclock->gui.minute_spin_button, FALSE);
		gtk_widget_set_sensitive(_neomoonclock->gui.second_spin_button, FALSE);
		
		uint64_t timer_time_in_seconds = hour_duration * 3600 + minute_duration * 60 + second_duration;
		
		if(_neomoonclock->timer_time_in_seconds != timer_time_in_seconds) {
			_neomoonclock->timer_time_in_seconds = timer_time_in_seconds;
			_neomoonclock->timer_counter_in_seconds = timer_time_in_seconds;
		}
		
		pthread_create(&_neomoonclock->timer_thread, NULL, timer_run, _neomoonclock);
		pthread_detach(_neomoonclock->timer_thread);
	}
}

void* timer_run(void* _arg) {
	neomoonclock_t* neomoonclock = (neomoonclock_t*) _arg;
	
	struct timespec duration = {1,0}, 
					remaining;
	
	while(neomoonclock->timer_running && neomoonclock->timer_counter_in_seconds > 0) {
		neomoonclock->timer_counter_in_seconds--;
		
		timer_update_gui(neomoonclock);
		
		nanosleep(&duration, &remaining);
	}
	
	if(neomoonclock->timer_counter_in_seconds <= 0) {
		timer_finished(neomoonclock);
	}
	return NULL;
}

void timer_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->timer_counter_in_seconds = _neomoonclock->timer_time_in_seconds;
	timer_update_gui(_neomoonclock);
}

void timer_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->timer_viewing = true;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.timer_view_button, FALSE);
	
	GtkWidget* timer_view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	_neomoonclock->gui.timer_view_window = timer_view_window;
	gtk_window_set_title(GTK_WINDOW(timer_view_window), "neomoonclock timer");
	gtk_window_set_keep_above(GTK_WINDOW(timer_view_window), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(timer_view_window), 196, 100);
	gtk_window_set_gravity(GTK_WINDOW(timer_view_window), GDK_GRAVITY_NORTH_EAST);
	gtk_window_move(GTK_WINDOW(timer_view_window), 0, 0);
	gtk_window_set_transient_for(GTK_WINDOW(timer_view_window), GTK_WINDOW(_neomoonclock->gui.window));
	
	GtkWidget* timer_view_window_grid = gtk_grid_new();
	_neomoonclock->gui.timer_view_window_grid = timer_view_window_grid;
	gtk_grid_set_column_homogeneous(GTK_GRID(timer_view_window_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(timer_view_window_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(timer_view_window_grid), 5);
	
	GtkWidget* timer_view_window_time_display_label = gtk_label_new("00:00:00");
	_neomoonclock->gui.timer_view_window_time_display_label = timer_view_window_time_display_label;
	gtk_label_set_justify(GTK_LABEL(timer_view_window_time_display_label), GTK_JUSTIFY_CENTER);
	
	GtkWidget* timer_view_window_progress_bar = gtk_progress_bar_new();
	_neomoonclock->gui.timer_view_window_progress_bar = timer_view_window_progress_bar;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(timer_view_window_progress_bar), 1.0);
	
	gtk_container_add(GTK_CONTAINER(timer_view_window_grid), timer_view_window_time_display_label);
	gtk_grid_attach_next_to(GTK_GRID(timer_view_window_grid), timer_view_window_progress_bar, timer_view_window_time_display_label, GTK_POS_BOTTOM, 1, 1);
	
	gtk_container_add(GTK_CONTAINER(timer_view_window), timer_view_window_grid);
	
	g_signal_connect(timer_view_window, "destroy", G_CALLBACK(timer_view_window_close_action), _neomoonclock);
	
	gtk_widget_show_all(_neomoonclock->gui.timer_view_window);
}

void timer_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	gtk_widget_destroy(_neomoonclock->gui.timer_view_window);
	_neomoonclock->gui.timer_view_window = NULL;
	_neomoonclock->gui.timer_view_window_grid = NULL;
	_neomoonclock->gui.timer_view_window_time_display_label = NULL;
	_neomoonclock->gui.timer_view_window_progress_bar = NULL;
	_neomoonclock->timer_viewing = false;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.timer_view_button, TRUE);
}

void timer_update_gui(neomoonclock_t* _neomoonclock) {
	gdouble hour_duration = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_neomoonclock->gui.hour_spin_button)),
			minute_duration = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_neomoonclock->gui.minute_spin_button)),
			second_duration = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_neomoonclock->gui.second_spin_button));
	
	uint64_t timer_time_in_seconds = hour_duration * 3600 + minute_duration * 60 + second_duration;
	
	if(hour_duration == 0 && minute_duration == 0 && second_duration == 0) {
		// causes visual bug when updating with all 0 values
		// probably because of the division
		return;
	}
	
	if(_neomoonclock->timer_time_in_seconds != timer_time_in_seconds) {
		_neomoonclock->timer_time_in_seconds = timer_time_in_seconds;
		_neomoonclock->timer_counter_in_seconds = timer_time_in_seconds;
	}
	
	static gchar timer_label_string[9 + 14]; // added 14 to avoid the warnings
	sprintf(timer_label_string, "%02lu:%02lu:%02lu", _neomoonclock->timer_counter_in_seconds / 3600, _neomoonclock->timer_counter_in_seconds % 3600 / 60, _neomoonclock->timer_counter_in_seconds % 60);
	gdouble progress_bar_fraction = (gdouble) _neomoonclock->timer_counter_in_seconds / (gdouble) _neomoonclock->timer_time_in_seconds;
	
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(_neomoonclock->gui.timer_progress_bar), progress_bar_fraction);
	gtk_label_set_text(GTK_LABEL(_neomoonclock->gui.timer_time_display_label), timer_label_string);
	
	if(_neomoonclock->timer_viewing) {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(_neomoonclock->gui.timer_view_window_progress_bar), progress_bar_fraction);
		gtk_label_set_text(GTK_LABEL(_neomoonclock->gui.timer_view_window_time_display_label), timer_label_string);
	}
}

void timer_finished(neomoonclock_t* _neomoonclock) {
	notification_notify("neomoonclock timer", "Timer has finished");
}
