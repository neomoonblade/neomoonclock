#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <gtk/gtk.h>
#include <pthread.h>

typedef struct neomoonclock_gui {
	GtkApplication* app;
	GtkWidget* window;
	GtkWidget* window_grid;
	// --- timer components
	GtkWidget* timer_frame;
	GtkWidget* timer_grid;
	GtkWidget* hours_label,
			*	minutes_label,
			*	seconds_label;
	GtkWidget* hour_spin_button,
			*	minute_spin_button,
			*	second_spin_button;
	GtkWidget* timer_progress_bar;
	GtkWidget* timer_time_display_label;
	GtkWidget* timer_start_pause_button,
			* timer_reset_button,
			* timer_view_button;
	GtkWidget* timer_view_window;
	GtkWidget* timer_view_window_grid;
	GtkWidget* timer_view_window_time_display_label;
	GtkWidget* timer_view_window_progress_bar;
	// -- stopwatch components
	GtkWidget* stopwatch_frame;
	GtkWidget* stopwatch_grid;
	GtkWidget* stopwatch_time_label;
	GtkWidget* stopwatch_time_display_label;
	GtkWidget* stopwatch_start_pause_button,
			* stopwatch_reset_button,
			* stopwatch_view_button;
	GtkWidget* stopwatch_view_window;
	GtkWidget* stopwatch_view_window_grid;
	GtkWidget* stopwatch_view_window_time_display_label;
	GtkWidget* stopwatch_view_window_start_pause_button;
	// --- alarm components
	GtkWidget* alarm_frame;
	GtkWidget* alarm_grid;
	GtkWidget* alarm_hours_label,
			*	alarm_minutes_label;
	GtkWidget* alarm_hour_spin_button,
			*	alarm_minute_spin_button;
	
	GtkWidget* alarm_5min_reminder_check_button;
	GtkWidget* alarm_enabled_check_button;
	
} neomoonclock_gui_t;

typedef struct neomoonclock {
	neomoonclock_gui_t gui;
	
	uint64_t timer_time_in_seconds,
			timer_counter_in_seconds;
	bool timer_running;
	bool timer_viewing;
	pthread_t timer_thread;
	
	bool stopwatch_running;
	bool stopwatch_viewing;
	uint64_t stopwatch_start_time_ns;
	uint64_t stopwatch_paused_time_ns;
	uint64_t stopwatch_elapsed_time_ns;
	pthread_t stopwatch_thread;
} neomoonclock_t;

void neomoonclock_init(neomoonclock_t* _neomoonclock);
void neomoonclock_gui_init(neomoonclock_gui_t* _neomoonclock_gui, GtkApplication* _app);
void neomoonclock_gui_init_functionality(neomoonclock_t* _neomoonclock);

