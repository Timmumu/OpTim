//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729723.aspx
//
// uploaded to Timu's Github https://github.com/Timmumu/20180608
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <Time.h>

#include "OptimMain.h"
#include "resource.h"
#include "Vertex.h"
#include "MathHelper.h"
#include "d3dApp.h"
#include "Effects.h"
#include "Camera.h"
#include "xnacollision.h"

#pragma warning(disable:4996)	//This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
 
using namespace std;


//用struct定义派生类，默认的继承方式为public
struct InstancedData
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4 Color;
};

struct LineDirectx
{	//line所在层， 起点，终点， 起点索引，终点索引，跨层数
	std::string LineName, Story ;
	std::string LineStart , LineEnd;
	UINT LineSIndex = 0;
	UINT LineEIndex = 0;
	UINT SpanFloor = 0;
};

// 3 types of variables: string, UINT and float, declare once, but initialization is required for each
struct Story { std::string StoryName;  float StoryHeight = 0.0; float TotalHeight = 0.0f; };
struct PointCoord { std::string PtName; float PtX = 0.0f; float PtY = 0.0f; };
struct Point3D { std::string PtName; float PtX = 0.0f; float PtY = 0.0f; float PtZ = 0.0f; };
struct LineConnect { std::string LineName, LineType; string LineStart; string LineEnd ; UINT SpanFloor = 0; };
struct LineAssign { std::string LineName; string Story; string LineStart; string LineEnd; UINT SpanFloor = 0;};
struct MaterialProp { std::string Name , Mass , Weight, ModulusE , PossionR ; };

//Beam Forces
struct Beam 
{
	std::string Floor;
	std::string Name;
	std::string LdCase;
	float Loc = 0.0f, P = 0.0f, V2 = 0.0f, V3 = 0.0f, T = 0.0f, M2 = 0.0f, M3 = 0.0f;
};

//用class定义派生类，默认的继承方式是private
//		派生类名 : 继承方式 基类名(Base)
class OptimMain : public D3DApp
	//基类的private成员在派生类中不可直接访问
	//基类的protected成员在派生类中为protected访问属性
	//基类的public成员在派生类中为public访问属性
{
public:
	OptimMain(HINSTANCE hInstance);
	~OptimMain();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);

	void DrawScene();		//Example

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	void MouseMidDown(WPARAM btnState, int x, int y);
	void MouseMidUp(WPARAM btnState, int x, int y);

private:

	void BuildGeometryBuffers();
	void BuildInstancedBuffer();

	void StructureGeometryBuffers();
	void FdtGeometryBuffers();

	void ImportDLLL();
	void ImportDLLLTest();
	
	void ImportDLLL_MMAP();

private:
	ID3D11Buffer* mTargetVB = 0;
	ID3D11Buffer* mTargetIB = 0;
	ID3D11Buffer* mInstancedBuffer = 0;			//for 动态缓冲

	// Bounding box of the skull.
	XNA::AxisAlignedBox mSkullBox;
	XNA::Frustum mCamFrustum;

	UINT mVisibleObjectCount;

	// Keep a system memory copy of the world matrices for culling.
	std::vector<InstancedData> mInstancedData;

	bool mFrustumCullingEnabled;

	DirectionalLight mDirLights[3];
	Material mSkullMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mSkullWorld;

	//UINT mSkullIndexCount;
	UINT mIndexCount = 0;

	Camera mCam;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory leaks check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	//_CrtSetBreakAlloc(148); //Set breakpoint to locate the memory leak code's line 

	//declare theApp
	OptimMain theApp(hInstance);

	if (!theApp.Init())
	{
		OutputDebugStringW(L" theApp.Init() in OptimMain.cpp failed.");
		return 0;
	}
	return theApp.Run();
}

