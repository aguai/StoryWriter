///////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Story Writer editor                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
/*//**************************************** Controls: ****************************************//*/
SPACE: pause / play video
down : half the seek time
up   : double the seek time
left : jump backwards the seek time
right: jump forward the seek time
c    : clear the display
f    : fix current subtitles
n    : new subtitle
s    : save the current changes
t    : display time
num 1: move fixed subtitle beginning backwards for the seek time
num 3: move fixed subtitle beginning forward for the seek time
num 4: move fixed subtitle end backwards for the seek time
num 6: move fixed subtitle end forward for the seek time

/*************************************************************************************************/

#include "header.h"
#include <Windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

// system variables -------------------------------------------------------
// window parametres
HWND okno;
ULONG_PTR gdiplus_token;
int	win_boarder_width;
int win_boarder_height;
// graphics
Graphics     * SCR = 0; // Screen graphics - draws to window
Graphics     * BMP = 0; // Background buffer graphics - draws to memory_bitmap
Bitmap       * memory_bitmap = 0;
CachedBitmap * cached_bitmap = 0;
// program states
bool fullscreen  = false;
bool initialized = false; // Controll for cleaning
bool active      = true;  // Controlls activation / Deactivation of the whole process
// engine
DWORD        start_time  = 0;
GAMECONTROL* controller  = 0;

// Functions-----------------------------------------------------------------
void VytvorOkno(HINSTANCE & instance, HWND & okno);
bool Initialize(HINSTANCE & instance, HWND & okno);
bool ChangeDisplayMode(int pixel_width, int pixel_height, bool fullscreen);
void CleanDaMess();

