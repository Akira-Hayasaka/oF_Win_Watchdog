# oF_Win_Watchdog #

### usage ###
#### 1.

compile watchdog

#### 2.

make watchdog folder on your app

![oF_Win_Watchdog](https://github.com/Akira-Hayasaka/oF_Win_Watchdog/raw/master/redmeimg/a.PNG)

#### 3.

copy `oF_Win_Watchdog\Watchdog\bin\*` to your watchdog folder

![oF_Win_Watchdog](https://github.com/Akira-Hayasaka/oF_Win_Watchdog/raw/master/redmeimg/c.PNG)

#### 4.

edit `watchdog\data\setting\watchdog.xml` like this

```xml
<!-- exe base name (w/o .exe) -->
<exe_name>Watchdog_example</exe_name>
<!-- watchdog <=> app com port -->
<communication_port>9000</communication_port>
<!-- watchdog <=> app polling interval for life monitoring -->
<ping_wait_interval_sec>5.0</ping_wait_interval_sec>
<!-- dont care about this. 2.0 sec is usually OK -->
<terminate_closealert_boot_interval>2.0</terminate_closealert_boot_interval>
```

#### 5.

copy `oF_Win_Watchdog\Watchdog\put_this_to_tgt_app\Watchdog_Responder.h`  on your app src

![oF_Win_Watchdog](https://github.com/Akira-Hayasaka/oF_Win_Watchdog/raw/master/redmeimg/b.PNG)

#### 6.

on your app, 

.h

```c++
#pragma once

#include "ofMain.h"
#include "Watchdog_Responder.h"

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

	unique_ptr<WatchDog_Responder> wdr;
};
```

.cpp

```c++
#include "ofApp.h"

void ofApp::setup() 
{
	wdr = make_unique<WatchDog_Responder>(true, 9000, "../../watchdog");
}
void ofApp::update() {}
void ofApp::draw() {}
void ofApp::keyPressed(int key) {}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
```





#### 7.

check watchdog_tester.maxpat

#### 8.

check Watchdog_example

