#ifndef D3DAPP_H
#define D3DAPP_H

#include "OptimMain.h"
#include "AppTimer.h"
#include <string>
#include "d3dUtil.h"
#include <CommCtrl.h>
#include "DXTrace.h"

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

	virtual void PlanView() = 0;				//default make it Base Floor
	virtual void ThreeDimView() = 0;			//once clicked, recover to 3D view

	virtual void DrawImgui() = 0;
	virtual void UpdateScene(float dt) = 0;		// 子类需要实现该方法，完成每一帧的更新
	virtual void DrawScene() = 0;				// 子类需要实现该方法，完成每一帧的绘制
 
	virtual void BuildGeometryBuffers() = 0;		//car geometry
	virtual void ImportE2k() = 0;				//assign value to vertex and create vertex buffer
	virtual void FdtGeometryBuffers() = 0;

	virtual void ImportDLLL() = 0;
	virtual void ImportDLLLTest() = 0;

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	// 窗口的消息回调函数
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

	void CalculateFrameStats();				// 计算每秒帧数并在窗口显示

	void CleanupDevice();

	bool MouseWheelUp = false;
	bool MouseWheelDown = false;

	HINSTANCE mhAppInst;			// 应用实例句柄
	HWND      mhMainWnd;			//main window 主窗口句柄
	HWND	  DirectHwnd;			//child window

	bool      mAppPaused = false;	// 应用是否暂停
	bool      mMinimized = false;
	bool      mMaximized = false;
	bool      mResizing = false;	// 窗口大小是否变化
	UINT      m4xMsaaQuality = 0;	// MSAA支持的质量等级

	AppTimer mTimer;				// 计时器

	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	ComPtr<ID3D11Device> md3dDevice;					// 顶点缓冲区数组
	ComPtr<ID3D11DeviceContext> md3dImmediateContext;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;
	ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	LPCTSTR DebuggerMarker;								//useless

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
	unsigned int PlanViewClicked = 0;

};


#endif					//d3dApp.h