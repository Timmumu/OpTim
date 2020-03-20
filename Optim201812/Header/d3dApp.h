#include "OptimMain.h"
#include "GameTimer.h"
#include <string>
#include "d3dUtil.h"
#include <CommCtrl.h>

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
	virtual void UpdateScene(float dt ) = 0;
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

protected:
	bool InitMainWindow();
	void CreateButton();
	void CreateScrollbar();
	void CreateStatic();		//Text
	void CreateCombobox();

	bool InitDirect3D();
	
	void CalculateFrameStats();
	
	void CleanupDevice();

protected:

	bool MouseWheelUp = false;
	bool MouseWheelDown = false;

	HINSTANCE mhAppInst;
	HWND      mhMainWnd;
	HWND	  DirectHwnd;

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

};
