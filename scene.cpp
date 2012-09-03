#include "header.h"

extern HWND okno;
/* Empty constructor. */
SCENE::SCENE(){
	graphs_built = audio_built = file_rendered = video_clipped = blending_setup = text_setup = scene_created = false;	
}
/* Real constructor. */
SCENE::SCENE(const CMarkup * original_script){
	// initial values
	source_path = L''; 
	graphs_built = file_rendered = video_clipped = blending_setup = text_setup = scene_created = false;
	fail_msg = "Non-HRESULT function."; hr = E_FAIL;
	time_passed = 0;
	seek = 10000000;
	top_pix = 5;

	// Read the file
	script = CMarkup(*original_script);
	bool file_read = script.FindElem(L"SOURCE");
		FunctionLog("CMarkup.FindElem(L\"SOURCE\")", file_read, true);
	source_path = script.GetData();

	// Build the video
	if(file_read){
		graphs_built   = BuildGraphs();
			if(!graphs_built)   {HRLog(fail_msg.c_str(), hr, true); return;}
		audio_built    = BuildAudio();
			if(!audio_built)    {HRLog(fail_msg.c_str(), hr, false);}
		file_rendered  = RenderFile();
			if(!file_rendered)  {HRLog(fail_msg.c_str(), hr, true); return;}
		video_clipped  = ClipVideo();
			if(!video_clipped)  {HRLog(fail_msg.c_str(), hr, true); return;}
		blending_setup = SetupTextBlending();
			if(!blending_setup) {FunctionLog(fail_msg.c_str(), true, true); return;}
		text_setup     = SetupTextProperties();
			if(!text_setup)     {HRLog(fail_msg.c_str(), hr, true); return;}
		scene_created = graphs_built | file_rendered | video_clipped | blending_setup | text_setup;
	}

	// Editor
	video_paused = false;
}
/* Destructor - destroys only things that were really built. */
SCENE::~SCENE(){
	if(text_setup){
		delete fontFamily;
		delete text_outline;
		delete text_filler;
		delete dark_filler;
	}
	if(blending_setup){
		DeleteDC(bitmap_hdc);
		delete drawing_bitmap;
		delete BMP_mix;
	}
	if(audio_built){
		audio_graph->Release(); 
		audio_media_event->Release();
		audio_media_control->Release();
	}
	if(graphs_built){
		video_media_control->Stop();
		video_media_event->SetNotifyWindow(NULL, 0, 0);

		bitmap_mixer->Release();
		video_mixer->Release(); 
		windowless_control->Release();
		video_media_control->Release();
		video_media_seeking->Release();
		video_media_event->Release();
		capture_graph->Release();
		video_graph->Release();
	}
}
/*Function builds up the video graph and renders it's video. There is no need to release the interfaces - they will be released through the destructor anyway. */
bool SCENE::BuildGraphs(){


	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&capture_graph);
		if(hr != S_OK) {fail_msg = "CoCreateInstance - ICaptureGraphBuilder2"; return false;}
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&video_graph);
		if(hr != S_OK) {fail_msg = "CoCreateInstance - IGraphBuilder (video)"; capture_graph->Release(); return false;}
	hr = capture_graph->SetFiltergraph(video_graph);
		if(hr != S_OK) {fail_msg = "ICaptureGraphBuilder2.SetFiltergraph(IGraphBuilder)"; video_graph->Release(); capture_graph->Release(); return false;}

	hr = video_graph->QueryInterface(IID_IMediaControl, (void **)&video_media_control);
		if(hr != S_OK) {fail_msg = "QueryInterface - IMediaControl (video)"; video_graph->Release(); capture_graph->Release(); return false;}
	hr = video_graph->QueryInterface(IID_IMediaSeeking, (void **)&video_media_seeking);
		if(hr != S_OK) {fail_msg = "QueryInterface - IMediaSeeking (video)"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); return false;}
	hr = video_graph->QueryInterface(IID_IMediaEvent, (void **)&video_media_event);
		if(hr != S_OK) {fail_msg = "QueryInterface - IMediaEvent"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); return false;}

	// Registeres a window that will recieve event notifications
	video_media_event->SetNotifyWindow((OAHWND)okno, WM_GRAPHNOTIFY, 0);
		if(hr != S_OK) {fail_msg = "IMediaEvent.SetNotifyWindow"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

	if(!InitWindowlessVMR()) 
		{video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

	hr = windowless_control->QueryInterface(IID_IVMRMixerBitmap9, (LPVOID *)&bitmap_mixer);
		if(hr != S_OK) {fail_msg = "QueryInterface - IVMRMixerBitmap9"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

    return true; 
}
/* Creates a new filter graph, which is later used for rendering additional audio files. */
bool SCENE::BuildAudio(){
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&audio_graph);
		if(hr != S_OK) {fail_msg = "CoCreateInstance - IGraphBuilder (audio)"; return false;}	
	hr = audio_graph->QueryInterface(IID_IMediaControl, (void **)&audio_media_control);
		if(hr != S_OK) {fail_msg = "QueryInterface - IMediaControl (video)"; audio_graph->Release(); return false;}
	hr = audio_graph->QueryInterface(IID_IMediaEvent, (void **)&audio_media_event);
		if(hr != S_OK) {fail_msg = "QueryInterface - IMediaEvent"; audio_graph->Release(); audio_media_control->Release(); video_media_seeking->Release(); return false;}
    return true; 
}
/* Embeddes the video into the main window. */
bool SCENE::InitWindowlessVMR()
{ 
	// Video Mixer initialization
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&video_mixer); 
		if(hr != S_OK) {fail_msg = "CoCreateInstance - IBaseFilter (video mixer)"; return false;}
    hr = video_graph->AddFilter(video_mixer, L"Video Mixing Renderer 9");
		if(hr != S_OK) {fail_msg = "IGraphBuilder.AddFilter(IBaseFilter)"; return false;}

    // Sets the rendering mode and number of the streams (used for image blending)
	IVMRFilterConfig9* filter_config; 
    hr = video_mixer->QueryInterface(IID_IVMRFilterConfig9, (void**)&filter_config); 
		if(hr != S_OK) {fail_msg = "QueryInterface - IVMRFilterConfig9"; video_mixer->Release(); return false;}	
    hr = filter_config->SetRenderingMode(VMR9Mode_Windowless); 
		filter_config->Release(); 
		if(hr != S_OK) {fail_msg = "IVMRFilterConfig9.SetRenderingMode(VMR9Mode_Windowless)"; video_mixer->Release(); return false;}	

	// Sets target window to display video within
    hr = video_mixer->QueryInterface(IID_IVMRWindowlessControl9, (void**)&windowless_control); 
		if(hr != S_OK) {fail_msg = "QueryInterface - IVMRWindowlessControl9"; video_mixer->Release(); return false;}	
	hr = windowless_control->SetVideoClippingWindow(okno);  
		if(hr != S_OK) {fail_msg = "QueryInterface - IVMRWindowlessControl9.SetVideoClippingWindow()"; windowless_control->Release(); return false;}	

	return true; 
} 
/* Finds and renders the video */
bool SCENE::RenderFile(){
	IBaseFilter * Source;

	// Create source filter
	hr = video_graph->AddSourceFilter(source_path.c_str(), L"Source1", &Source);
		if(hr != S_OK) {fail_msg = wstring2string(source_path.append(L" file loading - function IGraphBuilder.AddSourceFilter()")); return false;}
		
	// Render the video and audio
	hr = capture_graph->RenderStream(0, 0, Source, 0, video_mixer); 
		if(hr != S_OK) {fail_msg = "ICaptureGraphBuilder2.RenderStream() - video"; Source->Release(); return false;}
	hr = capture_graph->RenderStream(0, &MEDIATYPE_Audio, Source, 0, NULL);
		if(hr != S_OK) {fail_msg = "ICaptureGraphBuilder2.RenderStream() - audio"; Source->Release(); return false;}

	hr = video_media_seeking->GetDuration(&stop_time);
		if(hr != S_OK) {fail_msg = "IMediaSeeking.GetDuration()"; Source->Release(); return false;}
	Source->Release();
	return true;
}
/* Clips the playback from video dimensions to window dimensions, video is resized to full window width*/
bool SCENE::ClipVideo(){

	// Find out wheather the performance will be lowered
	hr = windowless_control->GetMaxIdealVideoSize(&video_width, &video_height);
		HRLog("IVMRWindowlessControl9.GetMaxIdealVideoSize()", hr, false);
		if(video_width < window_width) Log("Clipping will cause performance lost");

	// Obtain the source video dimensions and compute window video dimensions
	long source_width, source_height;
	hr = windowless_control->GetNativeVideoSize(&source_width, &source_height, NULL, NULL);
		HRLog("IVMRWindowlessControl9.GetNativeVideoSize()", hr, false);
		if(hr != S_OK) {video_width = window_width; video_height = window_height;}
	hr = windowless_control->SetAspectRatioMode(VMR_ARMODE_NONE);
		HRLog("IVMRWindowlessControl9.SetAspectRatioMode()", hr, false);
	

	double window_aspect = static_cast<double>(window_height) / static_cast<double>(window_width);
	double video_aspect  = static_cast<double>(source_height) / static_cast<double>(source_width);
	int y_position, x_position;
	if(window_aspect > video_aspect){ // will have top and bottom black border
		video_scale   = static_cast<double>(window_width) / static_cast<double>(source_width);
		video_width  = window_width;
		video_height = static_cast<long>(static_cast<double>(source_height) * video_scale);
		x_position = 0;
		y_position = (window_height - video_height)/2;
	}
	else { // will have left and right black border
		video_scale   = static_cast<double>(window_height) / static_cast<double>(source_height);
		video_height = window_height;
		video_width  = static_cast<long>(static_cast<double>(source_width) * video_scale);
		x_position = (window_width - video_width)/2;
		y_position = 0;
	}

	// clipping
	RECT source_rect; 
	SetRect(&source_rect, 0, 0, source_width, source_height);
	SetRect(&video_position, x_position, y_position, video_width + x_position, video_height + y_position);
	hr = windowless_control->SetVideoPosition(&source_rect, &video_position); // Set the video position.
		if(hr != S_OK) {fail_msg = "IVMRWindowlessControl9.SetVideoPosition()";return false;}

	return true;
}
/*Setups variables needed for text blending*/
bool SCENE::SetupTextBlending(){
	// Builds drawing device
	drawing_bitmap = new Gdiplus::Bitmap(video_width, video_height);
		if(!drawing_bitmap){
			fail_msg = "Bitmap allocation failed";
			return false;
		}
	BMP_mix = new Gdiplus::Graphics(drawing_bitmap);
		if(!BMP_mix){
			fail_msg = "Graphics allocation failed";
			delete drawing_bitmap;
			return false;
		}

	// Sets the DC
    HDC hdc = GetDC(okno);
		if (hdc == NULL) {fail_msg = "GetDC()"; delete drawing_bitmap; delete BMP_mix; return false;}
    bitmap_hdc = CreateCompatibleDC(hdc);
    ReleaseDC(okno, hdc);
		if (bitmap_hdc == NULL) {fail_msg = "CreateCompatibleDC()"; delete drawing_bitmap; delete BMP_mix; return false;}

	// connect a bitmap with the DC
	HBITMAP hbitmap;
	drawing_bitmap->GetHBITMAP(key_color, &hbitmap);   
    HBITMAP hbmOld = (HBITMAP)SelectObject(bitmap_hdc, hbitmap);
		if (hbmOld == 0) {fail_msg = "SelectObject()"; delete drawing_bitmap; delete BMP_mix; return false;}
 
	// Sets bitmap info 
    ZeroMemory(&bmpInfo, sizeof(bmpInfo));
    bmpInfo.dwFlags = VMR9AlphaBitmap_hDC | VMR9AlphaBitmap_SrcColorKey; // From HDC with blending
    bmpInfo.hdc = bitmap_hdc;
	SetRect(&bmpInfo.rSrc, 0, 0, drawing_bitmap->GetWidth(), drawing_bitmap->GetHeight()); // Whole picture to whole area
    bmpInfo.rDest.top = 0.0f;
	bmpInfo.rDest.left = 0.0f;
    bmpInfo.rDest.right = 1.0f;
    bmpInfo.rDest.bottom = 1.0f;
    bmpInfo.fAlpha = 1.0f; // transparency
	bmpInfo.clrSrcKey = key_color.ToCOLORREF(); // Key color

	// Mixes
    hr = bitmap_mixer->SetAlphaBitmap(&bmpInfo);
		HRLog("IVMRMixerBitmap9.SetAlphaBitmap()", hr, false);
		if(hr != S_OK) {fail_msg = "DirectShow failure"; delete drawing_bitmap; delete BMP_mix; return false;}

    DeleteObject(SelectObject(bitmap_hdc, hbmOld)); // deletes HBITMAP
    return true;
}
/* Mixes the bitmap with the current video. */
bool SCENE::BlendText(){
	// connects new bitmap with the DC
    HBITMAP hbitmap;
	drawing_bitmap->GetHBITMAP(key_color, &hbitmap);   
    HBITMAP hbmOld = (HBITMAP)SelectObject(bitmap_hdc, hbitmap);
		if (hbmOld == 0) {fail_msg = "SelectObject()"; delete drawing_bitmap; delete BMP_mix; return false;}
 
	// mixes
    bitmap_mixer->SetAlphaBitmap(&bmpInfo);
		HRLog("IVMRMixerBitmap9.SetAlphaBitmap()", hr, false);

	// releases the bitmap
    DeleteObject(SelectObject(bitmap_hdc, hbmOld)); // deletes HBITMAP
    return true;
}
/* Setup function for variables associated with the text. */
bool SCENE::SetupTextProperties(){
	fontFamily = 0; 
	text_outline = 0; 
	text_filler = 0;
	fontFamily = new Gdiplus::FontFamily(L"Times New Roman"); 
		if(fontFamily == 0) {Log("fontFamily allocation failed"); return false;}
	text_outline = new Gdiplus::Pen(text_surrounding_color, 3); 
		if(text_outline == 0) {Log("text_outline allocation failed"); delete fontFamily; return false;}
	text_filler = new Gdiplus::SolidBrush(text_fill_color);
		if(text_filler == 0) {Log("text_filler allocation failed"); delete fontFamily; delete text_outline; return false;}
	dark_filler = new Gdiplus::SolidBrush(nonactive_fill_color);
		if(dark_filler == 0) {Log("dark_filler allocation failed");delete text_filler; delete fontFamily; delete text_outline; return false;}

	StatusLog("Graphics.SetSmoothingMode()", BMP_mix->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias), false);
	return true;
}
/* After-scene system message posting. */
void SCENE::PostSceneFinished(){
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	APILog("PostMessage SCENE_FINISHED", !PostMessage(okno, SCENE_FINISHED, wParam, lParam), true);
}
/* After-built system message posting. */
void SCENE::PostSceneCreated(){
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	APILog("PostMessage SCENE_CREATED", !PostMessage(okno, SCENE_CREATED, wParam, lParam), true);	
}
/* Function goes through all the tests and determines wheather they passed or not.*/
bool SCENE::CheckTests(){
	return true;
}
/* Playback start. */
HRESULT SCENE::Play(){
	OAFilterState filter_state;
	
	// Covers the audio (additional audio can be paused->play or stopped->don't play)
	hr = audio_media_control->GetState(INFINITE, &filter_state);
		HRLog("IMediaControl.GetState - audio playing", hr, false);
	if(filter_state == 1) // Paused (not stopped)
		audio_media_control->Run();

	// Covers the video
	hr = video_media_control->Run();
	if(hr){
		hr = video_media_control->GetState(INFINITE, &filter_state);
			HRLog("IMediaControl.GetState - video playing", hr, false);
	}

	Repaint();

	return hr;
}
/* Playback pause. */
HRESULT SCENE::Pause(){
	OAFilterState filter_state;

	// Covers the audio 
	hr = audio_media_control->GetState(INFINITE, &filter_state);
		HRLog("IMediaControl.GetState - audio pause", hr, false);
	if(filter_state == 2) // Running
		audio_media_control->Pause();	

	// Covers the video
	hr = video_media_control->Pause();
	if(hr){
		hr = video_media_control->GetState(INFINITE, &filter_state);
			HRLog("IMediaControl.GetState - video pause", hr, false);
	}

	return hr;
}
/* Forced re-paint of the <b>video</b>. */
HRESULT SCENE::Repaint(){
	hr = windowless_control->RepaintVideo(okno, GetWindowDC(okno)); // Draws video on screen ... just to be sure
		HRLog("IVMRWindowlessControl9.RepaintVideo", hr, false);
	
	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions of the editor

/* Displaying the text on the screen. */
bool SCENE::TextOut(std::wstring text){

	// used for obtaining parametres of the string
	bounding_rect.Width = bounding_rect.Height = 0;
	StatusLog("GraphicPath.AddString()", path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleRegular, text_size, Gdiplus::PointF(0, 0), &strformat), false);
	StatusLog("GraphicPath.AddString()", path.GetBounds(&bounding_rect, NULL, text_outline), false); 
	// Top left text corner (the hit position defines middle of the text)
	Gdiplus::REAL left, top;
	left = 5;
	top  =  top_pix;
	top_pix += text_size + 4;

	// Build the real text path
	StatusLog("GraphicPath.Reset()", path.Reset(), false); // remove temp path
	StatusLog("GraphicPath.AddString()", path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleBold, text_size, Gdiplus::PointF(left, top), &strformat), false);	
	
	// Draw on the screen
	StatusLog("Graphic.DrawPath()", BMP_mix->DrawPath(text_outline, &path), false);	
	StatusLog("Graphic.FillPath()", BMP_mix->FillPath(text_filler, &path), false);

	StatusLog("GraphicPath.Reset()", path.Reset(), false); // remove path

	BlendText();
	Repaint();

	return true;
}