OptimMain::OptimMain(HINSTANCE hInstance)
	: D3DApp(hInstance),
	mTargetVB(0), mTargetIB(0),
	mIndexCount(0), mInstancedBuffer(0),
	mVisibleObjectCount(0), mFrustumCullingEnabled(true)
{
	//change App's Caption 1st time
	mMainWndCaption = L"Optim";

	srand((unsigned int)time((time_t *)NULL));

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	XMMATRIX I = DirectX::XMMatrixIdentity();

	XMMATRIX skullScale = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = DirectX::XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	DirectX::XMStoreFloat4x4(&mSkullWorld, DirectX::XMMatrixMultiply(skullScale, skullOffset));

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

OptimMain::~OptimMain()
{
	ReleaseCom(mTargetVB);
	ReleaseCom(mTargetIB);
	ReleaseCom(mInstancedBuffer);

	
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool OptimMain::Init()
{
	if (!D3DApp::Init())
	{	
		OutputDebugStringW(L" D3DApp::Init() in OptimMain.cpp failed.");
		return false;
	}

	// Must init Effects in prior to InputLayouts, the later depends on Effects' shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	BuildInstancedBuffer();

	return true;
}

void OptimMain::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	// Build the frustum from the projection matrix in view space.
	ComputeFrustumFromProjection(&mCamFrustum, &mCam.Proj());
}

void OptimMain::UpdateScene(float dt)
{
	// Control the camera. use W,S,A,D to Translation
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(500.0f*dt);
	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-500.0f*dt);
	if (GetAsyncKeyState('A') & 0x8000)
		mCam.RightLeft(500.0f*dt);
	if (GetAsyncKeyState('D') & 0x8000)
		mCam.RightLeft(-500.0f*dt);
	if (GetAsyncKeyState('1') & 0x8000)
		mFrustumCullingEnabled = true;
	if (GetAsyncKeyState('2') & 0x8000)
		mFrustumCullingEnabled = false;

	if (GetAsyncKeyState(VK_UP) & 0x8000)
		//mCam.Walk(500.0f*dt);			//zoom in and out
		mCam.UpDown(500.0f*dt);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		//mCam.Walk(-500.0f*dt);
		mCam.UpDown(-500.0f*dt);		//translate up and down
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		mCam.RightLeft(500.0f*dt);		//translate left and right
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		mCam.RightLeft(-500.0f*dt);

	if (MouseWheelUp)
	{
		mCam.Walk(50000.0f*dt);
		MouseWheelUp = false;
	}
	if (MouseWheelDown)
	{
		mCam.Walk(-50000.0f*dt);
		MouseWheelDown = false;
	}

	//
	// Perform frustum culling.
	//
	mCam.UpdateViewMatrix();
	mVisibleObjectCount = 0;

	if (mFrustumCullingEnabled)
	{
		XMVECTOR detView = DirectX::XMMatrixDeterminant(mCam.View());
		XMMATRIX invView = DirectX::XMMatrixInverse(&detView, mCam.View());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			XMMATRIX W = DirectX::XMLoadFloat4x4(&mInstancedData[i].World);
			XMMATRIX invWorld = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(W), W);

			// View space to the object's local space.
			XMMATRIX toLocal = DirectX::XMMatrixMultiply(invView, invWorld);

			// Decompose the matrix into its individual parts.
			XMVECTOR scale;
			XMVECTOR rotQuat;
			XMVECTOR translation;
			DirectX::XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

			// Transform the camera frustum from view space to the object's local space.
			XNA::Frustum localspaceFrustum;
			XNA::TransformFrustum(&localspaceFrustum, &mCamFrustum,
				XMVectorGetX(scale), rotQuat, translation);

			// Perform the box/frustum intersection test in local space.
			if (XNA::IntersectAxisAlignedBoxFrustum(&mSkullBox, &localspaceFrustum) != 0)
			{
				// Write the instance data to dynamic VB of the visible objects.
				dataView[mVisibleObjectCount++] = mInstancedData[i];
			}
		}
		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
	}
	else // No culling enabled, draw all objects.
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			dataView[mVisibleObjectCount++] = mInstancedData[i];
		}
		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
	}

	std::wostringstream outs;
	outs.precision(6);
	outs << L"OptimMain" <<
		L"    " << mVisibleObjectCount <<
		L" objects visible out of " << mInstancedData.size();
	//change App's Caption 2nd time
	mMainWndCaption = outs.str();
}

