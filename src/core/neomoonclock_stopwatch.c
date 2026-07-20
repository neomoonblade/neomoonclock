#include "neomoonclock_stopwatch.h"

#include "util/timeutil.h"

void stopwatch_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->stopwatch_running = !_neomoonclock->stopwatch_running;
	
	if(_neomoonclock->stopwatch_running) {
		gtk_widget_set_sensitive(_neomoonclock->gui.stopwatch_reset_button, FALSE);
		
		if(time_get_current_nanotime(&_neomoonclock->stopwatch_start_time_ns) == false) {
			GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(_neomoonclock->gui.window),
								GTK_DIALOG_MODAL,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_OK,
								"An unexpected error occured when starting the timer");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return;
		}
		
		pthread_create(&_neomoonclock->stopwatch_thread, NULL, stopwatch_run, _neomoonclock);
		pthread_detach(_neomoonclock->stopwatch_thread);
	} else {
		gtk_widget_set_sensitive(_neomoonclock->gui.stopwatch_reset_button, TRUE);
	}
}

void* stopwatch_run(void* _arg) {
	neomoonclock_t* neomoonclock = (neomoonclock_t*) _arg;
	
	struct timespec duration = {0, 50000000ULL}, 
					remaining;
	
	uint64_t current_time_ns;
	uint64_t elapsed_time_ns;
	
	while(neomoonclock->stopwatch_running) {
		if(time_get_current_nanotime(&current_time_ns) == false) {
			continue;
		}
		
		elapsed_time_ns = current_time_ns - neomoonclock->stopwatch_start_time_ns + neomoonclock->stopwatch_paused_time_ns;
		neomoonclock->stopwatch_elapsed_time_ns = elapsed_time_ns;
		
		stopwatch_update_gui(neomoonclock);
		
		nanosleep(&duration, &remaining);
	}
	
	neomoonclock->stopwatch_paused_time_ns = elapsed_time_ns;
	
	return NULL;
}

void stopwatch_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->stopwatch_paused_time_ns = 0;
	_neomoonclock->stopwatch_elapsed_time_ns = 0;
	
	stopwatch_update_gui(_neomoonclock);
}

void stopwatch_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->stopwatch_viewing = true;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.stopwatch_view_button, FALSE);
	
	GtkWidget* stopwatch_view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	_neomoonclock->gui.stopwatch_view_window = stopwatch_view_window;
	gtk_window_set_title(GTK_WINDOW(stopwatch_view_window), "neomoonclock stopwatch");
	gtk_window_set_keep_above(GTK_WINDOW(stopwatch_view_window), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(stopwatch_view_window), 196, 100);
	gtk_window_set_gravity(GTK_WINDOW(stopwatch_view_window), GDK_GRAVITY_NORTH_EAST);
	gtk_window_move(GTK_WINDOW(stopwatch_view_window), 0, 0);
	
	GtkWidget* stopwatch_view_window_grid = gtk_grid_new();
	_neomoonclock->gui.stopwatch_view_window_grid = stopwatch_view_window_grid;
	gtk_grid_set_column_homogeneous(GTK_GRID(stopwatch_view_window_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(stopwatch_view_window_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(stopwatch_view_window_grid), 5);
	
	GtkWidget* stopwatch_view_window_time_display_label = gtk_label_new("00:00:00.000");
	_neomoonclock->gui.stopwatch_view_window_time_display_label = stopwatch_view_window_time_display_label;
	gtk_label_set_justify(GTK_LABEL(stopwatch_view_window_time_display_label), GTK_JUSTIFY_CENTER);
	
	GtkWidget* stopwatch_view_window_start_pause_button = gtk_button_new_with_label("Start/Pause");
	_neomoonclock->gui.stopwatch_view_window_start_pause_button = stopwatch_view_window_start_pause_button;
	g_signal_connect(stopwatch_view_window_start_pause_button, "clicked", G_CALLBACK(stopwatch_start_pause_button_action), _neomoonclock);
	
	gtk_container_add(GTK_CONTAINER(stopwatch_view_window_grid), stopwatch_view_window_time_display_label);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_view_window_grid), stopwatch_view_window_start_pause_button, stopwatch_view_window_time_display_label, GTK_POS_BOTTOM, 1, 1);
	
	gtk_container_add(GTK_CONTAINER(stopwatch_view_window), stopwatch_view_window_grid);
	
	g_signal_connect(stopwatch_view_window, "destroy", G_CALLBACK(stopwatch_view_window_close_action), _neomoonclock);
	
	gtk_widget_show_all(_neomoonclock->gui.stopwatch_view_window);
	stopwatch_update_gui(_neomoonclock);
}

void stopwatch_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	gtk_widget_destroy(_neomoonclock->gui.stopwatch_view_window);
	_neomoonclock->gui.stopwatch_view_window = NULL;
	_neomoonclock->gui.stopwatch_view_window_grid = NULL;
	_neomoonclock->gui.stopwatch_view_window_time_display_label = NULL;
	_neomoonclock->gui.stopwatch_view_window_start_pause_button = NULL;
	_neomoonclock->stopwatch_viewing = false;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.stopwatch_view_button, TRUE);
}

void stopwatch_update_gui(neomoonclock_t* _neomoonclock) {
	static char time_display_label_string[13];
	
	uint64_t elapsed_time_ms = _neomoonclock->stopwatch_elapsed_time_ns / 1000000ULL,
			elapsed_time_s = _neomoonclock->stopwatch_elapsed_time_ns / 1000000000ULL,
			elapsed_time_min = _neomoonclock->stopwatch_elapsed_time_ns / 60000000000ULL,
			elapsed_time_hr = _neomoonclock->stopwatch_elapsed_time_ns / 3600000000000ULL;
	
	sprintf(time_display_label_string, "%02lu:%02lu:%02lu.%03lu", elapsed_time_hr % 100, elapsed_time_min % 100, elapsed_time_s % 100, elapsed_time_ms % 1000);
	gtk_label_set_text(GTK_LABEL(_neomoonclock->gui.stopwatch_time_display_label), time_display_label_string);
	
	if(_neomoonclock->stopwatch_viewing) {
		gtk_label_set_text(GTK_LABEL(_neomoonclock->gui.stopwatch_view_window_time_display_label), time_display_label_string);
	}
}