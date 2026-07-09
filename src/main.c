#include <gtk/gtk.h>

#include <stdio.h>
#include <time.h>
#include "neomoonclock.h"

static void activate(GtkApplication* _app, gpointer _data) {
	neomoonclock_t* neomoonclock = (neomoonclock_t*) _data;
	neomoonclock_init(neomoonclock);
	neomoonclock_gui_init(&neomoonclock->gui, _app);
	neomoonclock_gui_init_functionality(neomoonclock);
	gtk_widget_show_all(neomoonclock->gui.window);
}

int main(int argc, char** argv) {
	neomoonclock_t neomoonclock = { 0 };
	GtkApplication* app;
	int status;
	
	app = gtk_application_new("org.neomoonblade.neomoonclock", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK (activate), &neomoonclock);
	status = g_application_run(G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	
	return status;
}
