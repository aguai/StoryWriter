#ifndef DEF_ENGINE_DIALOGUE
#define DEF_ENGINE_DIALOGUE

#include <map>
#include <string>
#include <vector>

class CMarkup;
class SCENE;
class STATES;

class DIALOGUE : public SCENE {
private:
	// CLASS for Option to choose from in conversation
	class OPTION {
	private:
		friend class DIALOGUE;

		Gdiplus::GraphicsPath * text_path;
		std::wstring text;
		std::vector<state_changes> caused_changes; 
	public:		
		OPTION(std::wstring & object_text, Gdiplus::GraphicsPath * object_path, std::vector<state_changes> & object_state_changes);
		OPTION(const OPTION& original);
		~OPTION();
	};

	std::vector<OPTION> options;
	int line_top; // top position of the bottom option line
	int line_height; // height of the single line
	int highlited_option;

public:
	DIALOGUE(const CMarkup*);
	~DIALOGUE();
	bool CreateOptions();
	bool DrawOptions();
	int  DetectCollision(const Gdiplus::PointF & hit_position);

	HRESULT LeftClick(const int x, const int y);
	HRESULT RightClick(const int x, const int y);
	HRESULT MouseMove(const int x, const int y);
	HRESULT Update();
	HRESULT EventResponse();
};
#endif