// Body ----------------------------------------------------------------------
/* Window procedure which handles system messages. */
LRESULT CALLBACK WindowProcedure( HWND okno, UINT zprava, WPARAM w_param,  LPARAM l_param )
{
	switch (zprava)
	{
		case WM_CLOSE: // On close message
			PostQuitMessage(0);
		break;

		case WM_PAINT: // When repaint is needed
			FunctionLog("WM_PAINT ValidateRect(okno, NULL) call.", ValidateRect(okno, NULL), false);
			if(controller->IsSceneAccesible())
				FunctionLog("WM_PAINT game Display call.", controller->Display(), false);			
		break;

		case WM_MOUSEMOVE:
			if(controller->IsSceneAccesible())
				controller->MouseMove(MAKEPOINTS(l_param).x, MAKEPOINTS(l_param).y);
		break;

		case WM_LBUTTONDOWN:
			if(controller->IsSceneAccesible())
				controller->LeftClick(MAKEPOINTS(l_param).x, MAKEPOINTS(l_param).y);
		break;

		case WM_RBUTTONDOWN:

			if(controller->IsSceneAccesible())
				controller->RightClick(MAKEPOINTS(l_param).x, MAKEPOINTS(l_param).y);
		break;

		case WM_KEYDOWN:
			switch (w_param){
				case VK_ESCAPE:
				break;

				case VK_SPACE:
					if(!controller->SpacePressed())
						MessageBox(okno, L"Space press action failed", L"Fail", MB_OK);
				break;

				case VK_UP:
					if(!controller->UpPressed())
						MessageBox(okno, L"Up key press action failed", L"Fail", MB_OK);
				break;

				case VK_DOWN:
					if(!controller->DownPressed())
						MessageBox(okno, L"Down key press action failed", L"Fail", MB_OK);
				break;

				case VK_LEFT:
					if(!controller->LeftPressed())
						MessageBox(okno, L"Left key press action failed", L"Fail", MB_OK);
				break;

				case VK_RIGHT:
					if(!controller->RightPressed())
						MessageBox(okno, L"Right key press action failed", L"Fail", MB_OK);
				break;

				case (0x43):
					if(!controller->CPressed())
						MessageBox(okno, L"C key press action failed", L"Fail", MB_OK);
				break;

				case (0x44):
					if(!controller->DPressed())
						MessageBox(okno, L"D key press action failed", L"Fail", MB_OK);
				break;

				case (0x46):
					if(!controller->FPressed())
						MessageBox(okno, L"F key press action failed", L"Fail", MB_OK);
				break;

				case (0x4E):
					if(!controller->NPressed())
						MessageBox(okno, L"N key press action failed", L"Fail", MB_OK);
						
				break;

				case (0x53):
					if(!controller->SPressed())
						MessageBox(okno, L"S key press action failed", L"Fail", MB_OK);
				break;

				case (0x54):
					if(!controller->TPressed())
						MessageBox(okno, L"T key press action failed", L"Fail", MB_OK);
				break;

				case (0x61):
					if(!controller->NumPressed(1))
						MessageBox(okno, L"1 key press action failed", L"Fail", MB_OK);
				break;

				case (0x63):
					if(!controller->NumPressed(3))
						MessageBox(okno, L"3 key press action failed", L"Fail", MB_OK);
				break;

				case (0x64):
					if(!controller->NumPressed(4))
						MessageBox(okno, L"4 key press action failed", L"Fail", MB_OK);
				break;

				case (0x66):
					if(!controller->NumPressed(6))
						MessageBox(okno, L"6 key press action failed", L"Fail", MB_OK);
				break;
			}
		break;

		case WM_GETMINMAXINFO: // Blocks window parametres change
			LPMINMAXINFO area_minimax;
			area_minimax = (LPMINMAXINFO)l_param;
			if(area_minimax){
				area_minimax->ptMaxSize.x = win_boarder_width;
				area_minimax->ptMaxSize.y = win_boarder_height;
				area_minimax->ptMaxTrackSize.x = win_boarder_width;
				area_minimax->ptMaxTrackSize.y = win_boarder_height;
				area_minimax->ptMinTrackSize.x = win_boarder_width;
				area_minimax->ptMinTrackSize.y = win_boarder_height;
			}
		break;

		case WM_GRAPHNOTIFY: // video event
			if(controller->IsSceneAccesible())
				FunctionLog("Event handeling after the notification", controller->VideoEventOccured(), false);
		break;

		case SCENE_CREATED: // New scene was created
			controller->SceneAccesible(true);
		break;

		default:
			return (DefWindowProc(okno, zprava, w_param, l_param)); // Pøesmìrování nezachycených zpráv
	}
	return 0;
}


/* Main function - creates a window and starts the message loop. */
INT WINAPI WinMain( HINSTANCE instance, HINSTANCE predchozi_instance, LPSTR sp_prikazovy_radek, INT i_prikaz_zobrazeni ) 
{ 
	start_time = GetTickCount(); // Sets up internal time of the program

	Log("Engine initialization started."); // Engine initialization begun

		// fullscreen variable is set depending on wheather the change passed - if not, it does not try it again -> the variable is set furthermore
	fullscreen = ChangeDisplayMode(window_width, window_height, fullscreen);
	
		// Creates a window - window parameters depends on wheather the fullscreen is on; 
	VytvorOkno(instance, okno); 

	initialized = Initialize(instance, okno);
	FunctionLog("Engine initialization succesful.", initialized, true); 
	
    ShowWindow(okno, SW_SHOW);
    FunctionLog("Startup window update.", UpdateWindow(okno), false);

	// Message pump - just for messagesDestroyWindow
    MSG msg;
    DWORD last_tick = 0;
	while( true )
	{
		Sleep( 1 );
		if (active == true) {
			if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				// Log(msg.message); // When the log is needed
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT){ 
					CleanDaMess();
					break;
				}
			}
			if (GetTickCount() - last_tick > 1000 / 100 ) {
				int difference = GetTickCount() - last_tick;
				last_tick = GetTickCount();	
				if(controller->IsSceneAccesible())
					controller->Update(difference); // Adds milliseconds of the running time to the counter of current game
			}		
		}
		else {
		   GetMessage(&msg,(HWND) NULL, 0, 0);
		   TranslateMessage(&msg);
		   DispatchMessage(&msg);
		   if (msg.message == WM_QUIT){ 
				CleanDaMess();
				break;
			}
		}	
    }
	return static_cast<int>(msg.wParam);
}          