void OptimMain::DrawScene()
{	
	// imgui begin frame
	if (D3DApp::imguiEnabled)		//imguiEnable = true by default
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)& clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(InputLayouts::InstancedBasic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	UINT stride[2] = { sizeof(Vertex::Basic32), sizeof(InstancedData) };
	UINT offset[2] = { 0,0 };

	ID3D11Buffer* vbs[2] = { mTargetVB, mInstancedBuffer };

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	// Set per frame constants.
	Effects::InstancedBasicFX->SetDirLights(mDirLights);
	Effects::InstancedBasicFX->SetEyePosW(mCam.GetPosition());

	ID3DX11EffectTechnique* activeTech = Effects::InstancedBasicFX->Light3Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (int32_t p = 0; p < techDesc.Passes; ++p)		//passes is int32_t, just to be consistenet for <
	{
		md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		md3dImmediateContext->IASetIndexBuffer(mTargetIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = DirectX::XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

		Effects::InstancedBasicFX->SetWorld(world);
		Effects::InstancedBasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::InstancedBasicFX->SetViewProj(viewProj);
		Effects::InstancedBasicFX->SetMaterial(mSkullMat);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexedInstanced(mIndexCount, mVisibleObjectCount, 0, 0, 0);
	}
	ThrowIfFailed(mSwapChain->Present(0, 0));

}

void OptimMain::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mhMainWnd);
}

void OptimMain::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void OptimMain::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}
	if ((btnState & MK_MBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.UpDown(50 * dy);
		mCam.RightLeft(-50 * dx);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void OptimMain::MouseMidDown(WPARAM btnState, int x, int y)
{
	//
}

void OptimMain::MouseMidUp(WPARAM btnState, int x, int y)
{
	//
}

void OptimMain::BuildGeometryBuffers()
{
	std::ifstream fin("Models/car.txt");	//std::ifstream fin("Models/car.txt");
	if (!fin)
	{
		MessageBox(0, L"Models/car.txt not found.", 0, 0);
		return;
	}
	UINT vcount = 0;
	UINT tcount = 0;

	std::string skip;

	fin >> skip >> vcount;
	fin >> skip >> tcount;
	fin >> skip >> skip >> skip >> skip;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = DirectX::XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = DirectX::XMLoadFloat3(&vMaxf3);
	std::vector<Vertex::Basic32> vertices(vcount);

	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = DirectX::XMLoadFloat3(&vertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	DirectX::XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin + vMax));
	DirectX::XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax - vMin));

	fin >> skip;
	fin >> skip;
	fin >> skip;

	mIndexCount = 3 * tcount;
	std::vector<UINT> indices(mIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, &mTargetVB));

	// 
	// Pack the indices of all the meshes into one index buffer.
	//
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, &mTargetIB));
}

