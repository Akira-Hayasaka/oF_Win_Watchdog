#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#include <tlhelp32.h>
#include <locale> 
#include <codecvt>
#include <windows.h>

class ofApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:

	void termApp(const string appName);
	void bootApp(const string path);
	bool isProcessRunning(const string procName);

	ofxOscReceiver receiver;
	bool bBooting;
	bool bManualBoot;
	float lastBootTime;
	float lastRespondTime;
	string pathToBoot;
	string nameToKill;
};
