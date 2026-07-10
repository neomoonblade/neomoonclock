#include "neomoonclock.h"

#include <stdio.h>
#include <time.h>

#include "util/notification.h"
#include "util/timeutil.h"

void neomoonclock_init(neomoonclock_t* _neomoonclock) {
	_neomoonclock->timer_running = false;
	_neomoonclock->timer_viewing = false;
	_neomoonclock->timer_time_in_seconds = 0;
	_neomoonclock->timer_counter_in_seconds = 0;
	
	_neomoonclock->gui.timer_view_window = NULL;
	_neomoonclock->gui.timer_view_window_grid = NULL;
	_neomoonclock->gui.timer_view_window_time_display_label = NULL;
	_neomoonclock->gui.timer_view_window_progress_bar = NULL;
	
	_neomoonclock->stopwatch_running = false;
	_neomoonclock->stopwatch_viewing = false;
	_neomoonclock->stopwatch_start_time_ns = 0;
	_neomoonclock->stopwatch_paused_time_ns = 0;
	_neomoonclock->stopwatch_elapsed_time_ns = 0;
	
	_neomoonclock->alarm_running = false;
	_neomoonclock->alarm_5min_reminded = false;
}

void neomoonclock_gui_init(neomoonclock_gui_t* _neomoonclock_gui, GtkApplication* _app) {
	_neomoonclock_gui->app = _app;
	_neomoonclock_gui->window = gtk_application_window_new(_app);
	//neomoonclock->date_time = gtk_date_time_new();
	
	gtk_window_set_title(GTK_WINDOW (_neomoonclock_gui->window), "neomoonclock");
	gtk_window_set_default_size(GTK_WINDOW (_neomoonclock_gui->window), 480, 200);
	
	GtkWidget* window_grid = gtk_grid_new();
	_neomoonclock_gui->window_grid = window_grid;
	gtk_grid_set_column_homogeneous(GTK_GRID(window_grid), TRUE);
	gtk_grid_set_row_spacing(GTK_GRID(window_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(window_grid), 5);
	
	// --- timer
	GtkWidget* timer_frame = gtk_frame_new("Timer");
	_neomoonclock_gui->timer_frame = timer_frame;
	
	GtkWidget* timer_grid = gtk_grid_new();
	_neomoonclock_gui->timer_grid = timer_grid;
	gtk_grid_set_column_homogeneous(GTK_GRID(timer_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(timer_grid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(timer_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(timer_grid), 5);
	
	GtkWidget* hours_label = gtk_label_new("Hours"),
			*	minutes_label = gtk_label_new("Minutes"),
			*	seconds_label = gtk_label_new("Seconds");
	_neomoonclock_gui->hours_label = hours_label;
	_neomoonclock_gui->minutes_label = minutes_label;
	_neomoonclock_gui->seconds_label = seconds_label;
	GtkWidget* hour_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 23, 1, 2, 0), 1, 0),
			*	minute_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 59, 1, 2, 0), 1, 0),
			*	second_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 59, 1, 2, 0), 1, 0);
	_neomoonclock_gui->hour_spin_button = hour_spin_button;
	_neomoonclock_gui->minute_spin_button = minute_spin_button;
	_neomoonclock_gui->second_spin_button = second_spin_button;
	
	GtkWidget* timer_progress_bar = gtk_progress_bar_new();
	_neomoonclock_gui->timer_progress_bar = timer_progress_bar;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(timer_progress_bar), 1.0);
	
	GtkWidget* timer_time_display_label = gtk_label_new("00:00:00");
	_neomoonclock_gui->timer_time_display_label = timer_time_display_label;
	gtk_label_set_justify(GTK_LABEL(timer_time_display_label), GTK_JUSTIFY_CENTER);
	
	GtkWidget* timer_start_pause_button = gtk_button_new_with_label("Start/Pause"),
			* timer_reset_button = gtk_button_new_with_label("Reset"),
			* timer_view_button = gtk_button_new_with_label("View");
	_neomoonclock_gui->timer_start_pause_button = timer_start_pause_button;
	_neomoonclock_gui->timer_reset_button = timer_reset_button;
	_neomoonclock_gui->timer_view_button = timer_view_button;
			
	gtk_container_add(GTK_CONTAINER(timer_grid), hours_label);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), minutes_label, hours_label, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), seconds_label, minutes_label, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), hour_spin_button, hours_label, GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), minute_spin_button, hour_spin_button, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), second_spin_button, minute_spin_button, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), timer_progress_bar, hour_spin_button, GTK_POS_BOTTOM, 3, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), timer_time_display_label, timer_progress_bar, GTK_POS_BOTTOM, 3, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), timer_start_pause_button, timer_time_display_label, GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), timer_reset_button, timer_start_pause_button, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(timer_grid), timer_view_button, timer_reset_button, GTK_POS_RIGHT, 1, 1);
	
	gtk_container_add(GTK_CONTAINER(timer_frame), timer_grid);
	gtk_container_add(GTK_CONTAINER(window_grid), timer_frame);
	
	// --- stopwatch
	GtkWidget* stopwatch_frame = gtk_frame_new("Stopwatch");
	_neomoonclock_gui->stopwatch_frame = stopwatch_frame;
	
	GtkWidget* stopwatch_grid = gtk_grid_new();
	_neomoonclock_gui->stopwatch_grid = stopwatch_grid;
	gtk_grid_set_column_homogeneous(GTK_GRID(stopwatch_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(stopwatch_grid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(stopwatch_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(stopwatch_grid), 5);
	
	GtkWidget* stopwatch_time_label = gtk_label_new("Time: ");
	_neomoonclock_gui->stopwatch_time_label = stopwatch_time_label;
	gtk_widget_set_size_request(stopwatch_time_label, 100, 30);
	
	GtkWidget* stopwatch_time_display_label = gtk_label_new("00:00:00.000");
	_neomoonclock_gui->stopwatch_time_display_label = stopwatch_time_display_label;
	gtk_label_set_width_chars(GTK_LABEL(stopwatch_time_display_label), 10);
	gtk_label_set_justify(GTK_LABEL(stopwatch_time_display_label), GTK_JUSTIFY_CENTER);
	
	GtkWidget* stopwatch_start_pause_button = gtk_button_new_with_label("Start/Pause"),
			* stopwatch_reset_button = gtk_button_new_with_label("Reset"),
			* stopwatch_view_button = gtk_button_new_with_label("View");
	_neomoonclock_gui->stopwatch_start_pause_button = stopwatch_start_pause_button;
	_neomoonclock_gui->stopwatch_reset_button = stopwatch_reset_button;
	_neomoonclock_gui->stopwatch_view_button = stopwatch_view_button;
	
	gtk_container_add(GTK_CONTAINER(stopwatch_grid), stopwatch_time_label);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_time_display_label, stopwatch_time_label, GTK_POS_RIGHT, 2, 1);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_start_pause_button, stopwatch_time_label, GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_reset_button, stopwatch_start_pause_button, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_view_button, stopwatch_reset_button, GTK_POS_RIGHT, 1, 1);
	
	gtk_container_add(GTK_CONTAINER(stopwatch_frame), stopwatch_grid);
	gtk_grid_attach_next_to(GTK_GRID(window_grid), stopwatch_frame, timer_frame, GTK_POS_BOTTOM, 1, 1);
	
	// --- alarm
	GtkWidget* alarm_frame = gtk_frame_new("Alarm");
	_neomoonclock_gui->alarm_frame = alarm_frame;
	
	GtkWidget* alarm_grid = gtk_grid_new();
	_neomoonclock_gui->alarm_grid = alarm_grid;
	gtk_grid_set_column_homogeneous(GTK_GRID(alarm_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(alarm_grid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(alarm_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(alarm_grid), 5);
	
	GtkWidget* alarm_hours_label = gtk_label_new("Hours"),
			*	alarm_minutes_label = gtk_label_new("Minutes");
	_neomoonclock_gui->alarm_hours_label = alarm_hours_label;
	_neomoonclock_gui->alarm_minutes_label = alarm_minutes_label;
	
	GtkWidget* alarm_hour_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 23, 1, 2, 0), 1, 0),
			*	alarm_minute_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 59, 1, 2, 0), 1, 0);
	_neomoonclock_gui->alarm_hour_spin_button = alarm_hour_spin_button;
	_neomoonclock_gui->alarm_minute_spin_button = alarm_minute_spin_button;
	
	GtkWidget* alarm_5min_reminder_check_button = gtk_check_button_new_with_label("Remind 5 minutes before alarm");
	_neomoonclock_gui->alarm_5min_reminder_check_button = alarm_5min_reminder_check_button;
	
	GtkWidget* alarm_enabled_check_button = gtk_check_button_new_with_label("Enabled");
	_neomoonclock_gui->alarm_enabled_check_button = alarm_enabled_check_button;
	
	gtk_container_add(GTK_CONTAINER(alarm_grid), alarm_hours_label);
	gtk_grid_attach_next_to(GTK_GRID(alarm_grid), alarm_minutes_label, alarm_hours_label, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(alarm_grid), alarm_hour_spin_button, alarm_hours_label, GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(alarm_grid), alarm_minute_spin_button, alarm_hour_spin_button, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(alarm_grid), alarm_5min_reminder_check_button, alarm_hour_spin_button, GTK_POS_BOTTOM, 3, 1);
	gtk_grid_attach_next_to(GTK_GRID(alarm_grid), alarm_enabled_check_button, alarm_5min_reminder_check_button, GTK_POS_BOTTOM, 3, 1);
	
	gtk_container_add(GTK_CONTAINER(alarm_frame), alarm_grid);
	gtk_grid_attach_next_to(GTK_GRID(window_grid), alarm_frame, stopwatch_frame, GTK_POS_BOTTOM, 1, 1);
	
	// --- window
	gtk_container_add(GTK_CONTAINER(_neomoonclock_gui->window), window_grid);
}

