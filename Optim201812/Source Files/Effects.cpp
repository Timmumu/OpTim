//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	//simple checker
	ThrowIfFailed(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, device, &mFX));
	
}	

Effect::~Effect()
{
	ReleaseCom(mFX);
}
#pragma endregion

#pragma region InstancedBasicEffect
InstancedBasicEffect::InstancedBasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

InstancedBasicEffect::~InstancedBasicEffect()
{
	ReleaseCom(Light1Tech);
	ReleaseCom(Light2Tech);
	ReleaseCom(Light3Tech);
	ReleaseCom(Light0TexTech)
	ReleaseCom(Light1TexTech)
	ReleaseCom(Light2TexTech)
	ReleaseCom(Light3TexTech)
	ReleaseCom(Light0TexAlphaClipTech)
	ReleaseCom(Light1TexAlphaClipTech)
	ReleaseCom(Light2TexAlphaClipTech)
	ReleaseCom(Light3TexAlphaClipTech)
	ReleaseCom(Light1FogTech)
	ReleaseCom(Light2FogTech)
	ReleaseCom(Light3FogTech)
	ReleaseCom(Light0TexFogTech)
	ReleaseCom(Light1TexFogTech)
	ReleaseCom(Light2TexFogTech)
	ReleaseCom(Light3TexFogTech)
	ReleaseCom(Light0TexAlphaClipFogTech)
	ReleaseCom(Light1TexAlphaClipFogTech)
	ReleaseCom(Light2TexAlphaClipFogTech)
	ReleaseCom(Light3TexAlphaClipFogTech)
	ReleaseCom(ViewProj)
	ReleaseCom(World)
	ReleaseCom(WorldInvTranspose)
	ReleaseCom(TexTransform)
	ReleaseCom(EyePosW)
	ReleaseCom(FogColor)
	ReleaseCom(FogStart)
	ReleaseCom(FogRange)
	ReleaseCom(DirLights)
	ReleaseCom(Mat)
	ReleaseCom(DiffuseMap)
}
#pragma endregion

#pragma region Effects

InstancedBasicEffect* Effects::InstancedBasicFX = 0;

void Effects::InitAll(ID3D11Device* device)
{

	InstancedBasicFX = new InstancedBasicEffect(device, L"FX/InstancedBasic.fxo");

}

void Effects::DestroyAll()
{
	SafeDelete(InstancedBasicFX);

}
#pragma endregion
 