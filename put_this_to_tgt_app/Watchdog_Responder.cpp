#include "WatchDog_Responder.h"

void WatchDog_Responder::setup()
{
	b_use_watchdog = false;
	last_ping_time = Globals::ELAPSED_TIME;

	watchDogSender.setup("localhost", WATCHDOG_PORT);

	ofxXmlSettings xml;
	xml.load("package.xml");
	b_use_watchdog = (xml.getValue("useWatchDog", "") == "true") ? true : false;
	string watchDogPath = xml.getValue("watchDogPath", "");
	xml.clear();

	if (b_use_watchdog && !is_process_running("Watchdog.exe"))
	{
		ofDirectory dir;
		dir.open(watchDogPath);
		dir.listDir();
		vector<ofFile> files = dir.getFiles();
		dir.close();

		for (auto f : files)
		{
			if (f.getExtension() == "exe")
			{
				string bootPath = f.getAbsolutePath();
				ofStringReplace(bootPath, "\\", "\\\\");
				ofStringReplace(bootPath, "/", "\\\\");

				// boot watchdog
				boot_app(bootPath);
				break;
			}
		}
	}

	// my exe path
	ofDirectory dir;
	dir.open("../");
	dir.listDir();
	vector<ofFile> files = dir.getFiles();
	dir.close();

	for (auto f : files)
	{
		if (f.getExtension() == "exe")
		{
			my_exe_name = f.getFileName();
			my_exe_path = f.getAbsolutePath();
			ofStringReplace(my_exe_path, "\\", "\\\\");
			ofStringReplace(my_exe_path, "/", "\\\\");
			break;
		}
	}

	//ofAddListener(Globals::kill_watchdogEvent, this, &WatchDog_Responder::kill_watchdog);
	//ofAddListener(Globals::poweroff_event, this, &WatchDog_Responder::poweroff);
}

void WatchDog_Responder::update()
{
	// ping to watchDog
	if (b_use_watchdog)
	{
		if (Globals::ELAPSED_TIME - last_ping_time > 0.5)
		{
			ofxOscMessage m;
			m.setAddress("/imok");
			m.addStringArg(my_exe_path);
			m.addStringArg(my_exe_name);
			watchDogSender.sendMessage(m, false);
			last_ping_time = Globals::ELAPSED_TIME;
		}
	}
}

void WatchDog_Responder::poweroff()
{
	ofxOscMessage m;
	m.setAddress("/poweroff");
	watchDogSender.sendMessage(m, false);
	b_use_watchdog = false;
	ofLog() << "poweroff";
}

void WatchDog_Responder::kill_watchdog()
{
	ofxOscMessage m;
	m.setAddress("/killyou");
	watchDogSender.sendMessage(m, false);
	b_use_watchdog = false;
	ofLog() << "kill_watchdog";
}

bool WatchDog_Responder::is_process_running(const string proc_name)
{
	bool b_found = false;

	wstring_convert<codecvt_utf8<wchar_t>, wchar_t> cv;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			wstring w_proc_name = cv.from_bytes(proc_name);
			if (wcscmp(entry.szExeFile, w_proc_name.c_str()) == 0)
			{
				b_found = true;
				break;
			}
		}
	}

	CloseHandle(snapshot);

	return b_found;
}

void WatchDog_Responder::boot_app(const string path)
{
	wstring_convert<codecvt_utf8<wchar_t>, wchar_t> cv;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	wstring wpath = cv.from_bytes(path);

	CreateProcessW(
		NULL,
		&wpath[0],
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi);

	// Close process and thread handles.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