/* Function that initializates all the data structures needed. */
bool Initialize(HINSTANCE &  instance, HWND & okno){
	// System engine fire up
	GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::Status return_val = GdiplusStartup(&gdiplus_token, &gdiplusStartupInput, NULL);
		StatusLog("GdiplusStartup", return_val, true);
		if(return_val) return false;
	HRESULT hr = CoInitialize(NULL);
		HRLog("CoInitialize", hr, true); 
		if(hr) {GdiplusShutdown(gdiplus_token); return false;}

	// Data structures initialization (safe)
	SCR = new Graphics(okno);
		if(!SCR) 
			{TermLog("Screen graphics allocation failed, probably out of memory"); GdiplusShutdown(gdiplus_token); return false;}
	memory_bitmap = new Gdiplus::Bitmap(window_width, window_height);
		if(!memory_bitmap) 
			{TermLog("Bitmap allocation failed, probably out of memory"); GdiplusShutdown(gdiplus_token); delete SCR; return false;}
	BMP = new Gdiplus::Graphics(memory_bitmap);
		if(!BMP) 
			{TermLog("Bitmap graphics allocation failed, probably out of memory"); GdiplusShutdown(gdiplus_token); delete memory_bitmap; delete SCR; return false;}
	cached_bitmap = new Gdiplus::CachedBitmap(memory_bitmap, SCR);
		if(!cached_bitmap)
			{TermLog("Cached bitmap allocation failed, probably out of memory"); GdiplusShutdown(gdiplus_token); delete BMP; delete memory_bitmap; delete SCR; return false;}
	controller = new GAMECONTROL();
		if(!controller)
			{TermLog("GameControl allocation failed, probably out of memory"); GdiplusShutdown(gdiplus_token); delete cached_bitmap; delete BMP; delete memory_bitmap; delete SCR; return false;}
	return true;
}
/* Cleaning of the memory used. */
void CleanDaMess(){
	FunctionLog("Restoring the resolution in the end.", ChangeDisplayMode(window_width, window_height, false), false);			
	FunctionLog("DestroyWindow", DestroyWindow(okno), false);
	
	if(initialized){
		controller->SceneAccesible(false);
		delete controller;
		delete SCR;
		delete memory_bitmap;
		delete BMP;
		delete cached_bitmap; 
		CoUninitialize();
		GdiplusShutdown(gdiplus_token);
	}
}

