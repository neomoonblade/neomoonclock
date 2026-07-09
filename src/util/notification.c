#include "notification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void notification_notify(const char* _title, const char* _message) {
	size_t title_length = strlen(_title);
	size_t message_length = strlen(_message);
	char command[19 + title_length + message_length];
	sprintf(command, "notify-send \"%s\" \"%s\"", _title, _message);
	system(command);
}

