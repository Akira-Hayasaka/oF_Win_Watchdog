#include "WatchDogResponder.h"

void WatchDogResponder::setup()
{
	bUseWatchDog = false;
	lastPingTime = Globals::ELAPSED_TIME;

	watchDogSender.setup("localhost", WATCHDOG_PORT);

	ofxXmlSettings xml;
	xml.load("package.xml");
	bUseWatchDog = (xml.getValue("useWatchDog", "") == "true") ? true : false;
	string watchDogPath = xml.getValue("watchDogPath", "");
	xml.clear();

	if (bUseWatchDog && !isProcessRunning("Watchdog.exe"))
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
				bootApp(bootPath);
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
			myExeName = f.getFileName();
			myExePath = f.getAbsolutePath();
			ofStringReplace(myExePath, "\\", "\\\\");
			ofStringReplace(myExePath, "/", "\\\\");
			break;
		}
	}

	//ofAddListener(Globals::killWatchDogEvent, this, &WatchDogResponder::killWatchDog);
}

void WatchDogResponder::update()
{
	// ping to watchDog
	if (bUseWatchDog)
	{
		if (Globals::ELAPSED_TIME - lastPingTime > 0.5)
		{
			ofxOscMessage m;
			m.setAddress("/imok");
			m.addStringArg(myExePath);
			m.addStringArg(myExeName);
			watchDogSender.sendMessage(m, false);
			lastPingTime = Globals::ELAPSED_TIME;
		}
	}
}

void WatchDogResponder::killWatchDog()
{
	ofxOscMessage m;
	m.setAddress("/killyou");
	watchDogSender.sendMessage(m, false);
	bUseWatchDog = false;
	ofLog() << "killWatchDog";
}

bool WatchDogResponder::isProcessRunning(const string procName)
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

void WatchDogResponder::bootApp(const string path)
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
