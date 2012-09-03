#ifndef DEF_ENGINE_VIDEO
#define DEF_ENGINE_VIDEO
#include <dshow.h>
#include <vector>

class CMarkup;
class SCENE;

class VIDEO : public SCENE {
private:
	// Object class
	class SUBTITLE {
	private:
		friend class VIDEO;

		std::wstring text;
		LONGLONG time_start;
		LONGLONG time_end;

	public:	
		SUBTITLE(std::wstring sub_text, LONGLONG sub_start, LONGLONG sub_end);
		~SUBTITLE(){};
	};

	std::vector<SUBTITLE> scene_subtitles;
	int active_subtitle; // zero for none

	// Editor values
	int fixed_subtitle;

public:
	VIDEO(const CMarkup*);
	~VIDEO();

	bool CreateSubtitles();
	bool UpdateSubtitles();
	bool DrawSubtitles(int subtitle_num);

	HRESULT Play();
	HRESULT Pause();

	HRESULT LeftClick( const int x, const int y);
	HRESULT RightClick(const int x, const int y);
	HRESULT MouseMove( const int x, const int y);
	HRESULT Update();
	HRESULT EventResponse(); // Responses when video event occures

	// Editor functions
	HRESULT FPressed();
	HRESULT NPressed();
	HRESULT SPressed();
	HRESULT NumPressed(std::size_t);
};
#endif