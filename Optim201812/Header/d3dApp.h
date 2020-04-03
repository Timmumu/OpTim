#include "OptimMain.h"
#include "GameTimer.h"
#include <string>
#include "d3dUtil.h"
#include <CommCtrl.h>


//include d3d11.lib so that no need to link the lib in setting
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")			//#include <d3dcompiler.h> in OptimMain.h  Automatically link with d3dcompiler.lib for D3DCompile() below.
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#pragma warning(disable:4100)	//disable warning in this header: unreferenced formal parameter

class D3DApp //基类
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;

	int Run();

	virtual bool Init() = 0;
	virtual void OnResize() = 0;

	virtual void DrawImgui() = 0;
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;
 
	virtual void BuildGeometryBuffers() = 0;		//car geometry
	virtual void StructureGeometryBuffers() = 0;
	virtual void FdtGeometryBuffers() = 0;

	virtual void ImportDLLL() = 0;
	virtual void ImportDLLLTest() = 0;

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT ChildMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	virtual void MouseMidDown(WPARAM btnState, int x, int y) { }
	virtual void MouseMidUp(WPARAM btnState, int x, int y) { }

	//imgui
	void EnableImgui() noexcept;
	void DisableImgui() noexcept;
	bool IsImguiEnabled() const noexcept;

	bool imguiEnabled = true;


protected:
	bool InitMainWindow();
	bool InitDirect3D();
	bool InitImgui();

	void CalculateFrameStats();

	void CleanupDevice();

	bool MouseWheelUp = false;
	bool MouseWheelDown = false;

	HINSTANCE mhAppInst;
	HWND      mhMainWnd;		//main window
	HWND	  DirectHwnd;		//child window

	bool      mAppPaused = false;
	bool      mMinimized = false;
	bool      mMaximized = false;
	bool      mResizing = false;
	UINT      m4xMsaaQuality = 0;

	GameTimer mTimer;

	ID3D11Device* md3dDevice;
	LPCTSTR DebuggerMarker;

	ID3D11DeviceContext* md3dImmediateContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	int mClientWidth;
	int mClientHeight;
	bool mEnable4xMsaa;

	// Our imgui state at the beginning
	bool show_demo_window = true;
	bool show_main_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);	//background color

	float slider1 = 0.0f;
	unsigned int ButtonCounter1 = 0;
};
