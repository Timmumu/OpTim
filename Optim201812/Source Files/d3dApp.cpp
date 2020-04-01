#include "d3dApp.h"
#include "resource.h"
#include <WindowsX.h>
#include <sstream>

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gd3dApp->ChildMsgProc(hwnd, msg, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Import(HWND, UINT, WPARAM, LPARAM);

 

// Message handler for ImportE2k box.
INT_PTR CALLBACK ImportE2kBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

D3DApp::D3DApp(HINSTANCE hInstance)
	: mhAppInst(hInstance),
	mMainWndCaption(L"Optim"),
	md3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	mClientWidth(1200),
	mClientHeight(800),
	mEnable4xMsaa(false),
	mhMainWnd(0),		//parent window for MFC
	DirectHwnd(0),		//child window for directx rendering
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m4xMsaaQuality(0),

	md3dDevice(0),
	md3dImmediateContext(0),
	mSwapChain(0),
	mDepthStencilBuffer(0),
	mRenderTargetView(0),
	mDepthStencilView(0)
{
	//md3dDevice = 0???
	DebuggerMarker = L"DebuggerMarker is 1";

	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));

	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	gd3dApp = this;
}

D3DApp::~D3DApp()
{
	//destructor, remember to destroy created class
	//if (mRenderTargetView) mRenderTargetView->Release();
	DestroyWindow(mhMainWnd);
	ReleaseCom(mRenderTargetView);
	ReleaseCom(mDepthStencilView);
	ReleaseCom(mSwapChain);
	ReleaseCom(mDepthStencilBuffer);

	// Restore all default settings.
	if (md3dImmediateContext)
		md3dImmediateContext->ClearState();

	ReleaseCom(md3dImmediateContext);
	ReleaseCom(md3dDevice);
}

HINSTANCE D3DApp::AppInst()const
{
	return mhAppInst;
}

HWND D3DApp::MainWnd()const
{
	return mhMainWnd;
}

float D3DApp::AspectRatio()const
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

//Main Loop
int D3DApp::Run()
{
	MSG msg = { 0 };
	mTimer.Reset();

	//
	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}
		// Otherwise, do animation/game stuff.
		else
		{
			mTimer.Tick();
			if (!mAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());
				DrawImgui();		//virtual func in d3d, defined in OptimMain to pass some values
				DrawScene();		//main plot
			}
			else
			{
				//使用函数Sleep来暂停线程的执行。
				Sleep(100);
			}
		}
	}
	//CleanupDevice();
	return (int)msg.wParam;
}

bool D3DApp::Init()
{

	if (!InitMainWindow())	return false;

	if (!InitDirect3D())
	{
		OutputDebugStringW(L" InitDirect3D() in d3dApp.cpp failed.");
		return false;
	}

	if (!InitImgui())
	{
		OutputDebugStringW(L" InitImgui() in d3dApp.cpp failed.");
		return false;
	}
	return true;
}

void D3DApp::OnResize()
{
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	ReleaseCom(mRenderTargetView);
	ReleaseCom(mDepthStencilView);
	ReleaseCom(mDepthStencilBuffer);

	// Resize the swap chain and recreate the render target view.
	// (*mSwapChain).ResizeBuffers
	ThrowIfFailed(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer = 0;
	ThrowIfFailed(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	ThrowIfFailed(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));

	ReleaseCom(backBuffer);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ThrowIfFailed(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));

	// Create the depth stencil view
/*
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = depthStencilDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;*/
	ThrowIfFailed(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));

	// Bind the render target view and depth/stencil view to the pipeline.

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	// Set the viewport transform.
	mScreenViewport.TopLeftX = 120;
	mScreenViewport.TopLeftY = 40;
	mScreenViewport.Width = static_cast<float>(mClientWidth - 240);
	mScreenViewport.Height = static_cast<float>(mClientHeight - 80);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT D3DApp::ChildMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))	//plotted imgui's UI not function inside childWindow
	//	return true;
	 
	switch (message)
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	//Pause the app when the window is deactivated and unpause it 	 
	case WM_ACTIVATE:		
		if (LOWORD(wParam) == WA_INACTIVE)	
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else								// when it becomes active.
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the fresh client area dimensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;	 //end of WM_Size

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the fresh window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
		//MessageBox(0, L"WM_LBUTTONDOWN Clicked", L"FBI WARNING", MB_OK);
	case WM_LBUTTONUP:

	case WM_MBUTTONDOWN:
		MouseMidDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MBUTTONUP:
		MouseMidUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		if ((short)GET_WHEEL_DELTA_WPARAM(wParam) > 0)
		{
			MouseWheelUp = true;
		}
		if ((short)GET_WHEEL_DELTA_WPARAM(wParam) < 0)
		{
			MouseWheelDown = true;
		}

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(mhAppInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_DrawCar:
  			break;

		case IDM_ImportE2k:
			StructureGeometryBuffers();
			break;

		case IDM_ImportDLLL:
			ImportDLLL();
			break;

		case IDM_ImportDLLL_Test:
			ImportDLLLTest();
			break;

		case IDM_ImportWL:
			break;

		case IDM_ImportSF:
			FdtGeometryBuffers();
			break;

		case IDM_ImportE2k_Test:
			//DialogBox(mhAppInst, MAKEINTRESOURCE(IDD_ImportE2kBox), hWnd, ImportE2kBox);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
	}

	case WM_PAINT:
		/*hdc = BeginPaint(hWnd, &ps);		//childwindow's paint is not reuqired
		EndPaint(hWnd, &ps);*/
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);	//default case
}

