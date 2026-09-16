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
	struct ProjectorRange
	{
		float minX, maxX;
		float minY, maxY;

		ProjectorRange(float minX = 0.f, float minY = 0.f, float maxX = 0.f, float maxY = 0.f)
			: minX(minX), maxX(maxX), minY(minY), maxY(maxY)
		{
		}
	};

	bool CalculateProjectorMatrix(DirectX::XMMATRIX& projectorMatrix, const Camera3D& camera, const DirectX::XMVECTOR& oceanPosition, float maxWaveHeight);
}

#endif // !_PROJECTOR_H_