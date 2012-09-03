#ifndef DEF_ENGINE_FUNCTIONS
#define DEF_ENGINE_FUNCTIONS
#include <string>

// Log Functions
	// For functions that returns DX HRESULT
void HRLog(const char * function, const long value, const bool terminate);
void HRLog(const char * function, const char * failure_message, const long value, const bool terminate);
	// For functions whose errors can be obtained through GetLastError
void APILog(const char * function, const int passed, const bool terminate);
void APILog(const char * function, const char * failure_message, const int passed, const bool terminate);
	// For GDIPlus functions
void StatusLog(const char * funkce, const Gdiplus::Status, const bool terminate);
void StatusLog(const char * funkce, const char * failure_message, const Gdiplus::Status, const bool terminate);
	// For my functions
void FunctionLog(const char * function, const int passed, const bool terminate);
void FunctionLog(const char * function, const char * failure_message, const int passed, const bool terminate);
	// Other universal logs
void TermLog(char * zprava);
void Log(const unsigned int zprava);
void Log(const char * zprava);

	// Editor logs
void MousePosLog();

// Conversion functions
std::string wstring2string(std::wstring wstr); // Declared with log functions
std::wstring string2wstring(std::string str);

#endif