void SCENE::DisplayText(std::wstring text){
	LONGLONG current; video_media_seeking->GetCurrentPosition(&current); 
	if (video_paused){
		current -= 1;
		video_media_seeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &current, AM_SEEKING_NoPositioning);
		current += 1;
	}

	TextOut(text);	
	
	if(video_paused)
		video_media_seeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &current, AM_SEEKING_NoPositioning);
}

HRESULT SCENE::UpPressed(){
	seek *= 2;
	std::wstring text(L"Seek jump time now: ");
	text.append(boost::lexical_cast<std::wstring, LONGLONG>(seek));
	DisplayText(text);	

	return S_OK;
}

HRESULT SCENE::DownPressed(){
	seek /= 2;
	std::wstring text(L"Seek jump time now: ");
	text.append(boost::lexical_cast<std::wstring, LONGLONG>(seek));
	DisplayText(text);	

	return S_OK;
}

HRESULT SCENE::LeftPressed(){
	LONGLONG end;
	LONGLONG current;
	hr = video_media_control->Pause();
		HRLog("Left press IMediaControl.Pause",hr, false);
	if(hr == S_OK)
		hr = video_media_seeking->GetPositions(&current, &end);
	current -= seek;
	if(hr == S_OK){
	hr = video_media_seeking->SetPositions(&(current), AM_SEEKING_AbsolutePositioning, &end, AM_SEEKING_NoPositioning);
		HRLog("Left press IMediaSeeking.SetPositions",hr, false);
	}
	Repaint();
	return hr;
}

