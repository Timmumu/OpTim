
#pragma once

#ifndef _XNA_COLLISION_H_
#define _XNA_COLLISION_H_


	#if !defined(XMASSERT)
	#if defined(_PREFAST_)
	#define XMASSERT(Expression) __analysis_assume((Expression))
	#elif defined(XMDEBUG) // !_PREFAST_
	#define XMASSERT(Expression) ((VOID)((Expression) || (XMAssert(#Expression, __FILE__, __LINE__), 0)))
	#else // !XMDEBUG
	#define XMASSERT(Expression) ((VOID)0)
	#endif // !XMDEBUG
	#endif // !XMASSERT

	#if !defined(XM_NO_ALIGNMENT)
	#define _DECLSPEC_ALIGN_16_   __declspec(align(16))
	#else
	#define _DECLSPEC_ALIGN_16_
	#endif


#include "OptimMain.h"


namespace XNA
{

	//-----------------------------------------------------------------------------
	// Bounding volumes structures.
	//
	// The bounding volume structures are setup for near minimum size because there
	// are likely to be many of them, and memory bandwidth and space will be at a
	// premium relative to CPU cycles on Xbox 360.
	//-----------------------------------------------------------------------------

	#pragma warning(push)
	#pragma warning(disable: 4324)


_DECLSPEC_ALIGN_16_ struct AxisAlignedBox
{
	DirectX::XMFLOAT3 Center;            // Center of the box.
	DirectX::XMFLOAT3 Extents;           // Distance from the center to each side.
};

_DECLSPEC_ALIGN_16_ struct OrientedBox
{
	XMFLOAT3 Center;            // Center of the box.
	XMFLOAT3 Extents;           // Distance from the center to each side.
	XMFLOAT4 Orientation;       // Unit quaternion representing rotation (box -> world).
};

_DECLSPEC_ALIGN_16_ struct Frustum
{
	DirectX::XMFLOAT3 Origin;            // Origin of the frustum (and projection).
	DirectX::XMFLOAT4 Orientation;       // Unit quaternion representing rotation.

	FLOAT RightSlope;           // Positive X slope (X/Z).
	FLOAT LeftSlope;            // Negative X slope.
	FLOAT TopSlope;             // Positive Y slope (Y/Z).
	FLOAT BottomSlope;          // Negative Y slope.
	FLOAT Near, Far;            // Z of the near plane and far plane.
};


#pragma warning(pop)

//-----------------------------------------------------------------------------
// Bounding volume construction.
//-----------------------------------------------------------------------------
VOID ComputeFrustumFromProjection(Frustum* pOut, DirectX::XMMATRIX* pProjection);



//-----------------------------------------------------------------------------
// Bounding volume transforms.
//-----------------------------------------------------------------------------
VOID TransformFrustum(Frustum* pOut, const Frustum* pIn, FLOAT Scale, 
					DirectX::FXMVECTOR Rotation, DirectX::FXMVECTOR Translation);

//-----------------------------------------------------------------------------
// Frustum intersection testing routines.
// Return values: 0 = no intersection, 
//                1 = intersection, 
//                2 = A is completely inside B
//-----------------------------------------------------------------------------
INT IntersectAxisAlignedBoxFrustum(const AxisAlignedBox* pVolumeA, 
									const Frustum* pVolumeB);
INT IntersectOrientedBoxFrustum(const OrientedBox* pVolumeA, const Frustum* pVolumeB);

};	//namespace

#endif