void OptimMain::StructureGeometryBuffers()
{
	//for debugging
	int ImportDLLLfread_Start = clock();
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::ofstream fout("Models/Etabs_copy.txt");

	UINT vcount = 0;
	UINT lcount = 0;

	UINT Story_Count = 0;
	UINT Pt_Count = 0;
	UINT Pt_Total = 0;
	UINT Line_Count = 0;
	UINT LineAss_Count = 0;
	UINT MP_Count = 0;

	std::deque<Story> story;
	std::deque<MaterialProp> materialprop;
	std::deque<PointCoord> pointcoord;
	std::vector <std::vector<Point3D>> point3d;

	std::deque<LineConnect> lineconnect;
	std::deque<LineAssign> lineassign;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
	XMVECTOR vMin = DirectX::XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = DirectX::XMLoadFloat3(&vMaxf3);

	OPENFILENAME ofn;
	wchar_t FileNameTXT[250];
	//Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);                                              //size of structure
	ofn.hwndOwner = 0;                                                       //parent window
	ofn.lpstrFile = NULL;                                                       //path of open file is ofn.lpstrFile, short in szFile
	ofn.lpstrFileTitle = FileNameTXT;                                            //Name of File
	ofn.nMaxFile = sizeof(ofn);
	ofn.lpstrFilter = L"ALL\0*.*\0Text\0*.TXT\0*.DOC\0*.BAK";
	ofn.nFilterIndex = 1;
	ofn.nMaxFileTitle = sizeof(ofn);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn))
	{
		MessageBox(0, L"Get Open File Name Failed", L"FBI WARNING", MB_OK);
	}
	else
	{	
	
		std::string TextHandler;
		std::ifstream SM_read(FileNameTXT);

		if (!SM_read.is_open()) {
			MessageBox(0, L"Import Models.txt Failed.", 0, 0);
		}
		else
		{
			while (std::getline(SM_read, TextHandler))
			{
				std::istringstream iss;
				std::string skip;

				auto Story_pos = TextHandler.find(" STORY ");
				if (Story_pos != std::string::npos)
				{
					iss.str(TextHandler);
					Story st;
					if (iss >> skip >> st.StoryName >> skip >> st.StoryHeight)
						story.emplace_back(std::move(st));	//move: move into and delete the original 
				}

				auto Pt_pos = TextHandler.find("  POINT ");
				if (Pt_pos != std::string::npos)
				{
					iss.str(TextHandler);
					PointCoord pt;
					if (iss >> skip >> pt.PtName >> pt.PtX >> pt.PtY)
						pointcoord.emplace_back(std::move(pt));
				}

				auto Line_pos = TextHandler.find("  LINE  ");
				if (Line_pos != std::string::npos)
				{
					iss.str(TextHandler);
					LineConnect lc;
					if (iss >> skip >> lc.LineName
						>> lc.LineType >> lc.LineStart
						>> lc.LineEnd >> lc.SpanFloor)
						lineconnect.emplace_back(std::move(lc));
				}

				auto LineAss_pos = TextHandler.find(" LINEASSIGN ");
				if (LineAss_pos != std::string::npos)
				{
					iss.str(TextHandler);
					LineAssign la;
					if (iss >> skip >> la.LineName >> la.Story)
						lineassign.emplace_back(std::move(la));
				}

			}//end of while
		}
		SM_read.close();
	}// end reading

	Story_Count = story.size();
	float BaseLevel = story.back().StoryHeight;
	story.back().TotalHeight = story.back().StoryHeight;	//1st element
	for (UINT i = 1; i < story.size(); i++)					//except for 1st element
	{
		story[story.size() - 1 - i].TotalHeight = story[story.size() - i].TotalHeight + story[story.size() - 1 - i].StoryHeight;
	}

	Pt_Count = pointcoord.size();
	Pt_Total = pointcoord.size()*story.size();
	LineAss_Count = lineassign.size();

	vcount = Pt_Total;
	lcount = LineAss_Count;

	//expand 2D container points to 3D
	point3d.resize(story.size(), vector<Point3D>(pointcoord.size()));
	for (UINT i = 0; i < story.size(); i++)
	{
		for (UINT j = 0; j < pointcoord.size(); j++)
		{
			point3d[i][j].PtName = pointcoord[j].PtName;
			point3d[i][j].PtX = pointcoord[j].PtX;
			point3d[i][j].PtY = pointcoord[j].PtY;
			point3d[i][j].PtZ = story[i].TotalHeight;
		}
	}

	//matching line connectivity to line assignment 
	std::vector <Vertex::Basic32> vertices(vcount);		//1 vertex for each pt
	std::vector <LineDirectx> linedirectx(lcount);

	for (UINT i = 0; i < Story_Count; i++)
	{
		for (UINT j = 0; j < Pt_Count; j++)
		{
			vertices[i * Pt_Count + j].Pt_Num = point3d[i][j].PtName;
			vertices[i * Pt_Count + j].Pos.x = point3d[i][j].PtX;
			vertices[i * Pt_Count + j].Pos.y = point3d[i][j].PtY;
			vertices[i * Pt_Count + j].Pos.z = point3d[i][j].PtZ;
			vertices[i * Pt_Count + j].Normal.x = 0;
			vertices[i * Pt_Count + j].Normal.y = 0;
			vertices[i * Pt_Count + j].Normal.z = 1;

			XMVECTOR P = DirectX::XMLoadFloat3(&vertices[i * Pt_Count + j].Pos);
			vMin = XMVectorMin(vMin, P);
			vMax = XMVectorMax(vMax, P);
		}
	}

	//Initial Camera Setting
	DirectX::XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin + vMax));
	DirectX::XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax - vMin));

	//遍历全部，共计lcount 条 linedirectx
	for (UINT i = 0; i < lcount; ++i)
	{
		//get line floor inedex in vector Story[]
		UINT pos_story = 0;
		std::string Search_story = lineassign[i].Story;
		auto _predicate_s = [Search_story](const Story & item)
		{
			return item.StoryName == Search_story;
		};
		auto itr = std::find_if(std::begin(story), std::end(story), _predicate_s);
		if (itr >= std::end(story))
		{
			MessageBox(0, L"Can't match Line's Story", L"Warning", MB_OK); 
		}
		else 
		{
			pos_story = std::distance(story.begin(), itr);
		}

		//get line index in vector LinePtr[]
		UINT pos_lineconnect = 0;
		std::string Search_lincct = lineassign[i].LineName;
		auto _predicate_l = [Search_lincct](const LineConnect & item) {
			return item.LineName == Search_lincct;
		};
		auto itr_l = std::find_if(std::begin(lineconnect), std::end(lineconnect), _predicate_l);
		if (itr_l >= std::end(lineconnect)) {
			MessageBox(0, L"Can't match Line's Story, Close the App", L"Warning", MB_OK);
		}
		else {
			pos_lineconnect = std::distance(lineconnect.begin(), itr_l);
		}
		
		lineassign[i].LineStart = lineconnect[pos_lineconnect].LineStart;
		lineassign[i].LineEnd = lineconnect[pos_lineconnect].LineEnd;
		lineassign[i].SpanFloor = lineconnect[pos_lineconnect].SpanFloor;

		linedirectx[i].LineName = lineassign[i].LineName;
		linedirectx[i].Story = lineassign[i].Story;
		linedirectx[i].LineStart = lineassign[i].LineStart;
		linedirectx[i].LineEnd = lineassign[i].LineEnd;
		linedirectx[i].SpanFloor = lineassign[i].SpanFloor;		

		UINT pos_pt1 = 0;
		//search point p1 in Vertex::Basic32
		std::string Search_p1 = linedirectx[i].LineStart;
		auto _predicate_p1 = [Search_p1](const Vertex::Basic32 & item) {
			return item.Pt_Num == Search_p1;
		};
		auto itr_p1 = std::find_if(std::begin(vertices), std::end(vertices), _predicate_p1);
		if (itr_p1 >= std::end(vertices)) {
			MessageBox(0, L"Can't match Line's Start Point, Close the App", L"Warning", MB_OK);
		}
		else {
			pos_pt1 = std::distance(vertices.begin(), itr_p1);
		}
		linedirectx[i].LineSIndex = pos_story * Pt_Count + pos_pt1;
		
		UINT pos_pt2 = 0;
		std::string Search_p2 = linedirectx[i].LineEnd;
		auto _predicate_p2 = [Search_p2](const Vertex::Basic32 & item) {
			return item.Pt_Num == Search_p2;
		};
		auto itr_p2 = std::find_if(std::begin(vertices), std::end(vertices), _predicate_p2);
		if (itr_p2 >= std::end(vertices)) {
			MessageBox(0, L"Can't match Line's Start Point, Close the App", L"Warning", MB_OK);
		}
		else {
			pos_pt2 = std::distance(vertices.begin(), itr_p2);
		}
		//line的EndPt在vertex中的总索引 = （起始层 - 跨层数）x 每层点总数 + EndPt 当层的索引
		linedirectx[i].LineEIndex = (pos_story - linedirectx[i].SpanFloor) * Pt_Count  + pos_pt2;
	}

	mIndexCount = 2 * lcount;		//points to reder are twice of lines
	std::vector<UINT> indices(mIndexCount);
	for (UINT i = 0; i < lcount; ++i)
	{
		indices[i * 2 + 0] = linedirectx[i].LineSIndex;
		indices[i * 2 + 1] = linedirectx[i].LineEIndex;
	}
		
	
	fout << '\n' << "Materials" << endl;
	for (UINT i = 0; i < materialprop.size(); ++i)
	{
		fout << materialprop[i].Name << " " << materialprop[i].Mass << " "
			<< materialprop[i].Weight << " " << materialprop[i].ModulusE << " "
			<< materialprop[i].PossionR << std::endl;
	}

	fout << '\n' << "Story: NOS = " << story.size() << endl;
	for (UINT i = 0; i < story.size(); ++i)
	{
		fout << story[i].StoryName << " Height " << story[i].StoryHeight
			<< " mPD = " << story[i].TotalHeight << endl;
	}

	fout << '\n' << "Points Coordinates: " << pointcoord.size() << endl;
	for (UINT i = 0; i < story.size(); i++)
	{
		for (UINT j = 0; j < pointcoord.size(); j++)
		{
			fout << " PtName: " << point3d[i][j].PtName
				<< " @ Floor: " << story[i].StoryName
				<< " X: " << point3d[i][j].PtX
				<< " Y: " << point3d[i][j].PtY
				<< " Z: " << point3d[i][j].PtZ << endl;
		}
	}

	fout << '\n' << "LINE Connectivity: NOS = " << Line_Count << endl;
	for (UINT i = 0; i < lcount; ++i)
	{
		fout << linedirectx[i].LineName << " @ Floor " << linedirectx[i].Story << endl;
		fout << linedirectx[i].LineStart << " LineSIndex = " << linedirectx[i].LineSIndex << endl;
		fout << linedirectx[i].LineEnd << " LineEIndex = " << linedirectx[i].LineEIndex << endl;
	}

	fout << '\n' << "LINE Assignment: NOS = " << lineassign.size() << endl;
	for (UINT i = 0; i < lineassign.size(); ++i)
	{
		fout << lineassign[i].LineName << " " << lineassign[i].LineStart << " " << lineassign[i].LineEnd << " " << lineassign[i].Story << endl;
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	__int64 time_sec = (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()); //can use chrono::microseconds>
	fout << "Time elapsed = " << time_sec / 1000 << " s " << std::endl;
	fout.close();

	//performance testing
	float ImportDLLLfread_Elapsed = (float(clock() - ImportDLLLfread_Start) / CLOCKS_PER_SEC);
	std::wstringstream wtss(L"");
	wtss << "Loading Read & Write Done in " << ImportDLLLfread_Elapsed << " s ";
	MessageBox(NULL, wtss.str().c_str(), L"MsgBox", MB_OK);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, &mTargetVB));

	// 
	// Pack the indices of all the meshes into one index buffer.
	//
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, &mTargetIB));
}

