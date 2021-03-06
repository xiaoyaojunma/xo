#pragma once

#include "xoDefs.h"

/* A system window, or view.
TODO: Get rid of the ifdefs, and move them out into separate platform-specific implementations.
*/
class XOAPI xoSysWnd
{
public:
	enum SetPositionFlags
	{
		SetPosition_Move = 1,
		SetPosition_Size = 2,
	};
	enum CreateFlags
	{
		CreateMinimizeButton = 1,
		CreateMaximizeButton = 2,
		CreateCloseButton = 4,
		CreateBorder = 8,
		CreateDefault = CreateMinimizeButton | CreateMaximizeButton | CreateCloseButton | CreateBorder,
	};
#if XO_PLATFORM_WIN_DESKTOP
	HWND					SysWnd;
	uint					TimerPeriodMS;
	bool					QuitAppWhenWindowDestroyed;		// This is here for multi-window applications. Close the first window, and the app exits.
	enum WindowMessages
	{
		WM_XO_CURSOR_CHANGED = WM_USER,
	};
#elif XO_PLATFORM_ANDROID
	xoBox					RelativeClientRect;		// Set by XoLib_init
#elif XO_PLATFORM_LINUX_DESKTOP
	Display*				XDisplay;
	Window					XWindowRoot;
	//GLint					att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo*			VisualInfo;
	Colormap				ColorMap;
	Window					XWindow;
	GLXContext				GLContext;
	XEvent					Event;
#else
	XOTODO_STATIC;
#endif
	xoDocGroup*			DocGroup;
	xoRenderBase*		Renderer;

	xoSysWnd();
	~xoSysWnd();

	static xoSysWnd*	Create(uint createFlags = CreateDefault);
	static xoSysWnd*	CreateWithDoc(uint createFlags = CreateDefault);
	static void			PlatformInitialize();

	void	Attach(xoDoc* doc, bool destroyDocWithProcessor);
	void	Show();
	xoDoc*	Doc();
	bool	BeginRender();							// Basically wglMakeCurrent()
	void	EndRender(uint endRenderFlags);			// SwapBuffers followed by wglMakeCurrent(NULL). Flags are xoEndRenderFlags
	void	SurfaceLost();							// Surface lost, and now regained. Reinitialize GL state (textures, shaders, etc).
	void	SetPosition(xoBox box, uint setPosFlags);
	xoBox	GetRelativeClientRect();				// Returns the client rectangle (in screen coordinates), relative to the non-client window
	void	PostCursorChangedMessage();

protected:
	bool	InitializeRenderer();

	template<typename TRenderer>
	bool	InitializeRenderer_Any(xoRenderBase*& renderer);
};