LRESULT D3DApp::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))		//plotted imgui's UI only function inside Parent Window
		return true;

	switch (message)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the fresh client area dimensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;//end of WM_Size

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the fresh window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
		//MessageBox(0, L"WM_LBUTTONDOWN Clicked", L"FBI WARNING", MB_OK);
	case WM_LBUTTONUP:

	case WM_MBUTTONDOWN:
		MouseMidDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MBUTTONUP:
		MouseMidUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		if ((short)GET_WHEEL_DELTA_WPARAM(wParam) > 0)
		{
			MouseWheelUp = true;
		}
		if ((short)GET_WHEEL_DELTA_WPARAM(wParam) < 0)
		{
			MouseWheelDown = true;
		}


	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(mhAppInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_DrawCar:
		{
			BuildGeometryBuffers();

			std::wstringstream wtss(L"");
			wtss << "DrawCar Finished in line" << __LINE__;
			MessageBox(NULL, wtss.str().c_str(), L"FBI WARNING", MB_OK);
			break;
		}
		case IDM_ImportE2k:
			StructureGeometryBuffers();
			break;

		case IDM_ImportDLLL:
			ImportDLLL();
			break;

		case IDM_ImportDLLL_Test:
			ImportDLLLTest();
			break;

		case IDM_ImportWL:
			break;

		case IDM_ImportE2k_Test:
			//DialogBox(mhAppInst, MAKEINTRESOURCE(IDD_ImportE2kBox), hWnd, ImportE2kBox);
			break;

		case IDM_ImportSF:
			FdtGeometryBuffers();
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
	}

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);	//this paint is essential for whole project
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool D3DApp::InitMainWindow()
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(long);
	wcex.hInstance = mhAppInst;
	wcex.hIconSm = LoadIcon(mhAppInst, MAKEINTRESOURCE(IDI_HAMMER));
	wcex.hIcon = LoadIcon(mhAppInst, MAKEINTRESOURCE(IDI_HAMMER));
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_OPTIMV101);
	wcex.lpszClassName = L"D3DWndClassName";
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, L"RegisterClassEx Failed.", 0, 0);
		return false;
	}
	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };			//L, T, R, B
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"D3DWndClassName",
		mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, 0, 0, mhAppInst, this);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}
	::ShowWindow(mhMainWnd, SW_SHOW);
	::UpdateWindow(mhMainWnd);

	//Create Child Winodw for rendering the figures
	WNDCLASSEX wcexChild;
	wcexChild.cbSize = sizeof(WNDCLASSEX);
	wcexChild.style = CS_HREDRAW | CS_VREDRAW;
	wcexChild.lpfnWndProc = ChildWndProc;
	wcexChild.cbClsExtra = 0;
	wcexChild.cbWndExtra = sizeof(long);
	wcexChild.hInstance = mhAppInst;
	wcexChild.hIconSm = LoadIcon(mhAppInst, MAKEINTRESOURCE(IDI_HAMMER));
	wcexChild.hIcon = LoadIcon(mhAppInst, MAKEINTRESOURCE(IDI_HAMMER));
	wcexChild.hCursor = LoadCursor(0, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcexChild.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(0, 0, 0)));
	wcexChild.lpszMenuName = MAKEINTRESOURCEW(IDC_OPTIMV101);
	wcexChild.lpszClassName = L"ChildClassName";

	if (!RegisterClassEx(&wcexChild))
	{
		std::wstringstream wtss(L"");
		wtss << "RegisterClasswcexChild Failed in line" << __LINE__;	// << " in file " << __FILE__;
		MessageBox(NULL, wtss.str().c_str(), L"FBI WARNING", MB_OK);
		return false;
	}
	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT Rchild = { 100, 100, mClientWidth - 100, mClientHeight };			//L, T, R, B
	AdjustWindowRect(&Rchild, WS_OVERLAPPEDWINDOW, false);
	int widthChild = Rchild.right - Rchild.left;
	int heightChild = Rchild.bottom - Rchild.top;

	DirectHwnd = CreateWindow(L"D3DWndClassName",
		mMainWndCaption.c_str(),
		WS_CHILD | WS_VISIBLE | WS_CAPTION
		| WS_SYSMENU | WS_THICKFRAME
		| WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		widthChild, heightChild, mhMainWnd, 0, mhAppInst, 0);

	if (!DirectHwnd)
	{
		std::wstringstream wtss(L"");
		wtss << "CreateChildWindow Failed in line" << __LINE__;		// << " in file " << __FILE__;
		MessageBox(NULL, wtss.str().c_str(), L"FBI WARNING", MB_OK);
		return false;
	}
	::ShowWindow(DirectHwnd, SW_SHOW);
	::UpdateWindow(DirectHwnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	// Create the device and device context.
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                 // default adapter
		md3dDriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&md3dDevice,
		&featureLevel,
		&md3dImmediateContext);

	//debugging tracing
	/*DebuggerMarker = L"DebuggerMarker is 2";
	OutputDebugStringW(DebuggerMarker);*/

	if (FAILED(hr))
	{
		std::wstringstream wtss(L"");
		wtss << "Optim: D3D11CreateDevice Failed in line" << __LINE__;		// << " in file " << __FILE__;
		MessageBox(NULL, wtss.str().c_str(), L"FBI WARNING", MB_OK);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		std::wstringstream wtss(L"");
		wtss << "Optim: Direct3D Feature Level 11 unsupported in line" << __LINE__;		// << " in file " << __FILE__;
		MessageBox(NULL, wtss.str().c_str(), L"FBI WARNING", MB_OK);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	ThrowIfFailed(md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
	assert(m4xMsaaQuality > 0);

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));							// clear out the struct for use

	sd.BufferDesc.Width = mClientWidth;										// set the back buffer width, height
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// use 32-bit color
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if (mEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count = 1;											// how many multisamples
		sd.SampleDesc.Quality = 0;											// multisample quality level
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						// how swap chain is to be used
	sd.BufferCount = 1;														// one back buffer
	sd.OutputWindow = DirectHwnd;											// bind d3d with which window
	sd.Windowed = true;														//allow windowed/full-screen mode
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//sd.Flags = 0;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;						// allow full-screen switching  //Alt-Enter to switch. TODO:Failed		

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	ThrowIfFailed(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)& dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	ThrowIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)& dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)& dxgiFactory));

	ThrowIfFailed(dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain));

	ReleaseCom(dxgiDevice);
	ReleaseCom(dxgiAdapter);
	ReleaseCom(dxgiFactory);

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.

	//bind the ImGui with the d3d

	OnResize();

	DebuggerMarker = L"DebuggerMarker is 3";
	OutputDebugStringW(DebuggerMarker);

	return true;
}


bool D3DApp::InitImgui()
{
	// Setup Dear ImGui context for child window 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(DirectHwnd);
	ImGui_ImplDX11_Init(md3dDevice, md3dImmediateContext);
	return true;
}

void D3DApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// can be appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages in per one second period
	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f)		// if time elapsed > 1 second
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << mMainWndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(DirectHwnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void D3DApp::CleanupDevice()
{
	if (md3dImmediateContext) md3dImmediateContext->ClearState();
	if (md3dDevice) md3dDevice->Release();
	if (md3dImmediateContext) md3dImmediateContext->Release();
	if (mSwapChain) mSwapChain->Release();
	if (mRenderTargetView) mRenderTargetView->Release();
	if (mDepthStencilView) mDepthStencilView->Release();

	// Cleanup imgui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	DestroyWindow(mhMainWnd);
	DestroyWindow(DirectHwnd);

}


void D3DApp::EnableImgui() noexcept
{
	imguiEnabled = true;
}

void D3DApp::DisableImgui() noexcept
{
	imguiEnabled = false;
}

bool D3DApp::IsImguiEnabled() const noexcept
{
	return imguiEnabled;
}