void OptimMain::FdtGeometryBuffers()
{
	UINT vcount = 0;
	UINT lcount = 0;

	struct PointSFCoord { std::string PtName; float PtX = 0.0f; float PtY = 0.0f; float PtZ = 0.0f; };
	struct LineObject { std::string LineName, LineStart, LineEnd; };

	std::deque<PointSFCoord> pointsfcoord;	//all pts stored here
	std::deque<LineObject> lineobject;		//all lines stored here

	std::ofstream fout("Models/SAFE_copy.txt");

	OPENFILENAME ofn;
	wchar_t FileNameTXT[250];
	//Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);                                              //size of structure
	ofn.hwndOwner = 0;                                                       //parent window
	ofn.lpstrFile = NULL;                                                       //path of open file is ofn.lpstrFile, short in szFile
	ofn.lpstrFileTitle = FileNameTXT;                                            //Name of File
	ofn.nMaxFile = sizeof(ofn);
	ofn.lpstrFilter = L"ALL\0*.*\0Text\0*.TXT\0*.DOC\0*.BAK";
	ofn.nFilterIndex = 1;
	ofn.nMaxFileTitle = sizeof(ofn);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn))
	{
		MessageBox(0, L"Get Open File Name Failed", L"FBI WARNING", MB_OK);
	}
	else
	{

		std::string TextHandler;
		std::ifstream SM_read(FileNameTXT);

		if (!SM_read.is_open()) {
			MessageBox(0, L"Import Models.txt Failed.", 0, 0);
		}
		else
		{
			bool FoundPt = false;
			bool FoundLine = false;

			//control the reading by found or not found status
			while (std::getline(SM_read, TextHandler))
			{
				std::istringstream iss;
				std::string skip;

				std::string TempStrX, TempStrY, TempStrZ;
				auto Pt_pos = TextHandler.find("POINT COORDINATES");
				if (Pt_pos != std::string::npos)
				{
					FoundPt = true;			//reading pts start
					continue;
				}

				if (std::all_of(TextHandler.begin(), TextHandler.end(), isspace))	////reading pts end
				{
					FoundPt = false;
					FoundLine = false;
					continue;
				}
				if (FoundPt == true && !TextHandler.empty())
				{
					iss.str(TextHandler);
					PointSFCoord ptsf;
					if (iss >> ptsf.PtName >> TempStrX >> TempStrY >> TempStrZ)
					{
						ptsf.PtName = ptsf.PtName.substr(6, string::npos);		//refer to the $sf file for position
						TempStrX = TempStrX.substr(8, string::npos);
						TempStrY = TempStrY.substr(8, string::npos);
						TempStrZ = TempStrZ.substr(8, string::npos);
						ptsf.PtX = std::stof(TempStrX.c_str());
						ptsf.PtY = std::stof(TempStrY.c_str());
						ptsf.PtZ = std::stof(TempStrZ.c_str());
						pointsfcoord.emplace_back(std::move(ptsf));
					}
				}

				auto Line_pos = TextHandler.find("OBJECT GEOMETRY - LINES");
				if (Line_pos != std::string::npos)
				{
					FoundLine = true;			//reading lines start
					continue;
				}

				if (FoundLine == true && !TextHandler.empty())
				{
					iss.str(TextHandler);
					LineObject lo;
					if (iss >> lo.LineName >> lo.LineStart >> lo.LineEnd)
					{
						lo.LineName = lo.LineName.substr(5, string::npos);
						lo.LineStart = lo.LineStart.substr(7, string::npos);
						lo.LineEnd = lo.LineEnd.substr(7, string::npos);
						lineobject.emplace_back(std::move(lo));
					}
				}

			}//end of while
		}
		SM_read.close();
	}// end reading

	fout << "SAFE output" << endl;
	for (UINT i = 0; i < pointsfcoord.size(); ++i)
	{
		fout << pointsfcoord[i].PtName << "\t " << pointsfcoord[i].PtX << "\t "
			<< pointsfcoord[i].PtY << "\t " << pointsfcoord[i].PtZ << endl;



	}
	for (UINT i = 0; i < lineobject.size(); ++i)
	{
		fout << lineobject[i].LineName << "\t " << lineobject[i].LineStart << "\t "
		<< lineobject[i].LineEnd << endl;
	}
	fout.close();
	MessageBox(0, L"SAFE Done", 0, MB_OK);
}
void OptimMain::ImportDLLLTest()
{

}
void OptimMain::ImportDLLL()
{
	int ImportDLLLTest_Start = clock();

	std::deque<Beam> beam;
	Beam *BeamPtr = NULL;
	UINT BeamForCount = 0;
	std::string TextHandler;

	UINT getlineCount = 0;
	bool NewBeamLoad = false;	//detect line Floor BeamName Ld4 or line forces

	std::ifstream fin("Models/2. Block 2 Beam(DLLL).txt");
	std::ofstream fout("Models/Force_copy.txt");

	if (!fin) {
		fout << L"IMPORT FORECE FAILED!" << std::endl;
		MessageBox(0, L"IMPORT FORECE FAILED.", L"FBI WARNING", MB_OK);
	}
	else
	{
		while (getline(fin, TextHandler))
		{	
			std::istringstream iss;
			std::string skip;
			getlineCount++;

			//if (TextHandler.find("STORY ") != std::string::npos)
			// STORY       BEAM        LOAD  starts at line 18
			if (getlineCount >= 18)
			{
				UINT wordCount = 0; // Holds number of words					
				char LineChar[256];
				strcpy(LineChar, TextHandler.c_str());

				for (UINT i = 0; i < std::strlen(LineChar); i++)
				{
					if ((LineChar[i] >= 65 && LineChar[i] <= 90)
						|| (LineChar[i] >= 97 && LineChar[i] <= 122)
						|| (LineChar[i] >= 46 && LineChar[i] <= 57))		//|| (LineChar[i] >= 46 && LineChar[i] <= 57)
					{
						wordCount++;
						while (
							((LineChar[i] >= 65 && LineChar[i] <= 90)
								|| (LineChar[i] >= 97 && LineChar[i] <= 122)
								|| (LineChar[i] >= 46 && LineChar[i] <= 57))	//|| (LineChar[i] >= 46 && LineChar[i] <= 57)
							&& LineChar[i] != '\0')
						{
							i++;
						}
					}
				}

				if (wordCount == 0)
				{
					//MessageBox(0, L"wordCount==0", L"FBI WARNING", MB_OK);
					continue;
				}
				else if (wordCount == 3)
				{
					NewBeamLoad = true;
					//MessageBox(0, L"wordCount==3", L"FBI WARNING", MB_OK);
					iss.str(TextHandler);
					Beam bm;
					if (iss >> bm.Floor >> bm.Name >> bm.LdCase)
					{
						bm.Loc = 0;
						bm.P = 0;
						bm.V2 = 0;
						bm.V3 = 0;
						bm.T = 0;
						bm.M2 = 0;
						bm.M3 = 0;
						beam.emplace_back(std::move(bm));
					}
					
				}

				else //if (wordCount == 7)
				{
					//MessageBox(0, L"wordCount==7", L"FBI WARNING", MB_OK);
					iss.str(TextHandler);
					Beam bm;
					std::string a, b, c, d, e, f, g;
					if (iss >> a >> b >> c >> d >> e >> f >> g)
					{
						bm.Loc = std::stof(a.c_str());
						bm.P = std::stof(b.c_str());
						bm.V2 = std::stof(c.c_str());
						bm.V3 = std::stof(d.c_str());
						bm.T = std::stof(e.c_str());
						bm.M2 = std::stof(f.c_str());
						bm.M3 = std::stof(g.c_str());
						bm.Floor = beam.back().Floor;
						bm.Name = beam.back().Name;
						bm.LdCase = beam.back().LdCase;
						beam.emplace_back(std::move(bm));
					}
				}
			}
		}
	}
	fin.close();
	float ImportDLLLTest_Read_Elapsed =( float(clock() - ImportDLLLTest_Start) / CLOCKS_PER_SEC);

	//output for debugging
	fout << "Time used for Reading: " << ImportDLLLTest_Read_Elapsed << "\n";

	for (UINT i = 0; i < beam.size(); ++i)
	{
		fout << std::fixed << std::setprecision(4);
		fout << "Floor= " << beam[i].Floor << "\tName= " << beam[i].Name
			<< "\tLdCase= " << beam[i].LdCase << "\tLoc= " << beam[i].Loc
			<< "\tP= " << beam[i].P << "\tV2= " << beam[i].V2 << "\tV3= " << beam[i].V3
			<< "\tT= " << beam[i].T << "\tM2= " << beam[i].M2 << "\tM3= " << beam[i].M3 << "\n";
			
	}
	float ImportDLLLTest_Write_Elapsed =( float(clock() - ImportDLLLTest_Start- ImportDLLLTest_Read_Elapsed) / CLOCKS_PER_SEC);
	fout << "Time Elapsed for Writing: "<<ImportDLLLTest_Write_Elapsed<<"\n";
	std::wstringstream wtss(L"");
	wtss << "Loading Read & Write Done in " << ImportDLLLTest_Read_Elapsed << " s And " << ImportDLLLTest_Write_Elapsed << " s ";
	MessageBox(NULL, wtss.str().c_str(), L"MsgBox", MB_OK);
}

void OptimMain::ImportDLLL_MMAP()
{
	//To_be_done
}

void OptimMain::BuildInstancedBuffer()	//no need instanced for the moment
{
	const int n = 1;
	UINT m = max(n - 1, 1);
	mInstancedData.resize(n*n*n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f*width;
	float y = -0.5f*height;
	float z = -0.5f*depth;

	float dx = width / m;
	float dy = height / m;
	float dz = depth / m;
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				// Position instanced along a 3D grid.
				mInstancedData[k*n*n + i * n + j].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				// yellow color.
				mInstancedData[k*n*n + i * n + j].Color.x = 0.0f;	//red
				mInstancedData[k*n*n + i * n + j].Color.y = 100.0f;	//yellow
				mInstancedData[k*n*n + i * n + j].Color.z = 0.0f;	//blue
				mInstancedData[k*n*n + i * n + j].Color.w = 0.0f;	//black
			}
		}
	}
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;			//动态缓冲
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}

bool MouseWheelUp()
{
	return true;
}