HRESULT SCENE::RightPressed(){
	LONGLONG null = 0;
	hr = video_media_control->Pause();
		HRLog("Right press IMediaControl.Pause",hr, false);
	if(hr == S_OK){
	hr = video_media_seeking->SetPositions(&(seek), AM_SEEKING_RelativePositioning, &null, AM_SEEKING_NoPositioning);
		HRLog("Right press IMediaSeeking.SetPositions",hr, false);
	}
	Repaint();
	return hr;
}

HRESULT SCENE::TPressed(){
	LONGLONG current;
	video_media_seeking->GetCurrentPosition(&current);
	std::wstring text(boost::lexical_cast<std::wstring, LONGLONG>(current));
	DisplayText(text);	
	return S_OK;
}

HRESULT SCENE::CPressed(){
	top_pix = 5;
	LONGLONG current; video_media_seeking->GetCurrentPosition(&current); current -= 1;
	if (video_paused){
		current -= 1;
		video_media_seeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &current, AM_SEEKING_NoPositioning);
		current += 1;
	}

	BMP_mix->Clear(key_color);
	BlendText();
	Repaint();

	if(video_paused)
		video_media_seeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &current, AM_SEEKING_NoPositioning);

	return S_OK;
}

HRESULT SCENE::DPressed(){
	DisplayText(L"D has no affiliation");	
	return S_OK;
}

HRESULT SCENE::FPressed(){
	DisplayText(L"F has no affiliation");	
	return S_OK;
}

HRESULT SCENE::NPressed(){
	DisplayText(L"N has no affiliation");	
	return S_OK;
}

HRESULT SCENE::SPressed(){
	DisplayText(L"Save");	
	return S_OK;
}

HRESULT SCENE::NumPressed(std::size_t){
	DisplayText(L"Numpad has no affiliation");	
	return S_OK;
}