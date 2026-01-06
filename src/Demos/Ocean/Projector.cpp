#include "Projector.h"

////////////////////////////////////////////////////////////////////////////////
// Filename: Projector.h
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	/*// ray = rayO + rayD * t.
	bool RayPlaneIntersection(DirectX::XMVECTOR rayO, DirectX::XMVECTOR rayD, DirectX::XMVECTOR planePoint, DirectX::XMVECTOR normal, DirectX::XMVECTOR& outPoint)
	{
		using namespace DirectX;

		// Parallel?
		float denom = XMVectorGetX(XMVector3Dot(rayD, normal));
		if (fabsf(denom) < 1e-6f) return false;

		// Behind camera?
		float t = XMVectorGetX(XMVector3Dot(planePoint - rayO, normal)) / denom;
		if (t < 0.0f) return false;

		outPoint = rayO + rayD * t;
		return true;
	}

	// Only works on XZ-planes.
	bool CalculateRangeConversionMatrix(const DirectX::XMMATRIX& vpMatrix, DirectX::XMMATRIX& rangeMatrix, const DirectX::XMMATRIX& projectorMatrix, float heightScale, float heightSBase)
	{
		using namespace DirectX;

		const float waterY = 0.0f;                // y-hoogte van S_base
		const XMVECTOR planeP = XMVectorSet(0, waterY, 0, 0); // punt op het vlak
		const XMVECTOR planeN = XMVectorSet(0, 1, 0, 0);

		////////////////////////////////////////////////////////////////////////////////
		// STEP 1: Transform the corner-points of the camera frustum into world-space // 
		////////////////////////////////////////////////////////////////////////////////

		// Corner-points of the camera frustum in clip-space.
		static const XMVECTORF32 cornerPoints[8] =
		{
			{ -1.0f,  1.0f, 0.0f, 0.0f }, { 1.0f,  1.0f, 0.0f, 0.0f }, { 1.0f, -1.0f, 0.0f, 0.0f }, { -1.0f, -1.0f, 0.0f, 0.0f }, // near
			{ -1.0f,  1.0f, 1.0f, 0.0f }, { 1.0f,  1.0f, 1.0f, 0.0f }, { 1.0f, -1.0f, 1.0f, 0.0f }, { -1.0f, -1.0f, 1.0f, 0.0f }  // far
		};

		XMMATRIX inv_vp_matrix = XMMatrixInverse(nullptr, vpMatrix);

		// Corner-points in world space.
		XMVECTOR worldPoints[8];
		for (int i = 0; i < 8; i++)
			worldPoints[i] = XMVector3TransformCoord(cornerPoints[i], inv_vp_matrix);

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// STEP 2: Check for intersections between the edges of the camera frustum and the bound planes // 
		//////////////////////////////////////////////////////////////////////////////////////////////////

		std::vector<XMVECTOR> buffer;
		auto pushCornerIfInside = [&](XMVECTOR p)
			{
				float y = XMVectorGetY(p);
				if (y > -heightScale && y < heightScale) buffer.push_back(p);
			};

		for (int i = 0; i < 8 ;++i) 
			pushCornerIfInside(worldPoints[i]);

		// (b) alle 12 randen van het frustum
		static const int edge[12][2] =
		{
			{0,1},{1,2},{2,3},{3,0},      // near
			{4,5},{5,6},{6,7},{7,4},      // far
			{0,4},{1,5},{2,6},{3,7}       // vertical
		};
		for (auto& e : edge)
		{
			XMVECTOR h;
			if (ClipLineWithYPlane(worldPoints[e[0]], worldPoints[e[1]], heightScale, h)) buffer.push_back(h);
			if (ClipLineWithYPlane(worldPoints[e[0]], worldPoints[e[1]], -heightScale, h)) buffer.push_back(h);
		}

		// No intersection found, skip rendering.
		if (buffer.empty())
		{
			rangeMatrix = XMMatrixIdentity();
			return false;
		}

		///////////////////////////////////////////////////
		// STEP 3: Project all points onto Sbase (y = 0) //
		///////////////////////////////////////////////////

		for (XMVECTOR& p : buffer)
			p = XMVectorSetY(p, heightSBase);

		///////////////////////////////////////////////////////
		// STEP 4: Naar projector-space en uv-bereik bepalen //
		///////////////////////////////////////////////////////

		Span2D span;
		const XMMATRIX inv_projector = XMMatrixInverse(nullptr, projectorMatrix);
		for (const XMVECTOR& pW : buffer)
		{
			XMVECTOR ndc = XMVector3TransformCoord(pW, inv_projector);     // wereld => projector-clip

			float u = 0.5f * XMVectorGetX(ndc) + 0.5f;                    // map [-1,1] => [0,1]
			float v = 0.5f * XMVectorGetY(ndc) + 0.5f;
			Accumulate(span, u, v);
		}

		////////////////////////////////
		// STEP 5: Create rangeMatrix //
		////////////////////////////////

		float sx = span.xmax - span.xmin;
		float sy = span.ymax - span.ymin;
		rangeMatrix =
			XMMatrixScaling(sx, sy, 1.0f) *
			XMMatrixTranslation(span.xmin, span.ymin, 0.0f);

		//rangeMatrix = XMMatrixTranspose(rangeMatrix);

		return true;
	}

	DirectX::XMVECTOR CalculateProjectorLookAtPoint(const Camera3D& camera, DirectX::XMVECTOR pBase, DirectX::XMVECTOR nBase, float farZ)
	{
		using namespace DirectX;

		XMVECTOR n_plane = XMVector3Normalize(nBase);

		// Method 1: Aim the projector at the point where the view-vector of the camera intersects the ocean base plane.
		XMVECTOR ray = camera.GetForwardVector();
		float facing_ocean_plane = XMVectorGetX(XMPlaneDot(camera.GetForwardVector(), n_plane));

		// Mirror the ray if the camera is looking away.
		if (facing_ocean_plane >= 0.0f)
			ray -= n_plane * (2.0f * facing_ocean_plane);

		XMVECTOR look_at_1;
		bool has_hit = RayPlaneIntersection(camera.GetPositionVector(), ray, pBase, n_plane, look_at_1);

		// Method 2: Calculates a point at a fixed distance from the camera in its forward direction. The projector is aimed at this point projected onto S_base.
		XMVECTOR ahead = camera.GetPositionVector() + camera.GetForwardVector() * farZ;
		float height = XMVectorGetX(XMVector3Dot(ahead - pBase, n_plane));
		XMVECTOR look_at_2 = ahead - n_plane * height;

		// Final: The points are interpolated between depending on the direction of the camera.
		const float k = 0.4f; // When to switch from Method 1 to 2: k = 1 - cos(angle) => 53° = 0.4f.
		float down = fabsf(XMVectorGetX(XMVector3Dot(camera.GetForwardVector(), -n_plane)));
		float blend_factor = std::clamp((1.0f - down) / k, 0.0f, 1.0f);

		if (!has_hit)
			return look_at_2;

		return XMVectorLerp(look_at_1, look_at_2, blend_factor);
	}*/


	/// // TEST 2.4.2

	std::array<DirectX::XMFLOAT3, 8> GetFrustumCornersWorldSpace(const DirectX::XMMATRIX& viewProjectionMatrix)
	{
		// Calculate camera’s inverted viewproj matrix.
		DirectX::XMVECTOR determinant;
		DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&determinant, viewProjectionMatrix);

		// Define the 8 cornerpoints in NDC.
		float zNear = 0.0f; // Should be -1.0f if OpenGL is used.
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
		// Indices who from the 12 edges (defined in step 1 cornersNDC).
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

			// Check 1: Intersection with Supper.
			if (IntersectSegmentPlane(corners[idxA], corners[idxB], Supper, intersectPt))
			{
				DirectX::XMStoreFloat3(&pt, intersectPt);
				outBuffer.push_back(pt);
			}

			// Check 2: Intersection with Slower.
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

			// Calculate distance to upper and lower plane.
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

			// Calculate distance to the plane (signed distance).
			DirectX::XMVECTOR distanceVec = DirectX::XMPlaneDotCoord(Sbase, p);
			DirectX::XMVECTOR displacement = DirectX::XMVectorMultiply(distanceVec, planeNormal);

			// Project the point on Sbase.
			DirectX::XMVECTOR pProjected = DirectX::XMVectorSubtract(p, displacement);
			DirectX::XMStoreFloat3(&intersectionPoints[i], pProjected);
		}
	}

	struct ProjectorRange
	{
		float minX, maxX;
		float minY, maxY;

		ProjectorRange(float minX = 0.f, float minY = 0.f, float maxX = 0.f, float maxY = 0.f)
			: minX(minX), maxX(maxX), minY(minY), maxY(maxY)
		{
		}
	};

	ProjectorRange CalculateVisibleSpan(std::vector<DirectX::XMFLOAT3>& pointsOnSbase, const DirectX::XMMATRIX& projectorViewProj)
	{
		// Calculate inverse projector matrix.
		/*DirectX::XMVECTOR determinant;
		DirectX::XMMATRIX invProjection = DirectX::XMMatrixInverse(&determinant, projectionMatrix);*/

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
		// Vlak vergelijking: 0x + 1y + 0z - height = 0
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

	///// TEST

	// 2.4.1
	DirectX::XMVECTOR CalculateProjectorPosition(DirectX::FXMVECTOR camPos, float SupperY)
	{
		// Create a bound zo we never go lower than our Upper plane (Supper).
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

		// Use absolute value as interpolationvalue [0.0f, 1.0f].
		float lerpFactor = fabsf(dot);

		// -------------------------------
		// --- METHODE 1: Intersection ---
		// -------------------------------

		DirectX::XMVECTOR aimPoint1;

		// If the camera is looking away from the plane, invert the vector.
		DirectX::XMVECTOR effectiveDir = camDir;
		if (dot > 0.0f)
			effectiveDir = DirectX::XMVector3Reflect(camDir, planeNormal);

		// Calculate intersection with Sbase.
		DirectX::XMVECTOR planeDotOrigin = DirectX::XMPlaneDotCoord(Sbase, camPos);
		DirectX::XMVECTOR planeDotDir = DirectX::XMPlaneDotNormal(Sbase, effectiveDir);

		// Avoid divinding by zero, meaning we're parallel.
		float denominator = DirectX::XMVectorGetX(planeDotDir);
		if (fabsf(denominator) < 0.0001f)
			aimPoint1 = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(effectiveDir, fixedDistance)); // Fallback: if we are perfectly parallel, just use a point far away.
		else
		{
			float t = -DirectX::XMVectorGetX(planeDotOrigin) / denominator;
			aimPoint1 = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(effectiveDir, t));
		}

		// --------------------------------------------
		// --- METHODE 2: Fixed Distance Projection ---
		// --------------------------------------------
		
		// Take a point at a fixed for the camera.
		DirectX::XMVECTOR pointInFront = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(camDir, fixedDistance));

		// Project point on the plane.
		DirectX::XMVECTOR distVec = DirectX::XMPlaneDotCoord(Sbase, pointInFront);
		DirectX::XMVECTOR displacement = DirectX::XMVectorMultiply(distVec, planeNormal);
		DirectX::XMVECTOR aimPoint2 = DirectX::XMVectorSubtract(pointInFront, displacement);

		// Interpolate between both methods based on camera angle.
		// If lerpFactor is 1 (looking down), use AimPoint1.
		// IF lerpFactor is 0 (horizon), use AimPoint2.
		return DirectX::XMVectorLerp(aimPoint2, aimPoint1, lerpFactor);
	}

	/*void GetProjectorMatrices(DirectX::FXMVECTOR camPos, DirectX::FXMVECTOR camDir, DirectX::FXMVECTOR Sbase, float planeUpperY, const DirectX::XMMATRIX& projectionMatrix, DirectX::XMMATRIX& projectorMatrix)
	{
		// Determine projector position.
		DirectX::XMVECTOR projPos = CalculateProjectorPosition(camPos, planeUpperY);

		// Determine aim point.
		DirectX::XMVECTOR aimPoint = CalculateAimPoint(projPos, camDir, Sbase);

		// NOG AANPASSEN
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		// Build matrices.
		DirectX::XMMATRIX projectorView = DirectX::XMMatrixLookAtLH(projPos, aimPoint, up);
		DirectX::XMMATRIX invProjectorMatrix = DirectX::XMMatrixMultiply(projectorView, projectionMatrix);

		DirectX::XMVECTOR determinant;
		projectorMatrix = DirectX::XMMatrixInverse(&determinant, invProjectorMatrix);
	}*/
	// 2.4.1

	bool CalculateProjectorMatrix(DirectX::XMMATRIX& projectorMatrix, const Camera3D& camera, const DirectX::XMVECTOR& oceanPosition, float maxWaveHeight)
	{
		// Create planes.
		// Sbase: Het waterniveau
		float oceanY = DirectX::XMVectorGetY(oceanPosition);
		DirectX::XMVECTOR sBase = CreateHorizontalPlane(oceanY);

		// Supper: Waterniveau + amplitude (veilige marge)
		// De 'D' component is negatief, dus -(Y + Amp)
		DirectX::XMVECTOR sUpper = CreateHorizontalPlane(oceanY + maxWaveHeight);

		// Slower: Waterniveau - amplitude
		DirectX::XMVECTOR sLower = CreateHorizontalPlane(oceanY - maxWaveHeight);

		// -------------
		// --- 2.4.1 ---
		// -------------

		// Determine projector position.
		DirectX::XMVECTOR projPos = CalculateProjectorPosition(camera.GetPositionVector(), -DirectX::XMVectorGetW(sUpper));

		// Determine aim point.
		DirectX::XMVECTOR aimPoint = CalculateAimPoint(projPos, camera.GetForwardVector(), sBase);

		// Standard UP vector (World Y).
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

		/*// Check if projector lookingstraight up/down, if so use different up vector.
		float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(aimPoint, projPos)), up));
		if (fabsf(dot) > 0.99f)
			up = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);*/

		// Build matrices.
		DirectX::XMMATRIX projView = DirectX::XMMatrixLookAtLH(projPos, aimPoint, up);
		DirectX::XMMATRIX projViewProj = DirectX::XMMatrixMultiply(projView, camera.GetProjectionMatrix());

		DirectX::XMVECTOR determinant;
		DirectX::XMMATRIX mProjector = XMMatrixInverse(&determinant, projViewProj);

		// -------------
		// --- 2.4.2 ---
		// -------------

		DirectX::XMMATRIX rangeMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX cameraViewProj = camera.GetViewMatrix() * camera.GetProjectionMatrix();
		if (!CreateRangeConversionMatrix(cameraViewProj, projViewProj, rangeMatrix, sBase, sUpper, sLower))
			return false;

		// ------------------
		// --- Final step ---
		// ------------------

		projectorMatrix = XMMatrixMultiply(rangeMatrix, mProjector);

		return true;
		/*// Step 2: Aim the projector.
		// TODO: Up should be based on the camera.
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMMATRIX view_matrix = XMMatrixLookAtLH(camera.GetPositionVector(), CalculateProjectorLookAtPoint(camera, pBase, nBase, farZ), up);
		XMMATRIX vp_matrix = XMMatrixMultiply(view_matrix, camera.GetProjectionMatrix());
		XMMATRIX inv_vp_matrix = XMMatrixInverse(nullptr, vp_matrix);

		// Step 1: Determine if any part of the displaceable volume is within the camera frustum. Abort rendering of the surface otherwise.
		XMMATRIX range_matrix = XMMatrixIdentity();
		if (!CalculateRangeConversionMatrix(camera.GetViewMatrix() * camera.GetProjectionMatrix(), range_matrix, vp_matrix))
			return false;

		// Step 3: Convert the range to the intersection between V_cam and V_displaceable.
		projectorMatrix = range_matrix * inv_vp_matrix;
		return true;*/
	}
}