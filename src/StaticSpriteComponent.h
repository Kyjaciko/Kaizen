////////////////////////////////////////////////////////////////////////////////
// Filename: StaticSpriteComponent.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _STATIC_SPRITE_COMPONENT_H_
#define _STATIC_SPRITE_COMPONENT_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "IComponent.h"
#include "Constants.h"
#include "TypeDefines.h"
#include "RenderTypes.h"

#include "Graphics/GameObject2D.h"
//#include "Graphics/ConstantBufferTypes.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: StaticSpriteComponent
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Component
{
    /**
     * Represents a sprite that uses frame-based, each of the same size (static), animations.
     *
     * Rules:
     * - Every animation is a 1D array corresponding to a row.
     * - All animations must have the same length and frame size.
     */
    class StaticSpriteComponent : public IComponent<DirectX11::VertexPosUV, DirectX11::CB_SpriteData>
    {
    public:
        // Explicitly call base constructor with required arguments.
        StaticSpriteComponent(Types::TextureID textureID, int columns, int rows)
            : IComponent<DirectX11::VertexPosUV, DirectX11::CB_SpriteData>()
            , textureID(textureID), columns(columns), rows(rows)
        {
            frameDuration = 1.f / static_cast<float>(Constant::STANDARD_ANIMATION_FPS);

            uw = 1.f / static_cast<float>(columns);
            vh = 1.f / static_cast<float>(rows);
        }

        void SetAnimation(int row, int fps)
        {
            frameDuration = 1.f / static_cast<float>(fps);
            if (currentAnimation == row)
                return;

            currentAnimation = row;
            v = currentAnimation * vh;

            currentFrame = 0;
            timer = 0;
        }

        virtual void Update(float deltaTime) override 
        {
            timer += deltaTime;
            if (timer < frameDuration)
                return;

            timer = 0;
            currentFrame = (currentFrame + 1) % columns;
            u = currentFrame * uw;
        }

		const float& GetU() const { return u; }
		const float& GetV() const { return v; }
		const float& GetWidth() const { return uw; }
		const float& GetHeight() const { return vh; }
		const Types::TextureID& GetTextureID() const { return textureID; }

    private:
        // Texture info.
        Types::TextureID textureID;
        int columns, rows;
        
        // Animation.
        float timer = 0.f;
        float frameDuration;

        int currentFrame = 0;
        int currentAnimation = 0;

        // Shader variables.
        float u = 0.f, v = 0.f;
        float uw, vh;

		Kaizen::Graphics::SpriteInstanceData m_InstanceData;
    };
}

#endif // !_STATIC_SPRITE_COMPONENT_H_