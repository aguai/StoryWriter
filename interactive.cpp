#include "header.h"
#include "Evcode.h"

extern HWND okno;

// Functions regarding the SCENE_OBJECT class------------------------------------
/* Constructor. */
INTERACTIVE::SCENE_OBJECT::SCENE_OBJECT(const Gdiplus::GraphicsPath* boundaries, const std::wstring object_name, 
										const std::wstring object_caption, const std::string object_sound_path, 
									    const std::string object_caption_sound_path, const std::vector<state_changes> & object_state_changes, 
										const bool finisher) {
	position = 0;
	position = new Gdiplus::Region(boundaries); 
	point_count = boundaries->GetPointCount();
	points = new Gdiplus::Point[point_count];
	boundaries->GetPathPoints(points,point_count); 
	if(position == 0)
		Log("Unable to create region for object");
	name = object_name; 
	caption = object_caption; 
	sound_path = object_sound_path; 
	caption_sound_path = object_caption_sound_path;
	caused_changes = object_state_changes; 
	finish_scene = finisher;
	is_new = false;
}
/* Copy constructor. */
INTERACTIVE::SCENE_OBJECT::SCENE_OBJECT(const SCENE_OBJECT& image) {
	position = 0;
	position = image.position->Clone();
	point_count = image.point_count;
	points = new Gdiplus::Point[point_count];
	for (int i = 0; i < point_count; i++)
		points[i] = image.points[i];
	if(position == 0)
		TermLog("Unable to create region for object");
	name = image.name;
	caption = image.caption;
	sound_path = image.sound_path;
	caption_sound_path = image.caption_sound_path;
	caused_changes = image.caused_changes;
	finish_scene = image.finish_scene;
	is_new = image.is_new;
}
/* Destructor. */
INTERACTIVE::SCENE_OBJECT::~SCENE_OBJECT() {
	// Primitive destructor
	caused_changes.clear(); 
	if(position != 0)
		delete position;
	if(points != 0)
		delete points;
	position = 0;
	points = 0;
}
// Functions regarding the INTERACTIVE class-------------------------------------
/* Constructor. */
INTERACTIVE::INTERACTIVE(const CMarkup * original_script) : SCENE(original_script) {	
	last_active_object = 0;
	caption_displayed  = false;
	
	if(!scene_created)
		return;
	if(!CreateObjects())
		return;
	if(Play() != S_OK)
		return;

	// Editor setup
	outliner = new Gdiplus::Pen(Gdiplus::Color(0,255,0),1.5f);
	nowliner = new Gdiplus::Pen(Gdiplus::Color(0,0,255),1.5f);
	temp_points_count = 0;
	temp_points = new Gdiplus::Point[1000];
	last_point = Gdiplus::Point(0,0);
	DrawOutlines();
	BlendText();

	PostSceneCreated();	
}
/* Destructor. */
INTERACTIVE::~INTERACTIVE() {
	delete outliner;
	delete nowliner;
	delete [] temp_points;
	objects.clear();
}
/* Function creates objects from script - first it take names and than it creates path for a region and afterwards the map of state changes. */
bool INTERACTIVE::CreateObjects() {
	// attributes
	std::wstring name;
	std::wstring caption;
	std::string sound_path;
	std::string caption_sound_path;
	bool finish_scene;

	// object's space
	Gdiplus::GraphicsPath boundaries;
	int verticle_count;
	Gdiplus::Point * verticles;
	int x, y;

	// caused changes
	std::string state_name;
	std::vector<state_changes> changes; 

	// control
	bool verticles_OK = true;

	// Tag is not necessary for the editor
	if(!script.FindElem(L"OBJECTS"))
		return true;
	script.IntoElem();

	// Tag is not necessary for the editor
	if(!script.FindElem(L"OBJECT")){
		script.OutOfElem();
		return true;
	}
	else do {
		// atributes reading
		name = script.GetAttrib(L"name");
		caption = script.GetAttrib(L"caption");
		sound_path = wstring2string(script.GetAttrib(L"sound_source"));
		caption_sound_path = wstring2string(script.GetAttrib(L"caption_sound")); 
		verticle_count = atoi((wstring2string(script.GetAttrib(L"v_count"))).c_str());
		if(verticle_count < 3){
			Log("Verticle count in one of the objects is too low");
			verticles_OK = false;
		} 
		finish_scene = (atoi((wstring2string(script.GetAttrib(L"finish_scene"))).c_str())) != 0 ? true : false;
		
		script.IntoElem(); // Jumps into the object

		if(!CheckTests()){
			script.OutOfElem();
			continue;
		}

		// creates verticles for the object polygon
		verticles = new Gdiplus::Point [verticle_count];
		for(int i = 0; i < verticle_count; i++){
			if(!script.FindElem(L"VERTEX")){
				Log("There is not responding count of vertices in one object in script.");
				verticles_OK = false;
			}
			x = atoi((wstring2string(script.GetAttrib(L"x"))).c_str()); 
			y = atoi((wstring2string(script.GetAttrib(L"y"))).c_str()); 
			verticles[i] = Gdiplus::Point(x, y);
			
		}
		if(!verticles_OK){
			delete [] verticles;
			script.OutOfElem();
			continue;
		}
		else{
			boundaries.AddPolygon(verticles, verticle_count);
			delete [] verticles;
		}

		// creates the vector of state changes
		while(script.FindElem(L"STATECHANGE")){
			state_changes current_changes;
			current_changes.name = wstring2string(script.GetData());
			current_changes.value = atoi((wstring2string(script.GetAttrib(L"value"))).c_str());
			current_changes.replace = atoi((wstring2string(script.GetAttrib(L"replace"))).c_str())  != 0 ? true : false;
			changes.push_back(current_changes);
		}
		
		script.OutOfElem(); // Jumps out of the object

		objects.push_back(SCENE_OBJECT(&boundaries, name, caption, sound_path, caption_sound_path, changes, finish_scene));
		boundaries.Reset();
		changes.clear();

	} while(script.FindElem(L"OBJECT"));

	script.OutOfElem();
	return true;
}
/* Function displays strings of the text, when needed. */
void INTERACTIVE::DrawString(const Gdiplus::PointF & hit_position, const std::wstring & text) {
	/*Finds out the most centered position, still within the video window. Uses some fixed corrections*/

	// used for obtaining parametres of the string
	bounding_rect.Width = bounding_rect.Height = 0;
	StatusLog("GraphicPath.AddString()", path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleRegular, text_size, Gdiplus::PointF(0, 0), &strformat), false);
	StatusLog("GraphicPath.AddString()", path.GetBounds(&bounding_rect, NULL, text_outline), false); 
	// Top left text corner (the hit position defines middle of the text)
	Gdiplus::REAL left, top;
	left = hit_position.X - (bounding_rect.Width)/2; 
	top  = hit_position.Y - (bounding_rect.Height)/2 + text_size/4;

	// Bounds correction
	if(left < 0) 
		left = static_cast<Gdiplus::REAL>(video_position.left) - text_size/6;
	if(left + bounding_rect.Width > static_cast<Gdiplus::REAL>(video_position.right)) 
		left = static_cast<Gdiplus::REAL>(video_position.right) - bounding_rect.Width + text_size/2;
	if(top  < 0) 
		top = static_cast<Gdiplus::REAL>(video_position.top);
	if(top + bounding_rect.Height > static_cast<Gdiplus::REAL>(video_position.bottom)) 
		top = static_cast<Gdiplus::REAL>(video_position.bottom) - bounding_rect.Height;
	
	// Build the real text path
	StatusLog("GraphicPath.Reset()", path.Reset(), false); // remove temp path
	StatusLog("GraphicPath.AddString()", path.AddString(text.c_str(), -1, fontFamily, Gdiplus::FontStyleBold, text_size, Gdiplus::PointF(left, top), &strformat), false);	
	
	// Draw on the screen
	StatusLog("Graphic.DrawPath()", BMP_mix->DrawPath(text_outline, &path), false);	
	StatusLog("Graphic.FillPath()", BMP_mix->FillPath(text_filler, &path), false);

	StatusLog("GraphicPath.Reset()", path.Reset(), false); // remove path
}
/* Provided the scene finished, system restarts the video. */
HRESULT INTERACTIVE::Update() {
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
/* Left click - acivates the object clicked on. */
HRESULT INTERACTIVE::LeftClick(const int x, const int y) {
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	if(i){
		// Play audio
		hr = audio_media_control->Stop();
		std::string source = objects[i-1].sound_path;
		hr = audio_graph->RenderFile(string2wstring(source).c_str(), NULL);
			HRLog("IGraphBuilder.RenderFile() - additional audio", hr, false);
		hr = audio_media_control->Run();
		if(hr){
			OAFilterState filter_state;
			hr = video_media_control->GetState(INFINITE, &filter_state);
				HRLog("IMediaControl.GetState - additional audio play", hr, false);
		}		
	}
	return hr;
}
/* Right click - get info about the object. */
HRESULT INTERACTIVE::RightClick(const int x, const int y) {
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	if(i){
		// Play audio
		hr = audio_media_control->Stop();
		std::string source = objects[i-1].caption_sound_path;
		hr = audio_graph->RenderFile(string2wstring(source).c_str(), NULL);
			HRLog("IGraphBuilder.RenderFile() - additional audio", hr, false);
		hr = audio_media_control->Run();
		if(hr){
			OAFilterState filter_state;
			hr = video_media_control->GetState(INFINITE, &filter_state);
				HRLog("IMediaControl.GetState - additional audio play", hr, false);
		}

		// Display the caption
		caption_displayed = true;
		BMP_mix->Clear(key_color); // Bitmap for video mixing
		DrawString(hit_position, objects[i-1].caption.c_str());
	}	
	DrawOutlines();
	BlendText();
	return hr;
}
/* Response to the move of the cursor - if there is a object hit, object is captioned. */
HRESULT INTERACTIVE::MouseMove(const int x,const int y) {
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	last_point.X = hit_position.X;
	last_point.Y = hit_position.Y;
	int i = DetectCollision(hit_position);

	if(i != 0 && i != last_active_object){
		last_active_object = i;
		caption_displayed = false;

		BMP_mix->Clear(key_color);
		DrawString(hit_position, objects[i-1].name.c_str());
		DrawOutlines();
		BlendText();
	}	
	else if (last_active_object != 0 && i == 0 && caption_displayed != true){	
		BMP_mix->Clear(key_color);
		last_active_object = 0;	
	DrawOutlines();
		BlendText();
	}
	return S_OK;
}
/* Logging of the event. */
HRESULT INTERACTIVE::EventResponse() {
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
/* Finds out wheather there is a collision with any object. */
int INTERACTIVE::DetectCollision(const Gdiplus::PointF & hit_position) {
	unsigned int i = 1; // i stores number of the intersecting object, if i is equal to number of objects, hit wasn't detected -> returns 0
	for(std::vector<SCENE_OBJECT>::iterator it = objects.begin(); it < objects.end(); it++){
		if (!it->is_new)
			if (it->position->IsVisible(hit_position))
				break;
		i++;
	}
	return (i != (objects.size() + 1) ) ? i : 0;	
}

void INTERACTIVE::DrawOutlines() {
	LONGLONG current; video_media_seeking->GetCurrentPosition(&current); 
	if (video_paused){
		current -= 1;
		video_media_seeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &current, AM_SEEKING_NoPositioning);
		current += 1;
	}

	for(std::vector<SCENE_OBJECT>::iterator it = objects.begin(); it < objects.end(); it++)
		BMP_mix->DrawPolygon(outliner, it->points, it->point_count);
	BMP_mix->DrawLines(nowliner, temp_points, temp_points_count);

	if(video_paused)
		video_media_seeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &current, AM_SEEKING_NoPositioning);
}

