#include "neomoonclock.h"

#include <stdio.h>
#include <time.h>

#include "neomoonclock_timer.h"
#include "neomoonclock_stopwatch.h"
#include "neomoonclock_alarm.h"

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
