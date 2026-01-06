////////////////////////////////////////////////////////////////////////////////
// Filename: Projector.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _PROJECTOR_H_
#define _PROJECTOR_H_

#include <DirectXMath.h>
#include "../../Graphics/Camera3D.h"

#include <array>
#include <limits>
#include <algorithm>

namespace DirectX11
{
	// ray = rayO + rayD * t.
	/*bool RayPlaneIntersection(DirectX::XMVECTOR rayO, DirectX::XMVECTOR rayD, DirectX::XMVECTOR planePoint, DirectX::XMVECTOR normal, DirectX::XMVECTOR& outPoint);

	struct Span2D { float xmin = 1, xmax = 0, ymin = 1, ymax = 0; };

	inline void Accumulate(Span2D& s, float u, float v)
	{
		s.xmin = std::min(s.xmin, u);  s.xmax = std::max(s.xmax, u);
		s.ymin = std::min(s.ymin, v);  s.ymax = std::max(s.ymax, v);
	}

	inline bool ClipLineWithYPlane( // true => er is snijpunt binnen segment
		DirectX::XMVECTOR  a, DirectX::XMVECTOR  b, // segmentpunten (wereld)
		float      yPlane,          // vlakhoogte  (+1 of -1)
		DirectX::XMVECTOR& outHit)
	{
		using namespace DirectX;

		float ya = DirectX::XMVectorGetY(a), yb = DirectX::XMVectorGetY(b);
		float dy = yb - ya;
		if (fabsf(dy) < 1e-6f) return false;           // evenwijdig
		float t = (yPlane - ya) / dy;                  // param binnen segment?
		if (t < 0.0f || t > 1.0f) return false;
		outHit = a + (b - a) * t;
		return true;
	}

	// Only works on XZ-planes.
	bool CalculateRangeConversionMatrix(const DirectX::XMMATRIX& vpMatrix, DirectX::XMMATRIX& rangeMatrix, const DirectX::XMMATRIX& projectorMatrix, float heightScale = 0.02f, float heightSBase = 0.0f);

	DirectX::XMVECTOR CalculateProjectorLookAtPoint(const Camera3D& camera, DirectX::XMVECTOR pBase, DirectX::XMVECTOR nBase, float farZ);*/

	// pBase and nBase form S_base.
	//bool CalculateProjectorMatrix(DirectX::XMMATRIX& projectorMatrix, const Camera3D& camera, DirectX::XMVECTOR pBase, DirectX::XMVECTOR nBase, float nearZ, float farZ);

	bool CalculateProjectorMatrix(DirectX::XMMATRIX& projectorMatrix, const Camera3D& camera, const DirectX::XMVECTOR& oceanPosition, float maxWaveHeight);
}

#endif // !_PROJECTOR_H_