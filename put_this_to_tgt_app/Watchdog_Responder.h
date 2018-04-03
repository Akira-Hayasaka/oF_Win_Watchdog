#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "Globals.h"
#include "Constants.h"

#include <tlhelp32.h>
#include <locale>
#include <codecvt>
#include <windows.h>

class WatchDog_Responder
{
public:

	void setup();
	void update();

private:

	void poweroff();
	void kill_watchdog();
	bool is_process_running(const string proc_name);
	void boot_app(const string path);

	ofxOscSender watchDogSender;
	bool b_use_watchdog;
	float last_ping_time;
	string my_exe_path, my_exe_name;
};
