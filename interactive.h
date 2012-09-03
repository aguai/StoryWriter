#ifndef DEF_ENGINE_INTERACTIVE
#define DEF_ENGINE_INTERACTIVE

#include <dshow.h>
#include <D3D9.h>
#include <Vmr9.h>
#include <windows.h>
#include <gdiplus.h>
#include <map>
#include <string>
#include <vector>

class CMarkup;
class SCENE;
class STATES;

class INTERACTIVE : public SCENE {
private:

	// Object class
	class SCENE_OBJECT {
	private:
		friend class INTERACTIVE;

		Gdiplus::Region * position;
		Gdiplus::Point * points;
		int point_count;
		std::wstring name;
		std::wstring caption;
		std::string sound_path;
		std::string caption_sound_path;
		std::vector<state_changes> caused_changes; 
		bool finish_scene;
		bool is_new;

	public:	
		SCENE_OBJECT(const Gdiplus::GraphicsPath* boundaries, const std::wstring object_name, const std::wstring object_caption, 
					 const std::string object_sound_path, const std::string object_caption_sound_path, const std::vector<state_changes> & object_state_changes, const bool finisher);
		SCENE_OBJECT(const SCENE_OBJECT&);
		~SCENE_OBJECT();
	};

	// objects system
	std::vector<SCENE_OBJECT> objects;
	int last_active_object;
	bool caption_displayed;

	// Editor variables
	Gdiplus::Pen * outliner;
	Gdiplus::Pen * nowliner;
	Gdiplus::Point last_point;
	Gdiplus::Point * temp_points;
	int temp_points_count;

public:
	INTERACTIVE(const CMarkup*);
	~INTERACTIVE();
	bool CreateObjects(); // function creates objects from script - first it take names and than it creates path for a region and afterwards the map of state changes
	void DrawString(const Gdiplus::PointF & hit_position, const std::wstring & text);
	int  DetectCollision(const Gdiplus::PointF & hit_position);

	HRESULT Update();
	HRESULT LeftClick(const int x, const int y);
	HRESULT RightClick(const int x, const int y);
	HRESULT MouseMove(const int x, const int y);
	HRESULT EventResponse(); // Responses when video event occures

	// Editor functions
	void DrawOutlines();
	HRESULT DPressed();
	HRESULT FPressed();
	HRESULT NPressed();
	HRESULT SPressed();
};
#endif