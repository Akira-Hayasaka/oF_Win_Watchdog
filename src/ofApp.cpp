#include "ofApp.h"

void ofApp::setup() 
{
	ofBackground(ofColor::black);
	ofSetFrameRate(30);
	ofLogToFile("log/watchdog.log");

	ofxXmlSettings xml;
	xml.load("setting/watchdog.xml");
	exe_name = xml.getValue("exe_name", "");
	receiver.setup(xml.getValue("communication_port", 0));
	ping_wait_interval_sec = xml.getValue("ping_wait_interval_sec", 1.0);
	terminate_closealert_boot_interval = xml.getValue("terminate_closealert_boot_interval", 1.0);
	xml.clear();
	last_respond_time = ofGetElapsedTimef();
	last_boot_time = ofGetElapsedTimef();
	b_booting = false;
	b_manual_boot = false;
	ofSetWindowPosition(0, 0);
}

void ofApp::update() 
{
	HWND AppWindow = GetActiveWindow();
	SetWindowPos(AppWindow, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	while (receiver.hasWaitingMessages())
	{
		ofxOscMessage m;
		receiver.getNextMessage(&m);

		if (m.getAddress() == "/imok")
		{
			path_to_boot = m.getArgAsString(0);
			name_to_kill = m.getArgAsString(1);
			last_respond_time = ofGetElapsedTimef();
			b_booting = false;
		}

		// someone wants me to die
		if (m.getAddress() == "/killyou")
		{
			ofLogNotice(ofGetTimestampString("%Y.%m.%d.%H:%M.%S")) << "killyou";
			ofExit();
		}

		if (m.getAddress() == "/killapp")
		{
			ofLogNotice(ofGetTimestampString("%Y.%m.%d.%H:%M.%S")) << "/killapp";
			terminate_app(name_to_kill);
			b_manual_boot = true;
		}

		if (m.getAddress() == "/startapp")
		{
			ofLogNotice(ofGetTimestampString("%Y.%m.%d.%H:%M.%S")) << "/startapp";
			boot_app(path_to_boot);
			b_manual_boot = false;
			last_respond_time = ofGetElapsedTimef(); // reset 
			b_booting = true;
		}
		if (m.getAddress() == "/reboot")
		{
			ofLogNotice(ofGetTimestampString("%Y.%m.%d.%H:%M.%S")) << "force reboot machine";

			terminate_app(name_to_kill);

			HANDLE hToken;
			TOKEN_PRIVILEGES TokenPri;

			if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == FALSE)
				ofLogFatalError("reboot machine") << "fail to open process token";

			if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &TokenPri.Privileges[0].Luid) == FALSE)
				ofLogFatalError("reboot machine") << "fail to get LUID";

			TokenPri.PrivilegeCount = 1;
			TokenPri.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &TokenPri, 0, (PTOKEN_PRIVILEGES)NULL, 0);
			if (GetLastError() != ERROR_SUCCESS)
				ofLogFatalError("reboot machine") << "fail to authorize reboot";

			ExitWindowsEx(EWX_REBOOT | EWX_FORCE | EWX_FORCEIFHUNG, 0);
			ofExit();
		}
		if (m.getAddress() == "/poweroff")
		{
			ofLogNotice(ofGetTimestampString("%Y.%m.%d.%H:%M.%S")) << "force reboot machine";

			terminate_app(name_to_kill);

			HANDLE hToken;
			TOKEN_PRIVILEGES TokenPri;

			if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == FALSE)
				ofLogFatalError("reboot machine") << "fail to open process token";

			if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &TokenPri.Privileges[0].Luid) == FALSE)
				ofLogFatalError("reboot machine") << "fail to get LUID";

			TokenPri.PrivilegeCount = 1;
			TokenPri.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &TokenPri, 0, (PTOKEN_PRIVILEGES)NULL, 0);
			if (GetLastError() != ERROR_SUCCESS)
				ofLogFatalError("reboot machine") << "fail to authorize reboot";

			ExitWindowsEx(EWX_POWEROFF | EWX_FORCE | EWX_FORCEIFHUNG, 0);
			ofExit();
		}
	}

	if (path_to_boot != "" && !b_manual_boot)
	{
		if (!b_booting && ofGetElapsedTimef() - last_respond_time > ping_wait_interval_sec)
		{
			ofLogError(ofGetTimestampString("%Y.%m.%d.%H:%M.%S")) << "no ping from App. restart! " << path_to_boot;
			terminate_app(name_to_kill);
			ofSleepMillis(1000 * terminate_closealert_boot_interval);
			close_alert_dialog(name_to_kill);
			ofSleepMillis(1000 * terminate_closealert_boot_interval);
			boot_app(path_to_boot);
		}
	}
}

void ofApp::draw() 
{
	ofDrawBitmapStringHighlight("alive:" + ofToString(ofGetElapsedTimef()), 10, 20);
	if (path_to_boot == "")
		ofDrawBitmapStringHighlight("NO BOOT PATH", 10, 40);
	else
	{
		if (b_booting)
			ofDrawBitmapStringHighlight("BOOTING", 10, 40);
		ofDrawBitmapStringHighlight(path_to_boot, 10, 60);
	}

}

void ofApp::terminate_app(const string appName)
{
	b_booting = true;
	last_boot_time = ofGetElapsedTimef();

	wstring_convert<codecvt_utf8<wchar_t>, wchar_t> cv;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			wstring wProcName = cv.from_bytes(appName);
			if (wcscmp(entry.szExeFile, wProcName.c_str()) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(snapshot);
}

void ofApp::close_alert_dialog(const string appName)
{
	auto conv_string_2_wstring = [](const string& s) -> wstring
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	};

	string s1 = appName;
	wstring ws;
	ws = conv_string_2_wstring(s1);
	LPCTSTR pS2 = ws.c_str();
	HWND hWnd = FindWindow(NULL, pS2);
	if (hWnd != NULL)
	{
		//DestroyWindow(hWnd);
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		ofLogNotice("closeWER") << "close " << pS2 << " window";
	}
	else
		ofLogNotice("closeWER") << "couldnt find " << appName << " window";
}

void ofApp::boot_app(const string path)
{
	if (!is_process_running(exe_name))
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
	else
		ofLogError() << "still process " << exe_name << " running. fail to boot app.";
}

bool ofApp::is_process_running(const string procName)
{
	bool bFound = false;

	wstring_convert<codecvt_utf8<wchar_t>, wchar_t> cv;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			wstring wProcName = cv.from_bytes(procName);
			if (wcscmp(entry.szExeFile, wProcName.c_str()) == 0)
			{
				bFound = true;
				break;
			}
		}
	}

	CloseHandle(snapshot);

	return bFound;

}
