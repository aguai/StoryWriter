#include "header.h"

extern HWND okno;

// Functions regarding the OPTION class------------------------------------------
/* Constructor. */
DIALOGUE::OPTION::OPTION(std::wstring & object_text, Gdiplus::GraphicsPath * object_path, std::vector<state_changes> & object_state_changes){
	text = object_text;
	text_path = 0;
	text_path = object_path->Clone();
	caused_changes = object_state_changes;
}
/* Copy constructor. */
DIALOGUE::OPTION::OPTION(const OPTION& original){
	text = original.text;
	text_path = 0;
	text_path = original.text_path->Clone();
	caused_changes = original.caused_changes;
}
/* Destructor. */
DIALOGUE::OPTION::~OPTION(){
	caused_changes.clear();
	if(text_path != 0)
		delete text_path;
}
// Functions regarding the DIALOGUE class----------------------------------------
/* Constructor. */
DIALOGUE::DIALOGUE(const CMarkup * original_script) : SCENE(original_script){
	Gdiplus::Font temp_font(fontFamily, text_size, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	line_height = static_cast<int>(temp_font.GetHeight(BMP_mix)); // sets the difference between two lines, in pixels
	line_top = video_height - 25; // sets top of the zero line 25 pixels beyond the bottom of the window

	highlited_option = 0;

	if(!scene_created)
		return;
	if(!CreateOptions())
		return;
	if(!DrawOptions())
		return;
	if(Play() != S_OK)
		return;
	PostSceneCreated();
}
/* Destructor. */
DIALOGUE::~DIALOGUE(){
	options.clear();
}
/* Creating of the options from the script. */
bool DIALOGUE::CreateOptions(){

	// atributes
	std::wstring text;
	Gdiplus::GraphicsPath path;
	// caused changes which will be paired with the options
	std::string state_name;
	std::vector<state_changes> changes; 

	if(!script.FindElem(L"OPTIONS")){
		TermLog("Options tag not found in the Dialogue scene, script.xml file is probably damaged.");
		return false;
	}
	script.IntoElem();

	// Going through the options
	if(!script.FindElem(L"OPTION")){
		script.OutOfElem();
		TermLog("No option to choose found in the dialogue scene.");
		return false;
	}
	else do {
		text = script.GetAttrib(L"text");
		
		line_top -= line_height; // Put the line above the previous
		stat = path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleBold, text_size, Gdiplus::PointF(0, Gdiplus::REAL(line_top)), &strformat);
			if (stat != 0) StatusLog("GraphicPath.AddString()", stat, false);

		script.IntoElem();

		if(!CheckTests()){
			script.OutOfElem();
			continue;
		}


		// creates the map of state changes
		while(script.FindElem(L"STATECHANGE")){
			state_changes current_changes;
			current_changes.name = wstring2string(script.GetData());
			current_changes.value = atoi((wstring2string(script.GetAttrib(L"value"))).c_str());
			current_changes.replace = atoi((wstring2string(script.GetAttrib(L"replace"))).c_str())  != 0 ? true : false;
			changes.push_back(current_changes);
		}
		
		script.OutOfElem(); // Jumps out of the object

		options.push_back(OPTION(text, &path, changes));
		changes.clear();
		StatusLog("GraphicPath.Reset()", path.Reset(), false);

	} while(script.FindElem(L"OPTION"));

	script.OutOfElem();
	return true;
}
/* Drawing of the options on the screen. */
bool DIALOGUE::DrawOptions(){
	StatusLog("Graphic.Clear()", BMP_mix->Clear(key_color), false);

	int i = 1;

	for(std::vector<OPTION>::iterator it = options.begin(); it < options.end(); it++, i++) {
		StatusLog("Graphic.DrawPath()", BMP_mix->DrawPath(text_outline, it->text_path), false);	
		if(i != highlited_option)
			StatusLog("Graphic.FillPath()", BMP_mix->FillPath(dark_filler, it->text_path), false);
		else
			StatusLog("Graphic.FillPath()", BMP_mix->FillPath(text_filler, it->text_path), false);
	}
	return BlendText();		
}
/* Detection of the collision with the text. */
int DIALOGUE::DetectCollision(const Gdiplus::PointF & hit_position){
	unsigned int i = 1; // i stores number of the intersecting object, if i is equal to number of objects, hit wasn't detected -> returns 0
	
	for(std::vector<OPTION>::iterator it = options.begin(); it < options.end(); it++)
		if (it->text_path->IsVisible(Gdiplus::PointF(hit_position)))
			break;
		else 
			i++;

	return (i != (options.size() + 1) ) ? i : 0; // if i is equal to count of options + 1, it means there were no hit -> returns 0 
}
/* Left click - activation of the option. */
HRESULT DIALOGUE::LeftClick(const int x, const int y){
	return S_OK;
}
/* Right click - no assignment. */
HRESULT DIALOGUE::RightClick(const int x, const int y){
	return S_OK;
}
/* Mouse move - detection of the collision with the option. */
HRESULT DIALOGUE::MouseMove(const int x, const int y){
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	if(i != highlited_option){ // Mouse moved to the new option or out
		highlited_option = i;
		DrawOptions();
	}	
	return S_OK;
}
/* Update - end of the video check. */
HRESULT DIALOGUE::Update(){
	hr = video_media_seeking->GetPositions(&time_passed,&stop_time);
		HRLog("IMediaEvent.GetEvent", hr, false);
	if (time_passed >= stop_time){
		LONGLONG null = 0;
			hr = video_media_control->Stop();
			HRLog("After-end-of-loop IMediaSeeking.Stop",hr, false);
		if(hr == S_OK){
			hr = video_media_seeking->SetPositions(&null, AM_SEEKING_AbsolutePositioning, &stop_time, AM_SEEKING_NoPositioning);
			HRLog("After-end-of-loop IMediaSeeking.SetPositions",hr, false);
		}
		if(hr == S_OK){
			OAFilterState filter_state;
			hr = video_media_control->Run();
			if(hr){
				hr = video_media_control->GetState(INFINITE, &filter_state);
					HRLog("IMediaControl.GetState - after-end-of-loop video playing", hr, false);
			}
		}
	}
	return hr;
}
/* Response for the video event - log. */
HRESULT DIALOGUE::EventResponse(){
	long event_code, param1, param2;
	hr = video_media_event->GetEvent(&event_code, &param1, &param2, 0);
		HRLog("IMediaEvent.GetEvent", hr, false);
	if(SUCCEEDED(hr)){
		char buffer [33];
		std::string msg = "Video event occured. Event code: ";
		msg.append(itoa(event_code,buffer,16));
		msg.append(".");
		Log(msg.c_str());
	}
	return hr;
}