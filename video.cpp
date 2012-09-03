#include "header.h"

extern HWND okno;

// Functions regarding the SUBTITLE class----------------------------------------
/* Constructor. */
VIDEO::SUBTITLE::SUBTITLE(std::wstring sub_text, LONGLONG sub_start, LONGLONG sub_end){
	text = sub_text;
	time_start = sub_start;
	time_end = sub_end;
}
// Functions regarding the VIDEO class-------------------------------------------
/* Constructor. */
VIDEO::VIDEO(const CMarkup * original_script) : SCENE(original_script)
{
	active_subtitle = fixed_subtitle = -1;

	if(!scene_created)
		return;
	if(!CreateSubtitles())
		return;
	if(Play() != S_OK)
		return;

	PostSceneCreated();
}
/* Destructor. */
VIDEO::~VIDEO(){
	while (ShowCursor(true) < 0) continue;
	scene_subtitles.clear();
}
/* Creation of the subtitles from the script. */
bool VIDEO::CreateSubtitles(){
		// atributes
	std::wstring text;
	LONGLONG start;
	LONGLONG end;

	if(!script.FindElem(L"SUBTITLES")){
		TermLog("SUBTITLES tag not found in the VIDEO scene, script.xml file is probably damaged.");
		return false;
	}
	script.IntoElem();

	// Going through the subtitles
	while(script.FindElem(L"SUBTITLE")){
		text  = script.GetData();
		start = boost::lexical_cast<LONGLONG,std::wstring>(script.GetAttrib(L"start"));
		end = boost::lexical_cast<LONGLONG,std::wstring>(script.GetAttrib(L"end"));

		scene_subtitles.push_back(SUBTITLE(text, start, end));
	}

	script.OutOfElem();
	return true;
}
/* Time update - checks wheather new subtitles arent required. */
bool VIDEO::UpdateSubtitles(){
	int last_active_subtitle = active_subtitle;
	active_subtitle = -1;

	int i = 0;
	for(std::vector<SUBTITLE>::iterator it = scene_subtitles.begin(); it != scene_subtitles.end(); it++, i++){
		if (it->time_start <= time_passed && it->time_end >= time_passed)
			active_subtitle = i;
	}
	if(active_subtitle != last_active_subtitle){
		DrawSubtitles(active_subtitle);
		BlendText();
	}
	return true;
}
/* Displeaying of the subtitle on the screen. */
bool VIDEO::DrawSubtitles(int subtitle_num){
	BMP_mix->Clear(key_color);
	// Thing of the editor
	top_pix = 5;

	// If there are no active subtitles, just clear and end
	if(subtitle_num == -1)
		return true;

	std::wstring text = scene_subtitles[subtitle_num].text;

	// used for obtaining parametres of the string
	bounding_rect.Width = bounding_rect.Height = 0;
	StatusLog("GraphicPath.AddString()", path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleRegular, text_size, Gdiplus::PointF(0, 0), &strformat), false);
	StatusLog("GraphicPath.AddString()", path.GetBounds(&bounding_rect, NULL, text_outline), false); 
	// Top left text corner (the hit position defines middle of the text)
	Gdiplus::REAL left, top;
	left = (static_cast<Gdiplus::REAL>(video_position.right) - bounding_rect.Width) / 2;
	top  = static_cast<Gdiplus::REAL>(video_position.bottom) - text_size*5;

	// Build the real text path
	StatusLog("GraphicPath.Reset()", path.Reset(), false); // remove temp path
	StatusLog("GraphicPath.AddString()", path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleBold, text_size, Gdiplus::PointF(left, top), &strformat), false);	
	
	// Draw on the screen
	StatusLog("Graphic.DrawPath()", BMP_mix->DrawPath(text_outline, &path), false);	
	StatusLog("Graphic.FillPath()", BMP_mix->FillPath(text_filler, &path), false);

	StatusLog("GraphicPath.Reset()", path.Reset(), false); // remove path
	return true;
}
/* Playback. */
HRESULT VIDEO::Play() {
	while (ShowCursor(false) >= 0) continue;
	return SCENE::Play();
}
/* Pause. */
HRESULT VIDEO::Pause() {
	while (ShowCursor(true) < 0) continue;
	return SCENE::Pause();
}
/* Left mouse click - finish the video. */
HRESULT VIDEO::LeftClick(const int x, const int y){
	PostSceneFinished(); return S_OK;
}
/* Right mouse click mouse click - nothing. */
HRESULT VIDEO::RightClick(const int x, const int y) {
	return S_OK;
}
/* Mouse move - nothing. */
HRESULT VIDEO::MouseMove(const int x, const int y) {
	return S_OK;
}
/* Checking the finish. */
HRESULT VIDEO::Update() {
	UpdateSubtitles();
	hr = video_media_seeking->GetPositions(&time_passed, &stop_time);
		HRLog("IMediaEvent.GetEvent", hr, false);
	if (time_passed >= stop_time){
		video_media_control->Stop();
		PostSceneFinished();
	}
	return hr;
}
/* Logging the event. */
HRESULT VIDEO::EventResponse() {
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

HRESULT VIDEO::FPressed() {
	fixed_subtitle = active_subtitle;
	std::wstring text(L"Fixed subtitle: ");
	text.append(boost::lexical_cast<std::wstring,int>(fixed_subtitle));
	DisplayText(text);
	return S_OK;
}

HRESULT VIDEO::NPressed() {
	DisplayText(L"New subtitle created");
	LONGLONG start, end;
	video_media_seeking->GetCurrentPosition(&start);
	end = start + seek;
	std::wstring text(L"Subtitle ");
	text.append(boost::lexical_cast<std::wstring, std::size_t>(scene_subtitles.size()));
	scene_subtitles.push_back(SUBTITLE(text, start, end));
	return S_OK;
}

HRESULT VIDEO::SPressed() {
	SCENE::SPressed();
	script.ResetPos();
	script.FindElem(L"SCENE");
	script.IntoElem();
	script.FindElem(L"SUBTITLES");
	script.IntoElem();
	while(script.FindElem(L"SUBTITLE"))
		script.RemoveElem();
	for(std::vector<SUBTITLE>::reverse_iterator it = scene_subtitles.rbegin(); it != scene_subtitles.rend(); it++){
		script.InsertElem(L"SUBTITLE", it->text);
		script.SetAttrib(L"start", it->time_start);
		script.SetAttrib(L"end", it->time_end);
	}

	script.Save(L"rough.xml");
	return S_OK;
}

HRESULT VIDEO::NumPressed(std::size_t number) {
	if (fixed_subtitle < 0)
		return S_FALSE;

	std::wstring text;
	switch (number) 
	{
		case 1:
			scene_subtitles[fixed_subtitle].time_start -= seek;
			text.append(L"Subtitle beggining -");
			text.append(boost::lexical_cast<std::wstring, LONGLONG>(seek));
			DisplayText(text);
		break;

		case 3:
			scene_subtitles[fixed_subtitle].time_start += seek;
			text.append(L"Subtitle beggining +");
			text.append(boost::lexical_cast<std::wstring, LONGLONG>(seek));
			DisplayText(text);
		break;

		case 4:
			scene_subtitles[fixed_subtitle].time_end -= seek;
			text.append(L"Subtitle end -");
			text.append(boost::lexical_cast<std::wstring, LONGLONG>(seek));
			DisplayText(text);
		break;

		case 6:
			scene_subtitles[fixed_subtitle].time_end += seek;
			text.append(L"Subtitle end +");
			text.append(boost::lexical_cast<std::wstring, LONGLONG>(seek));
			DisplayText(text);
		break;

		default:
			return S_FALSE;
		break;
	}
	return S_OK;
}