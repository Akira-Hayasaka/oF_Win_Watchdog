#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#include <tlhelp32.h>
#include <locale>
#include <codecvt>
#include <windows.h>

class WatchDog_Responder
{
public:

	WatchDog_Responder(const bool _b_use_watchdog, const int _watchdog_port, const string _watchdog_app_path)
	{
		b_use_watchdog = _b_use_watchdog;
		last_ping_time = ofGetElapsedTimef();
		watchdog_sender.setup("localhost", _watchdog_port);

		if (b_use_watchdog && !is_process_running("Watchdog.exe"))
		{
			ofDirectory dir;
			dir.open(_watchdog_app_path);
			dir.listDir();
			auto files = dir.getFiles();
			dir.close();

			for (auto f : files)
			{
				if (f.getExtension() == "exe")
				{
					auto boot_path = f.getAbsolutePath();
					ofStringReplace(boot_path, "\\", "\\\\");
					ofStringReplace(boot_path, "/", "\\\\");

					// boot watchdog
					boot_watchdog(boot_path);
					break;
				}
			}
		}

		// my exe path
		ofDirectory dir;
		dir.open("../");
		dir.listDir();
		auto files = dir.getFiles();
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

		ofAddListener(ofEvents().update, this, &WatchDog_Responder::update);
	}

	void update(ofEventArgs &args)
	{
		// ping to watchDog
		if (b_use_watchdog)
		{
			if (ofGetElapsedTimef() - last_ping_time > 0.5)
			{
				ofxOscMessage m;
				m.setAddress("/imok");
				m.addStringArg(my_exe_path);
				m.addStringArg(my_exe_name);
				watchdog_sender.sendMessage(m, false);
				last_ping_time = ofGetElapsedTimef();
			}
		}
	}

	void poweroff()
	{
		ofxOscMessage m;
		m.setAddress("/poweroff");
		watchdog_sender.sendMessage(m, false);
		b_use_watchdog = false;
		ofLog() << "poweroff";
	}

	void kill_watchdog()
	{
		ofxOscMessage m;
		m.setAddress("/killyou");
		watchdog_sender.sendMessage(m, false);
		b_use_watchdog = false;
		ofLog() << "kill_watchdog";
	}

private:

	void boot_watchdog(const string path)
	{
		wstring_convert<codecvt_utf8<wchar_t>, wchar_t> cv;
		STARTUPINFOW si;
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

	bool is_process_running(const string proc_name)
	{
		bool b_found = false;

		wstring_convert<codecvt_utf8<wchar_t>, wchar_t> cv;
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32FirstW(snapshot, &entry) == TRUE)
		{
			while (Process32NextW(snapshot, &entry) == TRUE)
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

	ofxOscSender watchdog_sender;
	bool b_use_watchdog;
	float last_ping_time;
	string my_exe_path, my_exe_name;
};
