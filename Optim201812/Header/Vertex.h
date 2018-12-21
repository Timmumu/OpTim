#pragma once

#ifndef VERTEX_H
#define VERTEX_H

#include "OptimMain.h"

namespace Vertex
{
	// Basic 32-byte vertex structure.
	struct Basic32
	{	
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
		std::string Pt_Num;
	};

}


class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC InstancedBasic32[9];
};

class InputLayouts
{
public:
	
	static void InitAll(ID3D11Device* device);
	
	static void DestroyAll();

	static ID3D11InputLayout* InstancedBasic32;

};

#endif // VERTEX_H