static void timer_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void* timer_run(void* _arg);
static void timer_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void timer_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void timer_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void timer_update_gui(neomoonclock_t* _neomoonclock);
static void timer_finished(neomoonclock_t* _neomoonclock);

static void stopwatch_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void* stopwatch_run(void* _arg);
static void stopwatch_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void stopwatch_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void stopwatch_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void stopwatch_update_gui(neomoonclock_t* _neomoonclock);

static void alarm_enabled_check_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock);
static void* alarm_run(void* _arg);

void neomoonclock_gui_init_functionality(neomoonclock_t* _neomoonclock) {
	// --- timer
	g_signal_connect(_neomoonclock->gui.timer_start_pause_button, "clicked", G_CALLBACK(timer_start_pause_button_action), _neomoonclock);
	g_signal_connect(_neomoonclock->gui.timer_reset_button, "clicked", G_CALLBACK(timer_reset_button_action), _neomoonclock);
	g_signal_connect(_neomoonclock->gui.timer_view_button, "clicked", G_CALLBACK(timer_view_button_action), _neomoonclock);
	
	// --- stopwatch
	g_signal_connect(_neomoonclock->gui.stopwatch_start_pause_button, "clicked", G_CALLBACK(stopwatch_start_pause_button_action), _neomoonclock);
	g_signal_connect(_neomoonclock->gui.stopwatch_reset_button, "clicked", G_CALLBACK(stopwatch_reset_button_action), _neomoonclock);
	g_signal_connect(_neomoonclock->gui.stopwatch_view_button, "clicked", G_CALLBACK(stopwatch_view_button_action), _neomoonclock);
	
	// --- alarm
	g_signal_connect(_neomoonclock->gui.alarm_enabled_check_button, "toggled", G_CALLBACK(alarm_enabled_check_button_action), _neomoonclock);
}

