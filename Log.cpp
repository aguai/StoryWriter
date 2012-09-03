#include "header.h"
#include <windows.h>
#include <fstream>
#include <DXErr.h>

std::wofstream fout("log.txt"); // output file

extern HWND okno;
extern DWORD start_time; // Start of the whole application

/* Log for functions that returns HRESULT. */
void HRLog(const char * function, const long hr, const bool terminate){
	if(hr != S_OK){
		long time_elapsed = GetTickCount() - start_time;
		std::wstring error_string = DXGetErrorString(hr);

		fout << L"Function: \"" << function << L"\" failed with result: " << error_string.c_str() << L" in time: " << time_elapsed << L" ms." << std::endl;
		if (terminate){
			fout << L"Application will terminate." << std::endl;
			PostQuitMessage(hr);
		}
	}
}
/* Extended log for functions that returns HRESULT. */
void HRLog(const char * function, const char * failure_message, const long hr, const bool terminate){
	if(hr != S_OK){
		long time_elapsed = GetTickCount() - start_time;
		std::wstring error_string = DXGetErrorString(hr);

		fout << L"Function: \"" << function << L"\" failed with result: " << error_string.c_str() << L" in time: " << time_elapsed << L" ms." << std::endl;
		fout << L"Addtional message covering failure: " << failure_message << std::endl;
		if (terminate){
			fout << L"Application will terminate." << std::endl;
			PostQuitMessage(hr);
		}
	}
}
/* Log for functions from WIN 32 API, whose failure message can be obtained by GetLastError. */
void APILog(const char * function, const int value, const bool terminate){
	if(value != 0){
		long time_elapsed = GetTickCount() - start_time;
		DWORD failure_report = GetLastError();

		fout << L"API function: \"" << function << L"\" failed in time: " << time_elapsed << L" ms." << std::endl;
		fout << L"Last error is: " << failure_report << L"." << std::endl;
		if(terminate){
			fout << L"Application will terminate." << std::endl;
			PostQuitMessage(failure_report);
		}
	}
}
/* Extended log for functions from WIN 32 API, whose failure message can be obtained by GetLastError. */
void APILog(const char * function, const char * failure_message, const int value, const bool terminate){
	if(value != 0){
		long time_elapsed = GetTickCount() - start_time;
		DWORD failure_report = GetLastError();

		fout << L"API function: \"" << function << L"\" failed with in time: " << time_elapsed << L" ms." << std::endl;
		fout << L"Last error is: " << failure_report << L"." << std::endl;
		fout << L"Addtional message covering failure: " << failure_message << std::endl;
		if(terminate){
			fout << L" Application will terminate." << std::endl;
			PostQuitMessage(failure_report);
		}
	}
}
/* Log for my functions (return must be true). */
void FunctionLog(const char * function, const int passed, const bool terminate){
	if(passed == false){
		long time_elapsed = GetTickCount() - start_time;
		fout << L"Function: \"" << function << L"\" failed in time: " << time_elapsed << L" ms." << std::endl;
		if(terminate){
			fout << L"Application will terminate." << std::endl;
			PostQuitMessage(1);
		}
	}
}
/* Extended log for my functions (return must be true). */
void FunctionLog(const char * function, const char * failure_message, const int passed, const bool terminate){
	if(passed == false){
		long time_elapsed = GetTickCount() - start_time;

		fout << L"Function: \"" << function << L"\" failed in time: " << time_elapsed << L" ms." << std::endl;
		fout << L"Addtional message covering failure: " << failure_message << std::endl;
		if(terminate){
			fout << L" Application will terminate." << std::endl;
			PostQuitMessage(1);
		}
	}
}
/* Log for functions that return status (return must be false). */
void StatusLog(const char * function, const Gdiplus::Status status, const bool terminate){
	if(status != 0){
		long time_elapsed = GetTickCount() - start_time;

		fout << L"Function: \"" << function << L"\" failed in time: " << time_elapsed << L" ms." << std::endl;
		fout << L"Gdiplus object status is: " << status << L"." << std::endl;
		if(terminate){
			fout << L" Application will terminate." << std::endl;
			PostQuitMessage(status);
		}
	}	
}
/* Extended log for functions that return status (return must be false). */
void StatusLog(const char * function, const char * failure_message, const Gdiplus::Status status, const bool terminate){
	if(status != 0){
		long time_elapsed = GetTickCount() - start_time;

		fout << L"Function: \"" << function << L"\" failed in time: " << time_elapsed << L" ms." << std::endl;
		fout << L"Gdiplus object status is: " << status << L"." << std::endl;
		fout << L"Addtional message covering failure: " << failure_message << std::endl;
		if(terminate){
			fout << L" Application will terminate." << std::endl;
			PostQuitMessage(status);
		}
	}	
}
/* With-message terminal log. */
void TermLog(char * message){
	long time_elapsed = GetTickCount() - start_time;

	fout << L"Terminal failure: \"" << message << L"\" in time: " << time_elapsed << L" ms." << std::endl;
	PostQuitMessage(1);
}
/* Integer log. */
void Log(const unsigned int message){
	long time_elapsed = GetTickCount() - start_time;

	fout << L"Message: " << message << L" = " << std::hex << L"0x" << message << std::dec << L" in time: " << time_elapsed << L"." << std::endl ;
}
/* Message log. */
void Log(const char * message){ // Engine reports
	long time_elapsed = GetTickCount() - start_time;

	fout << message <<  L" Time elapsed: " << time_elapsed << L"." << std::endl ;
}
/*
void MousePosLog(){
	POINT position;
	GetCursorPos(&position);
	fout << L"Mouse position x: " << position.x << L", y: " << position.y << std::endl;
}*/
/* Convertor. */
std::string wstring2string(std::wstring wstr){
std::string str(wstr.length(), ' ');
copy(wstr.begin(), wstr.end(), str.begin());
return str;
}
/* Convertor. */
std::wstring string2wstring(std::string str) {
std::wstring wstr(str.length(), L' ');
copy(str.begin(),str.end(),wstr.begin());
return wstr;
}