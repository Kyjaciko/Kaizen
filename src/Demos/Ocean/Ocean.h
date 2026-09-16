////////////////////////////////////////////////////////////////////////////////
// Filename: Ocean.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _OCEAN_H_
#define _OCEAN_H_

//////////////
// INCLUDES //
//////////////

#include <DirectXMath.h>
#include "../../Graphics/Color.h"
#include "../../Graphics/GameObject3d.h"
#include "../../Graphics/Shaders.h"

#include "Projector.h"

////////////////////////////////////////////////////////////////////////////////
// Structure: OceanCB
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct PerlinCB
	{
		UINT width;
		UINT height;
		UINT tileCount;
		UINT gridSize;

		UINT numOctaves;
		float totalTime;
	};

	struct OceanCB
	{
		DirectX::XMMATRIX vpMatrix;
		DirectX::XMMATRIX projectorMatrix;

		DirectX::XMFLOAT3 cameraPos;
		float heightScale; // Height of the waves.

		float totalTime;
	};
}

////////////////////////////////////////////////////////////////////////////////
// Structure name: OceanVertex
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct OceanVertex
	{
		DirectX::XMFLOAT3   position;
		DirectX::XMFLOAT2	uv;

		OceanVertex(float x, float y, float z, float u, float v)
			: position(x, y, z)
			, uv(u, v)
		{
		}
	};
}

////////////////////////////////////////////////////////////////////////////////
// Class name: Ocean
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Ocean : public GameObject3D
	{
	public:
		~Ocean();

		bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<PerlinCB>& cbPerlin, ConstantBuffer<OceanCB>& cbOcean, UINT textureWidth, UINT textureHeight);
		void Draw(const Camera3D& camera, const DirectX::XMMATRIX& viewProjectionMatrix, ComputeShader& computeShader, PixelShader& ps, VertexShader& vs, ID3D11SamplerState* const* ppSamplers, double deltaTime);

	private:
		static constexpr float kWaveHeight = 5.f;
		static double m_Time; // Should be fetched from main loop.

		UINT m_Width;
		UINT m_Height;

		IndexBuffer m_Indices;
		VertexBuffer<OceanVertex> m_Vertices;
		ConstantBuffer<PerlinCB>* m_pCB_CS_Perlin;
		ConstantBuffer<OceanCB>* m_pCB_VS_Ocean;

		DirectX::XMMATRIX m_WorldMatrix = DirectX::XMMatrixIdentity();

		ID3D11DeviceContext* m_pDeviceContext;

		ID3D11UnorderedAccessView* perlinUAV = nullptr;
		ID3D11ShaderResourceView* perlinSRV  = nullptr;

	private:
		void SetupGrid(ID3D11Device* device);
		void SetupVertexShaderStage(const DirectX::XMMATRIX& viewProjectionMatrix, const DirectX::XMMATRIX& projectorMatrix, VertexShader& vs, ID3D11SamplerState* const* ppSamplers, double totalTime, const DirectX::XMFLOAT3& pos);
		void SetupPixelShaderStage(PixelShader& ps);
		void SetupInputAssemblerStage(VertexShader& vs);

		void GeneratePerlinNoise(ComputeShader computeShader, double totalTime);
		void CreateUniformGridOfVertices(std::vector<OceanVertex>& vertices, std::vector<DWORD>& indices);

		virtual void UpdateMatrix() override;
	};
}

#endif // !_OCEAN_H_