/*End pointing an object.*/
HRESULT INTERACTIVE::DPressed() {
	if (temp_points_count <= 0)
		return S_FALSE;
	temp_points_count--;
	BMP_mix->Clear(key_color);
	DrawOutlines();
	BlendText();
	return S_OK;
}
/*Fix current cursor point.*/
HRESULT INTERACTIVE::FPressed() {
	if(temp_points_count <= 2) {
		DisplayText(L"No polygon exists");
		return S_FALSE;	
	}
	Gdiplus::GraphicsPath boundaries;
	boundaries.AddPolygon(temp_points, temp_points_count);
	std::wstring name(L"Object ");
	name.append(boost::lexical_cast<std::wstring, std::size_t>(objects.size()));
	std::vector<state_changes> changes;
	state_changes change = {"No_change", 0, 0};
	changes.push_back(change);
	objects.push_back(SCENE_OBJECT(&boundaries, name, L"!caption!", "!sound_path!", "!caption_sound_path!", changes, false));
	temp_points_count = 0;
	name.append(L" saved.");
	BMP_mix->Clear(key_color);
	DrawOutlines();
	DisplayText(name);
	return S_OK;
}
/*Starts pointing a new object.*/
HRESULT INTERACTIVE::NPressed() {
	temp_points[temp_points_count] = last_point;
	temp_points_count++;
	BMP_mix->Clear(key_color);
	DrawOutlines();
	BlendText();
	return S_OK;
}
/*Save the current objects to the file.*/
HRESULT INTERACTIVE::SPressed() {
	SCENE::SPressed();
	script.ResetPos();
	script.FindElem(L"SCENE");
	script.IntoElem();
	script.FindElem(L"OBJECTS");
	script.IntoElem();
	while(script.FindElem(L"OBJECT"))
		script.RemoveElem();
	for(std::vector<SCENE_OBJECT>::reverse_iterator it = objects.rbegin(); it != objects.rend(); it++){
		script.InsertElem(L"OBJECT", L"");
		script.SetAttrib(L"name", it->name);
		script.SetAttrib(L"caption", it->caption);
		script.SetAttrib(L"sound_source", string2wstring(it->sound_path));
		script.SetAttrib(L"caption_sound", string2wstring(it->caption_sound_path));
		script.SetAttrib(L"v_count", boost::lexical_cast<std::wstring, int>(it->point_count));
		script.SetAttrib(L"finish_scene", boost::lexical_cast<std::wstring, int>(static_cast<int>(it->finish_scene)));
		script.IntoElem();
		for (int i = (it->point_count-1); i >= 0; i--){
			script.InsertElem(L"VERTEX", L"");
			script.SetAttrib(L"x", it->points[i].X);
			script.SetAttrib(L"y", it->points[i].Y);
		}
		script.OutOfElem();
	}

	script.Save(L"rough.xml");
	return S_OK;	
}