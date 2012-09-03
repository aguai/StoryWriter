#ifndef DEF_ENGINE_GAMECONTROL
#define DEF_ENGINE_GAMECONTROL

#include <string>
#include <fstream>

class SCENE;
class CMarkup;
class STATES;

class GAMECONTROL {
private:
	// File-associated variables
	CMarkup* script;
	std::wofstream save;

	// Game-associated variables
	SCENE*   work_scene;

	// System variables
	bool new_game_started; // True after game creation - blocks continuing from the savefile
	bool scene_accesible;  // If false, scene is currently building, or scene built failed
	bool game_finished;    // State at the end of the game

	// Editor variables
	bool video_paused;

	unsigned long total_game_time;

public:
	GAMECONTROL();
	~GAMECONTROL();

	bool Load();
	bool CreateScene(); // Builds scene from information on the current cursor postition	
	void SceneAccesible(const bool is_accesible) {scene_accesible = is_accesible;}
	bool IsSceneAccesible() {return scene_accesible;}
	bool IsGameCreated() {return new_game_started;}

	bool Display(); // Response to the WM_PAINT message
	bool MouseMove(const int x, const int y);
	bool LeftClick(const int x, const int y);
	bool RightClick(const int x, const int y);
	bool Pause();
	bool Play();
	bool Update(int milliseconds); // Controls finishing, updates time and scene
	bool VideoEventOccured(); // Used when message for video event is recognized	
	
	bool SpacePressed();
	bool UpPressed();
	bool DownPressed();
	bool LeftPressed();
	bool RightPressed();
	bool CPressed();
	bool DPressed();
	bool FPressed();
	bool NPressed();
	bool SPressed();
	bool TPressed();
	bool NumPressed(std::size_t);
};

#endif