// --- timer

static void timer_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
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

static void* timer_run(void* _arg) {
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

static void timer_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->timer_counter_in_seconds = _neomoonclock->timer_time_in_seconds;
	timer_update_gui(_neomoonclock);
}

static void timer_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
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

static void timer_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	gtk_widget_destroy(_neomoonclock->gui.timer_view_window);
	_neomoonclock->gui.timer_view_window = NULL;
	_neomoonclock->gui.timer_view_window_grid = NULL;
	_neomoonclock->gui.timer_view_window_time_display_label = NULL;
	_neomoonclock->gui.timer_view_window_progress_bar = NULL;
	_neomoonclock->timer_viewing = false;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.timer_view_button, TRUE);
}

static void timer_update_gui(neomoonclock_t* _neomoonclock) {
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

static void timer_finished(neomoonclock_t* _neomoonclock) {
	notification_notify("neomoonclock timer", "Timer has finished");
}

// --- stopwatch

static void stopwatch_start_pause_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
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

static void* stopwatch_run(void* _arg) {
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

static void stopwatch_reset_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->stopwatch_paused_time_ns = 0;
	_neomoonclock->stopwatch_elapsed_time_ns = 0;
	
	stopwatch_update_gui(_neomoonclock);
}

static void stopwatch_view_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	_neomoonclock->stopwatch_viewing = true;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.stopwatch_view_button, FALSE);
	
	GtkWidget* stopwatch_view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	_neomoonclock->gui.stopwatch_view_window = stopwatch_view_window;
	gtk_window_set_title(GTK_WINDOW(stopwatch_view_window), "neomoonclock stopwatch");
	gtk_window_set_keep_above(GTK_WINDOW(stopwatch_view_window), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(stopwatch_view_window), 196, 100);
	gtk_window_set_gravity(GTK_WINDOW(stopwatch_view_window), GDK_GRAVITY_NORTH_EAST);
	gtk_window_move(GTK_WINDOW(stopwatch_view_window), 0, 0);
	gtk_window_set_transient_for(GTK_WINDOW(stopwatch_view_window), GTK_WINDOW(_neomoonclock->gui.window));
	
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

static void stopwatch_view_window_close_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
	gtk_widget_destroy(_neomoonclock->gui.stopwatch_view_window);
	_neomoonclock->gui.stopwatch_view_window = NULL;
	_neomoonclock->gui.stopwatch_view_window_grid = NULL;
	_neomoonclock->gui.stopwatch_view_window_time_display_label = NULL;
	_neomoonclock->gui.stopwatch_view_window_start_pause_button = NULL;
	_neomoonclock->stopwatch_viewing = false;
	
	gtk_widget_set_sensitive(_neomoonclock->gui.stopwatch_view_button, TRUE);
}

static void stopwatch_update_gui(neomoonclock_t* _neomoonclock) {
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

// --- alarm

static void alarm_enabled_check_button_action(GtkWidget* _widget, neomoonclock_t* _neomoonclock) {
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

static void* alarm_run(void* _arg) {
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

