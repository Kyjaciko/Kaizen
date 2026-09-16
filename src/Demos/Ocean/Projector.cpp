#include "Projector.h"

////////////////////////////////////////////////////////////////////////////////
// Filename: Projector.h
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	std::array<DirectX::XMFLOAT3, 8> GetFrustumCornersWorldSpace(const DirectX::XMMATRIX& viewProjectionMatrix)
	{
		// Calculate camera’s inverted viewproj matrix.
		DirectX::XMVECTOR determinant;
		DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&determinant, viewProjectionMatrix);

		// Define the 8 cornerpoints in NDC.
		float zNear = 0.0f;
		float zFar = 1.0f;

		DirectX::XMVECTOR cornersNDC[8];
		cornersNDC[0] = DirectX::XMVectorSet(-1.0f, -1.0f, zNear, 1.0f); // bl
		cornersNDC[1] = DirectX::XMVectorSet(-1.0f, 1.0f, zNear, 1.0f);	 // tl
		cornersNDC[2] = DirectX::XMVectorSet(1.0f, 1.0f, zNear, 1.0f);   // tr
		cornersNDC[3] = DirectX::XMVectorSet(1.0f, -1.0f, zNear, 1.0f);	 // br

		cornersNDC[4] = DirectX::XMVectorSet(-1.0f, -1.0f, zFar, 1.0f);  // bl
		cornersNDC[5] = DirectX::XMVectorSet(-1.0f, 1.0f, zFar, 1.0f);	 // tl
		cornersNDC[6] = DirectX::XMVectorSet(1.0f, 1.0f, zFar, 1.0f);	 // tr
		cornersNDC[7] = DirectX::XMVectorSet(1.0f, -1.0f, zFar, 1.0f);   // br

		// Transform NDC corners to world space.
		std::array<DirectX::XMFLOAT3, 8> cornersWorld{};
		for (std::ptrdiff_t i = 0; i < cornersWorld.size(); ++i)
		{
			DirectX::XMVECTOR pointWorld = XMVector3TransformCoord(cornersNDC[i], invViewProj);
			XMStoreFloat3(&cornersWorld[i], pointWorld);
		}

		return cornersWorld;
	}

	bool IntersectSegmentPlane(DirectX::FXMVECTOR p1, DirectX::FXMVECTOR p2, DirectX::FXMVECTOR planeEquation, DirectX::XMVECTOR& intersectionPoint)
	{
		// Calculate signed distance between each point.
		DirectX::XMVECTOR vec1 = DirectX::XMPlaneDotCoord(planeEquation, p1);
		DirectX::XMVECTOR vec2 = DirectX::XMPlaneDotCoord(planeEquation, p2);

		float d1 = DirectX::XMVectorGetX(vec1);
		float d2 = DirectX::XMVectorGetX(vec2);

		// Check for intersection.
		if (d1 * d2 > 0.0f) 
			// d1 * d2 > 0  -> no intersection.
			// d1 * d2 <= 0 -> intersection.
		{
			return false;
		}

		// Calculate interpolation factor t.
		float t = d1 / (d1 - d2);

		// Calculate intersection point.
		DirectX::XMVECTOR lineDirection = DirectX::XMVectorSubtract(p2, p1);
		intersectionPoint = DirectX::XMVectorAdd(p1, DirectX::XMVectorScale(lineDirection, t));

		return true;
	}

	void FindFrustumPlaneIntersections(const std::array<DirectX::XMFLOAT3, 8>& frustumCorners, DirectX::FXMVECTOR Supper, DirectX::FXMVECTOR Slower, std::vector<DirectX::XMFLOAT3>& outBuffer)
	{
		// Indices who form the 12 edges (defined in step 1 cornersNDC).
		constexpr int edges[12][2] = 
		{
			{0,1}, {1,2}, {2,3}, {3,0}, // Near plane edges.
			{4,5}, {5,6}, {6,7}, {7,4}, // Far plane edges.
			{0,4}, {1,5}, {2,6}, {3,7}  // Ribs from near to far.
		};

		// Convert input to XMVECTOR for calculations.
		DirectX::XMVECTOR corners[8] = {};
		for (int i = 0; i < 8; ++i)
			corners[i] = DirectX::XMLoadFloat3(&frustumCorners[i]);

		// Detect intersection for each edge.
		DirectX::XMFLOAT3 pt;
		DirectX::XMVECTOR intersectPt;
		for (int i = 0; i < 12; ++i)
		{
			int idxA = edges[i][0];
			int idxB = edges[i][1];

			// Intersection with Supper.
			if (IntersectSegmentPlane(corners[idxA], corners[idxB], Supper, intersectPt))
			{
				DirectX::XMStoreFloat3(&pt, intersectPt);
				outBuffer.push_back(pt);
			}

			// Intersection with Slower.
			if (IntersectSegmentPlane(corners[idxA], corners[idxB], Slower, intersectPt))
			{
				DirectX::XMStoreFloat3(&pt, intersectPt);
				outBuffer.push_back(pt);
			}
		}
	}

	void AddFrustumCornerPointsInsidePlanes(const std::array<DirectX::XMFLOAT3, 8>& frustumCorners, DirectX::FXMVECTOR Supper, DirectX::FXMVECTOR Slower, std::vector<DirectX::XMFLOAT3>& outBuffer)
	{
		DirectX::XMVECTOR point;
		for (const auto& corner : frustumCorners)
		{
			point = DirectX::XMLoadFloat3(&corner);

			// Calculate distance to Supper and Slower.
			DirectX::XMVECTOR distUpperVec = DirectX::XMPlaneDotCoord(Supper, point);
			float dUpper = DirectX::XMVectorGetX(distUpperVec); // X is used as a convention; Y, Z and W have the same value.

			DirectX::XMVECTOR distLowerVec = DirectX::XMPlaneDotCoord(Slower, point);
			float dLower = DirectX::XMVectorGetX(distLowerVec);

			// Check if the corner points lie within them.
			if (dUpper <= 0.0f && dLower >= 0.0f)
				outBuffer.push_back(corner);
		}
	}

	void ProjectPointsOnBasePlane(std::vector<DirectX::XMFLOAT3>& intersectionPoints, DirectX::FXMVECTOR Sbase)
	{
		// Extract normal from plane equation vector.
		DirectX::XMVECTOR planeNormal = Sbase;
		planeNormal = DirectX::XMVectorSetW(planeNormal, 0.0f);

		for (int i = 0; i < intersectionPoints.size(); ++i)
		{
			DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&intersectionPoints[i]);

			// Calculate distance to the plane.
			DirectX::XMVECTOR distanceVec = DirectX::XMPlaneDotCoord(Sbase, p);
			DirectX::XMVECTOR displacement = DirectX::XMVectorMultiply(distanceVec, planeNormal);

			// Project the point on Sbase.
			DirectX::XMVECTOR pProjected = DirectX::XMVectorSubtract(p, displacement);
			DirectX::XMStoreFloat3(&intersectionPoints[i], pProjected);
		}
	}

	ProjectorRange CalculateVisibleSpan(std::vector<DirectX::XMFLOAT3>& pointsOnSbase, const DirectX::XMMATRIX& projectorViewProj)
	{
		// Transform each point.
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		for (const auto& pt : pointsOnSbase)
		{
			DirectX::XMVECTOR pWorld = DirectX::XMLoadFloat3(&pt);

			// Transform from World Space to Projector Space.
			DirectX::XMVECTOR pProjected = DirectX::XMVector3TransformCoord(pWorld, projectorViewProj);
			float x = DirectX::XMVectorGetX(pProjected);
			float y = DirectX::XMVectorGetY(pProjected);

			// Update the bounds.
			if (x < minX) minX = x;
			if (x > maxX) maxX = x;

			if (y < minY) minY = y;
			if (y > maxY) maxY = y;
		}

		return ProjectorRange(minX, minY, maxX, maxY);
	}

	void BuildRangeMatrix(const ProjectorRange& range, DirectX::XMMATRIX& rangeMatrix)
	{
		// Calculate scale factors.
		float scaleX = range.maxX - range.minX;
		float scaleY = range.maxY - range.minY;

		// Build range matrix.
		rangeMatrix.r[0] = DirectX::XMVectorSet(scaleX, 0.0f, 0.0f, 0.0f);
		rangeMatrix.r[1] = DirectX::XMVectorSet(0.0f, scaleY, 0.0f, 0.0f);
		rangeMatrix.r[3] = DirectX::XMVectorSet(range.minX, range.minY, 0.0f, 1.0f);
	}

	DirectX::XMVECTOR CreateHorizontalPlane(float yHeight)
	{
		// Plane equation: 0x + 1y + 0z - height = 0.
		return DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, -yHeight);
	}

	bool CreateRangeConversionMatrix(const DirectX::XMMATRIX& cameraViewProj, const DirectX::XMMATRIX& projectorViewProj, DirectX::XMMATRIX& rangeMatrix, DirectX::XMVECTOR sBase, DirectX::XMVECTOR sUpper, DirectX::XMVECTOR sLower)
	{
		auto frustumCorners = GetFrustumCornersWorldSpace(cameraViewProj);

		std::vector<DirectX::XMFLOAT3> intersectionPoints{};
		FindFrustumPlaneIntersections(frustumCorners, sUpper, sLower, intersectionPoints);
		AddFrustumCornerPointsInsidePlanes(frustumCorners, sUpper, sLower, intersectionPoints);

		if (intersectionPoints.empty())
			return false;

		ProjectPointsOnBasePlane(intersectionPoints, sBase);
		ProjectorRange range = CalculateVisibleSpan(intersectionPoints, projectorViewProj);
		BuildRangeMatrix(range, rangeMatrix);

		return true;
	}

	DirectX::XMVECTOR CalculateProjectorPosition(DirectX::FXMVECTOR camPos, float SupperY)
	{
		// Create a bound so we never go lower than Supper.
		float camY = DirectX::XMVectorGetY(camPos);
		float projectorY = std::max(camY, SupperY);

		// Create new position.
		DirectX::XMVECTOR projectorPos = DirectX::XMVectorSetY(camPos, projectorY);
		return projectorPos;
	}
	
	DirectX::XMVECTOR CalculateAimPoint(DirectX::FXMVECTOR camPos, DirectX::FXMVECTOR camDir, DirectX::FXMVECTOR Sbase, float fixedDistance = 1.0f)
	{
		// Extract normal from Sbase.
		DirectX::XMVECTOR planeNormal = Sbase;

		// Calculate factor t.
		DirectX::XMVECTOR dotVec = DirectX::XMVector3Dot(camDir, planeNormal);
		float dot = DirectX::XMVectorGetX(dotVec);
		float lerpFactor = fabsf(dot);

		// Method 1: intersection with Sbase.
		DirectX::XMVECTOR aimPoint1;
		{
			// If the camera is looking away from the plane, invert the vector.
			DirectX::XMVECTOR effectiveDir = camDir;
			if (dot > 0.0f)
				effectiveDir = DirectX::XMVector3Reflect(camDir, planeNormal);

			// Calculate intersection with Sbase.
			DirectX::XMVECTOR planeDotOrigin = DirectX::XMPlaneDotCoord(Sbase, camPos);
			DirectX::XMVECTOR planeDotDir = DirectX::XMPlaneDotNormal(Sbase, effectiveDir);

			// Avoid divinding by zero (the camera is parallel).
			float denominator = DirectX::XMVectorGetX(planeDotDir);
			if (fabsf(denominator) < 0.0001f)
				aimPoint1 = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(effectiveDir, fixedDistance)); // Fallback: if we are perfectly parallel, just use a point far away.
			else
			{
				float t = -DirectX::XMVectorGetX(planeDotOrigin) / denominator;
				aimPoint1 = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(effectiveDir, t));
			}
		}

		// --------------------------------------------
		// --- METHODE 2: Fixed Distance Projection ---
		// --------------------------------------------
		
		// Method 2: Project a point in front of the camera on the plane.
		DirectX::XMVECTOR aimPoint2;
		{
			// Take a point at a fixed distance from the camera.
			DirectX::XMVECTOR pointInFront = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(camDir, fixedDistance));

			// Project point on the plane.
			DirectX::XMVECTOR distVec = DirectX::XMPlaneDotCoord(Sbase, pointInFront);
			DirectX::XMVECTOR displacement = DirectX::XMVectorMultiply(distVec, planeNormal);
			aimPoint2 = DirectX::XMVectorSubtract(pointInFront, displacement);
		}

		// Interpolate between both methods based on camera angle.
		// If lerpFactor is 1 (looking down), use AimPoint1.
		// If lerpFactor is 0 (horizon), use AimPoint2.
		return DirectX::XMVectorLerp(aimPoint2, aimPoint1, lerpFactor);
	}

	bool CalculateProjectorMatrix(DirectX::XMMATRIX& projectorMatrix, const Camera3D& camera, const DirectX::XMVECTOR& oceanPosition, float maxWaveHeight)
	{
		// Create planes.
		DirectX::XMVECTOR sBase, sUpper, sLower;
		{
			float oceanY = DirectX::XMVectorGetY(oceanPosition);

			sBase = CreateHorizontalPlane(oceanY);
			sUpper = CreateHorizontalPlane(oceanY + maxWaveHeight);
			sLower = CreateHorizontalPlane(oceanY - maxWaveHeight);
		}

		// Aim the projector to avoid backfiring.
		DirectX::XMMATRIX mProjector = DirectX::XMMatrixIdentity(), projViewProj = DirectX::XMMatrixIdentity();
		{
			// Standard UP vector (World +Y).
			DirectX::XMVECTOR up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

			DirectX::XMVECTOR projPos = CalculateProjectorPosition(camera.GetPositionVector(), -DirectX::XMVectorGetW(sUpper));
			DirectX::XMVECTOR aimPoint = CalculateAimPoint(projPos, camera.GetForwardVector(), sBase);

			// Build matrices.
			DirectX::XMMATRIX projView = DirectX::XMMatrixLookAtLH(projPos, aimPoint, up);
			projViewProj = DirectX::XMMatrixMultiply(projView, camera.GetProjectionMatrix());

			DirectX::XMVECTOR determinant;
			mProjector = XMMatrixInverse(&determinant, projViewProj);
		}

		// Creating the range conversion matrix.
		DirectX::XMMATRIX rangeMatrix = DirectX::XMMatrixIdentity();
		{
			DirectX::XMMATRIX cameraViewProj = camera.GetViewMatrix() * camera.GetProjectionMatrix();
			if (!CreateRangeConversionMatrix(cameraViewProj, projViewProj, rangeMatrix, sBase, sUpper, sLower))
				return false;
		}

		projectorMatrix = XMMatrixMultiply(rangeMatrix, mProjector);

		return true;
	}
}