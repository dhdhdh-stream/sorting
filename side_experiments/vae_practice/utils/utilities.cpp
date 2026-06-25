#include "utilities.h"

#include <ctime>
#include <iostream>

using namespace std;

char* get_time() {
	time_t timestamp = time(NULL);
	struct tm datetime = *localtime(&timestamp);

	static const char wday_name[][4] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static const char mon_name[][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char result[25];
	sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
		wday_name[datetime.tm_wday],
		mon_name[datetime.tm_mon],
		datetime.tm_mday, datetime.tm_hour,
		datetime.tm_min, datetime.tm_sec,
		1900 + datetime.tm_year);
	return result;
}
