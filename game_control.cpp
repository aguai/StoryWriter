#include "header.h"

#define STATE std::pair<std::string, int>

extern HWND okno;
extern Gdiplus::Graphics * BMP;
extern Gdiplus::Graphics * SCR;
extern Gdiplus::CachedBitmap * cached_bitmap;
extern Gdiplus::Bitmap * memory_bitmap;

/* Constructor, also prepares external files. */
GAMECONTROL::GAMECONTROL(){
	scene_accesible = game_finished = new_game_started = false;
	total_game_time = 0;
	work_scene      = 0;
	script          = 0;
	video_paused    = true;

	script = new CMarkup;
		FunctionLog("Loading rough.xml failed. File is probably missing." , script->Load(L"rough.xml")  , true);	

	work_scene = new SCENE(); // Cannot be used
		FunctionLog("work_scene allocation failed, probably out of memory", (work_scene != 0), true);

	FunctionLog("Application couldn't find SCENE tag in rough.xml, file is probably damaged", script->FindElem(L"SCENE"), true);
	script->IntoElem();

	video_paused = !CreateScene();
}

/* Destructor. */
GAMECONTROL::~GAMECONTROL(){
	if(work_scene != 0){
		delete work_scene;
		work_scene = 0;
	}
	if(script != 0){
		delete script;
		script = 0;
	}
}

/* Building the scene at the position. */ 
bool GAMECONTROL::CreateScene(){	
	SCR->Clear(Gdiplus::Color(0,0,0)); // Covers what was left from previous session;
	
	if(work_scene != 0){
		delete work_scene;
		work_scene = 0;
	}

	FunctionLog("script->FindElem(L\"TYPE\")", "Scene type couldn't be resolved, <TYPE> tag is missing.", script->FindElem(L"TYPE"), true);

	if(script->GetData() == L"video"){
		Log("Creating VIDEO scene");		
		work_scene = new VIDEO(script);
	}
	else if(script->GetData() == L"dialogue"){
		Log("Creating DIALOGUE scene");		
		work_scene = new DIALOGUE(script);
	}
	else if(script->GetData() == L"interactive"){
		Log("Creating INTERACTIVE scene");		
		work_scene = new INTERACTIVE(script);	
	}
	if(work_scene == 0){
		TermLog("Application couldn't create next scene, file script.xml is probably damaged");
		return false;
	}

	return true;
}

/* Forced display of the current scene. */
bool GAMECONTROL::Display(){
	SCR->Clear(Gdiplus::Color(0,0,0));
	return !work_scene->Repaint();
}
/* Scene controll. */
bool GAMECONTROL::MouseMove(const int x, const int y){
	return !work_scene->MouseMove(x,y);
}
/* Scene controll. */
bool GAMECONTROL::LeftClick(const int x, const int y){
	return !work_scene->LeftClick(x,y);
}
/* Scene controll. */
bool GAMECONTROL::RightClick(const int x, const int y){
	return !work_scene->RightClick(x,y);
}
/* Scene controll. */
bool GAMECONTROL::Play(){
	return !work_scene->Play();
}
/* Scene controll. */
bool GAMECONTROL::Pause(){
	return !work_scene->Pause();
}
/* Scene controll - adds time to the counter. */
bool GAMECONTROL::Update(int milliseconds){
	if (!video_paused)
		work_scene->AddTime(milliseconds);
	return !work_scene->Update();
}
/* Scene controll. */
bool GAMECONTROL::VideoEventOccured(){
	return !work_scene->EventResponse();
}

bool GAMECONTROL::SpacePressed(){
	video_paused = !video_paused;
	if (video_paused){
		work_scene->Paused(true);
		return !work_scene->Pause();
	}
	else {
		work_scene->Paused(false);
		return !work_scene->Play();	
	}
}

bool GAMECONTROL::UpPressed(){
	return !work_scene->UpPressed();
}

bool GAMECONTROL::DownPressed(){
	return !work_scene->DownPressed();	
}

bool GAMECONTROL::LeftPressed(){
	video_paused = true;
	return !work_scene->LeftPressed();
}

bool GAMECONTROL::RightPressed(){
	video_paused = true;
	return !work_scene->RightPressed();	
}

bool GAMECONTROL::CPressed(){
	return !work_scene->CPressed();		
}

bool GAMECONTROL::DPressed(){
	return !work_scene->DPressed();		
}

bool GAMECONTROL::FPressed(){
	return !work_scene->FPressed();		
}

bool GAMECONTROL::NPressed(){
	return !work_scene->NPressed();		
}

bool GAMECONTROL::SPressed(){
	return !work_scene->SPressed();		
}

bool GAMECONTROL::TPressed(){
	return !work_scene->TPressed();		
}

bool GAMECONTROL::NumPressed(std::size_t number){
	return !work_scene->NumPressed(number);		
}