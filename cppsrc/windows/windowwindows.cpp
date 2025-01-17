#include "windowwindows.h"

#include <windows.h>
#include <stringapiset.h>

#include <codecvt>
#include <string>
#include <locale>

void windowwindows::getActiveWindow(Napi::Object& obj) {
	WCHAR window_title[256] {};
	HWND foreground_window = GetForegroundWindow();
	GetWindowTextW(foreground_window, window_title, 256);

	std:setlocale(LC_ALL, "en_US.UTF-8");

	DWORD pid;
	GetWindowThreadProcessId(foreground_window, &pid);
	// Process
	std::string fullpath;
	std::string shortpath;

	if (HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_QUERY_INFORMATION, false, pid)) {
		TCHAR process_filename[MAX_PATH];
		DWORD charsCarried = MAX_PATH;

		if (QueryFullProcessImageNameA(hProc, 0, process_filename, &charsCarried)) {
			fullpath = std::string(process_filename, charsCarried);
		}

		CloseHandle(hProc);
	}

	const size_t last_slash_idx = fullpath.find_last_of("\\/");

	if (std::string::npos != last_slash_idx) {
		shortpath = fullpath;
		shortpath.erase(0, last_slash_idx + 1);
	}

	// Last input time
	LASTINPUTINFO last_input {};

    // Without setting cbSize GetLastError() returns the parameter is incorrect
    last_input.cbSize = sizeof(last_input);
	DWORD idle_time = 0UL;

	if (GetLastInputInfo(&last_input)) {
		idle_time = ( GetTickCount() - last_input.dwTime ) / 1000;
	}

    if (std::to_string(pid) == "0") {
        // Just for the idle time
        obj.Set("os", "windows");
    	obj.Set("windowClass", "");
    	obj.Set("windowName", "");
    	obj.Set("windowDesktop", "0");
    	obj.Set("windowType", "0");
    	obj.Set("windowPid", "0");
    	obj.Set("idleTime", std::to_string(idle_time));
		obj.Set("windowPath", "");
    	return;
    }

	std::wstring ws( window_title );
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;

	obj.Set("os", "windows");
	obj.Set("windowClass", shortpath);
	obj.Set("windowName", myconv.to_bytes(ws));
	obj.Set("windowDesktop", "0");
	obj.Set("windowType", "0");
	obj.Set("windowPid", std::to_string(pid));
	obj.Set("idleTime", std::to_string(idle_time));
	obj.Set("windowPath", fullpath);
}

Napi::Object windowwindows::getActiveWindowWrapped(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	Napi::Object obj = Napi::Object::New(env);
	windowwindows::getActiveWindow(obj);

	return obj;
}

Napi::Object windowwindows::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		"getActiveWindow", Napi::Function::New(env, windowwindows::getActiveWindowWrapped)
	);

	return exports;
}
