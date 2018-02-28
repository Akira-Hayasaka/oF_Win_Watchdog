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

class WatchDogResponder
{
public:

	void setup();
	void update();

private:

	void poweroff();
	void killWatchDog();
	bool isProcessRunning(const string procName);
	void bootApp(const string path);

	ofxOscSender watchDogSender;
	bool bUseWatchDog;
	float lastPingTime;
	string myExePath;
	string myExeName;
};
