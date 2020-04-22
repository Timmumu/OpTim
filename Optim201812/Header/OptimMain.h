#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wrl.h>		//for smart pointer ComPtr

#include <vector>
#include <deque> // for std::deque
#include <iostream>	
#include <algorithm> //for find_if
#include <stdio.h> //for fread
#include <iomanip>
#include <fstream>
#include <sstream>
#include <Commdlg.h> // for OPENFILENAME Structure

#include <conio.h>

//#include <D3D11.h>					// directx 3d
#include <d3d11_1.h>
#include <d3dcompiler.h>  
#include <directxcolors.h>
#include <DirectXMath.h>

#include "d3dUtil.h"

#include "Vertex.h" 
#include <map>
#include <chrono>						//for time elapsed

//imgui
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
