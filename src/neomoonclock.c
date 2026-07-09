#include "neomoonclock.h"

#include <stdio.h>
#include <time.h>

#include "util/notification.h"

void neomoonclock_init(neomoonclock_t* _neomoonclock) {
	_neomoonclock->timer_running = false;
	_neomoonclock->timer_viewing = false;
	_neomoonclock->timer_time_in_seconds = 0;
	_neomoonclock->timer_counter_in_seconds = 0;
	
	_neomoonclock->gui.timer_view_window = NULL;
	_neomoonclock->gui.timer_view_window_grid = NULL;
	_neomoonclock->gui.timer_view_window_time_display_label = NULL;
	_neomoonclock->gui.timer_view_window_progress_bar = NULL;
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
	
	GtkWidget* stopwatch_grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(stopwatch_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(stopwatch_grid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(stopwatch_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(stopwatch_grid), 5);
	
	GtkWidget* stopwatch_time_label = gtk_label_new("Time: ");
	gtk_widget_set_size_request(stopwatch_time_label, 100, 30);
	
	GtkWidget* stopwatch_time_display_label = gtk_label_new("00:00:00.00");
	gtk_label_set_width_chars(GTK_LABEL(stopwatch_time_display_label), 10);
	gtk_label_set_justify(GTK_LABEL(stopwatch_time_display_label), GTK_JUSTIFY_CENTER);
	
	GtkWidget* stopwatch_start_pause_button = gtk_button_new_with_label("Start/Pause"),
			* stopwatch_reset_button = gtk_button_new_with_label("Reset"),
			* stopwatch_view_button = gtk_button_new_with_label("View");
			
	gtk_container_add(GTK_CONTAINER(stopwatch_grid), stopwatch_time_label);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_time_display_label, stopwatch_time_label, GTK_POS_RIGHT, 2, 1);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_start_pause_button, stopwatch_time_label, GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_reset_button, stopwatch_start_pause_button, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(stopwatch_grid), stopwatch_view_button, stopwatch_reset_button, GTK_POS_RIGHT, 1, 1);
	
	gtk_container_add(GTK_CONTAINER(stopwatch_frame), stopwatch_grid);
	gtk_grid_attach_next_to(GTK_GRID(window_grid), stopwatch_frame, timer_frame, GTK_POS_BOTTOM, 1, 1);
	
	// --- alarm
	GtkWidget* alarm_frame = gtk_frame_new("Alarm");
	
	GtkWidget* alarm_grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(alarm_grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(alarm_grid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(alarm_grid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(alarm_grid), 5);
	
	GtkWidget* alarm_hours_label = gtk_label_new("Hours"),
			*	alarm_minutes_label = gtk_label_new("Minutes");
	GtkWidget* alarm_hour_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 23, 1, 2, 0), 1, 0),
			*	alarm_minute_spin_button = gtk_spin_button_new(gtk_adjustment_new(0, 0, 59, 1, 2, 0), 1, 0);
	
	GtkWidget* alarm_5min_reminder_check_button = gtk_check_button_new_with_label("Remind 5 minutes before alarm");
	GtkWidget* alarm_enabled_check_button = gtk_check_button_new_with_label("Enabled");
	
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

void neomoonclock_gui_init_functionality(neomoonclock_t* _neomoonclock) {
	// --- timer
	g_signal_connect(_neomoonclock->gui.timer_start_pause_button, "clicked", G_CALLBACK(timer_start_pause_button_action), _neomoonclock);
	g_signal_connect(_neomoonclock->gui.timer_reset_button, "clicked", G_CALLBACK(timer_reset_button_action), _neomoonclock);
	g_signal_connect(_neomoonclock->gui.timer_view_button, "clicked", G_CALLBACK(timer_view_button_action), _neomoonclock);
}

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
	
	timer_finished(neomoonclock);
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
	gtk_window_set_transient_for(GTK_WINDOW(timer_view_window), _neomoonclock->gui.window);
	
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
	
	gtk_widget_set_sensitive(_neomoonclock->gui.timer_view_button, TRUE);
}

static void timer_update_gui(neomoonclock_t* _neomoonclock) {
	static gchar timer_label_string[9];
	sprintf(timer_label_string, "%02ld:%02ld:%02ld", _neomoonclock->timer_counter_in_seconds / 3600, _neomoonclock->timer_counter_in_seconds / 60, _neomoonclock->timer_counter_in_seconds % 60);
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