/* Main window creation. */
void VytvorOkno(HINSTANCE & instance, HWND & okno){

	WNDCLASSEX WC; // Vytvoøení promìnné struktury WNDCLASSEX - struktura tøídy okna

	WC.cbSize =        sizeof(WNDCLASSEX); //první parametr - velikost struktury
	WC.style =         0; // Styl okna (O pro základní styl
	WC.lpfnWndProc =   (WNDPROC)WindowProcedure; // Adresa procedury okna které se stará o zracování zpráv okna
	WC.cbClsExtra =    0; // Množství extra pamìti (v bitech)
	WC.cbWndExtra =    0; // pro alokování pro každé okno tøídy
	WC.hInstance =     instance; // pøiøazení manipulátoru instance vytvoøené na zaèátku programu 
	WC.hIcon =         LoadIcon(instance, MAKEINTRESOURCE(1000)); //Naètení ikony ze zdroje
	WC.hIconSm =       LoadIcon(instance, MAKEINTRESOURCE(1000)); //Naètení malé ikony ze zdroje
	WC.hCursor =       LoadCursor(NULL, IDC_ARROW); //Naètení defaultního kurzoru
	WC.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2); //Nastavení pozadí programu (nutno pøevést na štìtec a inkrementovat o 1 pro zákl. barvu)
	WC.lpszMenuName =  NULL; // Jméno pro menu loadované ze zdroje
	WC.lpszClassName = L"Moje trida"; // Jméno tøídy

	APILog("RegisterClassEx",  !RegisterClassEx(&WC), true);

	win_boarder_width  = window_width  + (GetSystemMetrics(SM_CXBORDER))*2;
	win_boarder_height = window_height + (GetSystemMetrics(SM_CYCAPTION)) + (GetSystemMetrics(SM_CYBORDER));

	if(fullscreen)	
		okno = CreateWindowEx(0, L"Moje trida", L"delta playground", WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
					   0, 0, window_width, window_height, NULL, NULL, instance, NULL ); 
	else	
		okno = CreateWindowEx(0, L"Moje trida", L"delta playground", WS_OVERLAPPEDWINDOW, (GetSystemMetrics(SM_CXSCREEN) - window_width)/2, (GetSystemMetrics(SM_CYSCREEN) - window_height)/2, 
					    window_width + ((GetSystemMetrics(SM_CXSIZEFRAME))*2), window_height + (GetSystemMetrics(SM_CYCAPTION)) + (GetSystemMetrics(SM_CYSIZEFRAME)*2), NULL, NULL, instance, NULL ); 
	
	APILog("CreateWindowEx", !okno, true);
}
/* Function which handles the change of the monitor resolution. 
   NEEDS TO BE REVIEWED*/
bool ChangeDisplayMode(int pixel_width, int pixel_height, bool fullscreen) {
	/* For now it terminates application, when unable to change the resolution*/
	if(!fullscreen) { 
		ChangeDisplaySettings(NULL,0); // Switch to registered resolution
		return false;
	}
	else {
		DEVMODE dev_mode = {0};

		FunctionLog("EnumDisplaySettings", EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dev_mode), true);

		dev_mode.dmPelsWidth = pixel_width;
		dev_mode.dmPelsHeight = pixel_height;
		dev_mode.dmBitsPerPel = 32;
		dev_mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;  // change of height and width

		LONG result = ChangeDisplaySettings(&dev_mode,CDS_FULLSCREEN);
		if(result){ // won't work with 32 bits, trying 24
			result = ChangeDisplaySettings(&dev_mode,CDS_FULLSCREEN);
			dev_mode.dmBitsPerPel = 24;
		}
		else if(result){ // won't work with 24 bits, trying 16
			result = ChangeDisplaySettings(&dev_mode,CDS_FULLSCREEN);
			dev_mode.dmBitsPerPel = 16;
		}
		else if(result){ // Won't work at all in fullscreen -> error log
			Log("Resolution Change failed due to error:");
			if (result == DISP_CHANGE_BADFLAGS)
			Log("DISP_CHANGE_BADFLAGS");
			if (result == DISP_CHANGE_BADMODE)
			Log("DISP_CHANGE_BADMODE");
			if (result == DISP_CHANGE_BADPARAM)
			Log("DISP_CHANGE_BADPARAM");
			if (result == DISP_CHANGE_FAILED)
			Log("DISP_CHANGE_FAILED");
			if (result == DISP_CHANGE_NOTUPDATED)
			Log("DISP_CHANGE_NOTUPDATED");
			if (result == DISP_CHANGE_RESTART)
			Log("DISP_CHANGE_RESTART");
			int user_response = MessageBox(NULL, L"Application failed to change the window resolution to 800*600 (your device probably doesn't support it)." 
										   L"Do you want to continue in the window mode instead?", L"Resolution change failure", MB_YESNO);
			if(user_response == IDNO)
				TermLog("Resolution change failure");
			return false;
		}
		return true;
	}
}