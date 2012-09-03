#ifndef DEF_ENGINE_SCENE
#define DEF_ENGINE_SCENE

#include <dshow.h>
#include <D3D9.h>
#include <Vmr9.h>
#include <windows.h>
#include <gdiplus.h>
#include <string>

class SCENE {
private:

protected:
	// Copies of controller properties
	CMarkup script;	
	// Video stream path
	std::wstring source_path;

	// system control
	HRESULT hr;	
	Gdiplus::Status stat;
	std::string fail_msg; // Used during building of the Scene
	bool graphs_built, audio_built, file_rendered, video_clipped, blending_setup, text_setup, scene_created;
	RECT video_position;
	long video_width;
	long video_height;
	double video_scale;
	LONGLONG time_passed;
	LONGLONG stop_time;

	// Video interfaces
	IGraphBuilder          *video_graph;
    IMediaControl          *video_media_control;
	IMediaSeeking		   *video_media_seeking;
    IMediaEventEx          *video_media_event;
	IBaseFilter            *video_mixer;
	ICaptureGraphBuilder2  *capture_graph;
	IVMRWindowlessControl9 *windowless_control;
	IVMRMixerBitmap9       *bitmap_mixer;

	// Additional audio interfaces
	IGraphBuilder *audio_graph;
	IMediaControl *audio_media_control;
	IMediaEvent   *audio_media_event;

	// Blending objects
	Gdiplus::Graphics * BMP_mix;
	Gdiplus::Bitmap * drawing_bitmap;
	HDC bitmap_hdc;
	VMR9AlphaBitmap bmpInfo;

	// text's objects
	Gdiplus::FontFamily*  fontFamily;
	Gdiplus::Pen*		  text_outline; 
	Gdiplus::SolidBrush*  text_filler;
	Gdiplus::SolidBrush*  dark_filler;
	Gdiplus::StringFormat strformat;
	Gdiplus::GraphicsPath path;
	Gdiplus::Rect		  bounding_rect;

	// editor values
	LONGLONG seek;
	int top_pix;
	bool video_paused;

public:
	SCENE();
	SCENE(const CMarkup * original_script);
	virtual ~SCENE();

	bool BuildGraphs();
	bool BuildAudio();
	bool InitWindowlessVMR();
	bool RenderFile();
	bool ClipVideo();
	bool SetupTextBlending();
	bool SetupTextProperties();
	bool BlendText();

	virtual HRESULT Play();
	virtual HRESULT Pause();
	virtual HRESULT Repaint();

	virtual HRESULT LeftClick(const int x, const int y) {return hr;}
	virtual HRESULT RightClick(const int x, const int y) {return hr;}
	virtual HRESULT MouseMove(const int x, const int y) {return hr;}
	virtual HRESULT Update() {return hr;}
	virtual HRESULT EventResponse() {return hr;}

	void AddTime(int milliseconds) {}
	void PostSceneFinished();
	void PostSceneCreated();
	bool CheckTests();
	bool SceneCreated() {return scene_created;}

	void Paused(bool paused) {video_paused = paused;}
	HRESULT UpPressed();
	HRESULT DownPressed();
	HRESULT LeftPressed();
	HRESULT RightPressed();
	HRESULT CPressed();
	virtual HRESULT DPressed();
	virtual HRESULT FPressed();
	virtual HRESULT NPressed();
	virtual HRESULT SPressed();
	virtual HRESULT NumPressed(std::size_t);
	HRESULT TPressed();
	bool TextOut(std::wstring);
	void DisplayText(std::wstring);
};

#endif