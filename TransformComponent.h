////////////////////////////////////////////////////////////////////////////////
// Filename: TransformComponent.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _TRANSFORM_COMPONENT_H_
#define _TRANSFORM_COMPONENT_H_

//////////////
// INCLUDES //
//////////////

#include <d3d11.h>

////////////////
// COMPONENTS //
////////////////

namespace Kaizen::Components
{
    struct TransformComponent
    {
        DirectX::XMFLOAT3 Position = { 0, 0, 0 };
        DirectX::XMFLOAT3 Rotation = { 0, 0, 0 };
        DirectX::XMFLOAT3 Scale    = { 1, 1, 1 };

        // De Matrix slaan we hier op (caching), maar we berekenen hem hier NIET.
        mutable DirectX::XMMATRIX WorldMatrix;
        mutable bool IsDirty = true; // Vlaggetje: is er iets veranderd?

        void SetPosition(float x, float y, float z)
        {
            Position = DirectX::XMFLOAT3(x, y, z);
            IsDirty = true;
        }
    };
}

#endif // !_TRANSFORM_COMPONENT_H_