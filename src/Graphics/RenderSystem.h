////////////////////////////////////////////////////////////////////////////////
// Filename: RenderSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDER_SYSTEM_H_
#define _RENDER_SYSTEM_H_

//////////////
// INCLUDES //
//////////////

#include <memory>
#include <cassert>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Material.h"
#include "../Systems/ECS/System.h"
#include "../Systems/ECS/Coordinator.h"
#include "Graphics/ConstantBufferTypes.h"
#include "../Demos/Where Penguins Dwell/Systems/AnimationComponent.h"
#include "../Demos/Where Penguins Dwell/Systems/Components.h"
#include "TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: RenderSystem
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Graphics
{
    /*struct SpriteComponent
    {
        float w, h;
        int cols, rows;
        float u = 0, v = 0;
        float uw = 1, vh = 1;

        float GetFrameWidth() const { return w / cols; }
        float GetFrameHeight() const { return h / rows; }
    };*/

    struct SpriteComponent
    {
        // Actuele UV's voor de shader (wordt geüpdatet door AnimationSystem)
        float u = 0.0f, v = 0.0f;
        float uw = 1.0f, vh = 1.0f;

        // Actuele pixelgrootte voor het schalen van de mesh (wordt geüpdatet door AnimationSystem)
        float CurrentPixelWidth = 1.0f;
        float CurrentPixelHeight = 1.0f;

        float RenderOffsetX = 0.0f;
        float RenderOffsetY = 0.0f;
    };

    struct MeshRendererComponent 
    {
        std::shared_ptr<MeshResource> mesh;
        std::shared_ptr<IMaterial> material;
    };

	class RenderSystem : public Logic::System
	{
    public:
        void Render(Logic::Coordinator& coordinator, ID3D11DeviceContext* deviceContext, Kaizen::Resources::TextureManager* textureManager, DirectX11::ConstantBuffer<DirectX11::CB_Object>& m_CB_Object)
        {
            for (auto const& entity : m_Entities)
            {
                auto& renderer = coordinator.GetComponent<MeshRendererComponent>(entity);
                auto& transform = coordinator.GetComponent<Components::TransformComponent>(entity);

                if (!renderer.mesh || !renderer.material) 
                    continue;

                assert(renderer.mesh->GetFormat() == renderer.material->GetRequiredFormat() && "Mesh and material format don't match!");

                DirectX::XMMATRIX worldMatrix;
                if (coordinator.HasComponent<Kaizen::Graphics::SpriteComponent>(entity))
                {
                    /*auto& sprite = coordinator.GetComponent<SpriteComponent>(entity);

                    float finalScaleX = sprite.GetFrameWidth() * transform.Scale.x;
                    float finalScaleY = sprite.GetFrameHeight() * transform.Scale.y;

                    worldMatrix = DirectX::XMMatrixScaling(finalScaleX, finalScaleY, 1.0f) *
                                  DirectX::XMMatrixRotationRollPitchYaw(transform.Rotation.x, transform.Rotation.y, transform.Rotation.z) *
                                  DirectX::XMMatrixTranslation(transform.Position.x, transform.Position.y, transform.Position.z);
                
                    DirectX11::CB_SpriteData spriteData;
                    spriteData.uvTransform = DirectX::XMFLOAT4(sprite.u, sprite.v, sprite.uw, sprite.vh);
                    renderer.material->UpdateData(&spriteData);*/

                    auto& sprite = coordinator.GetComponent<SpriteComponent>(entity);

                    // VERANDERD: Gebruik nu de dynamische pixelgrootte
                    float finalScaleX = sprite.CurrentPixelWidth * transform.Scale.x;
                    float finalScaleY = sprite.CurrentPixelHeight * transform.Scale.y;

                    float renderPosX = transform.Position.x - sprite.RenderOffsetX;
                    float renderPosY = transform.Position.y - sprite.RenderOffsetY;

                    worldMatrix = DirectX::XMMatrixScaling(finalScaleX, finalScaleY, 1.0f) *
                        DirectX::XMMatrixRotationRollPitchYaw(transform.Rotation.x, transform.Rotation.y, transform.Rotation.z) *
                        DirectX::XMMatrixTranslation(renderPosX, renderPosY, transform.Position.z);

                    DirectX11::CB_SpriteData spriteData;
                    spriteData.uvTransform = DirectX::XMFLOAT4(sprite.u, sprite.v, sprite.uw, sprite.vh);
                    renderer.material->UpdateData(&spriteData);
                }
                else
                {
                    worldMatrix = transform.WorldMatrix;
                }

                deviceContext->VSSetConstantBuffers(1, 1, m_CB_Object.GetAddressOf());
                m_CB_Object.data.worldMatrix = worldMatrix;
                m_CB_Object.ApplyChanges();

                renderer.material->Bind(deviceContext, textureManager);
                renderer.mesh->Bind(deviceContext);
                deviceContext->DrawIndexed(renderer.mesh->GetIndexCount(), 0, 0);
            }
        }
	};

    // Systems/TransformSystem.h
    class TransformSystem : public Logic::System
    {
    public:
        void Update(Logic::Coordinator& coordinator)
        {
            for (auto const& entity : m_Entities)
            {
                auto& transform = coordinator.GetComponent<Components::TransformComponent>(entity);
                if (!transform.IsDirty)
                    continue;

                transform.WorldMatrix = DirectX::XMMatrixScaling(transform.Scale.x, transform.Scale.y, transform.Scale.z) 
                    * DirectX::XMMatrixRotationRollPitchYaw(transform.Rotation.x, transform.Rotation.y, transform.Rotation.z) 
                    * DirectX::XMMatrixTranslation(transform.Position.x, transform.Position.y, transform.Position.z);
                transform.IsDirty = false;
            }
        }
    };

    class AnimationSystem : public Kaizen::Logic::System
    {
    public:
        void Update(Kaizen::Logic::Coordinator& coordinator, float dt)
        {
            for (const auto& entity : m_Entities)
            {
                /*auto& anim = coordinator.GetComponent<AnimationComponent>(entity);
                auto& sprite = coordinator.GetComponent<Kaizen::Graphics::SpriteComponent>(entity);

                // 1. Haal de snelheid van de huidige staat op
                float timePerFrame = 1 / Kaizen::Constant::STANDARD_ANIMATION_FPS;
                if (anim.StateSpeeds.find(anim.CurrentState) != anim.StateSpeeds.end())
                    timePerFrame = anim.StateSpeeds[anim.CurrentState];

                // 2. Update Timer
                anim.Timer += dt;

                // 3. Volgend frame?
                if (anim.Timer >= timePerFrame)
                {
                    anim.Timer -= timePerFrame; // Behoud resttijd voor soepele animatie
                    anim.CurrentFrame++;
                    anim.CurrentFrame %= sprite.cols;
                }

                // 4. Update de SpriteComponent UV's (De RenderSystem leest dit later uit)

                // Rij berekenen: Cast de Enum naar int (want enum waarde = rij index)
                int row = static_cast<int>(anim.CurrentState);

                // Kolom is het huidige frame
                int col = anim.CurrentFrame;

                // Schrijf naar Sprite Data (RenderSystem gebruikt dit voor de Constant Buffer)
                // U = Kolom * Breedte van 1 frame
                // V = Rij * Hoogte van 1 frame
                sprite.u = col * sprite.uw;
                sprite.v = row * sprite.vh;*/

                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);
                auto& sprite = coordinator.GetComponent<Kaizen::Graphics::SpriteComponent>(entity);

                // 1. Zoek de configuratie van de HUIDIGE animatiestaat op
                auto it = anim.Animations.find(anim.CurrentState);
                if (it == anim.Animations.end())
                    continue; // Veiligheidscheck voor als een state (nog) geen data heeft

                const AnimationData& animData = it->second;

                // 2. Update Timer
                anim.Timer += dt;

                // 3. Volgend frame? 
                if (anim.Timer >= animData.TimePerFrame)
                {
                    anim.Timer -= animData.TimePerFrame; // Behoud resttijd
                    anim.CurrentFrame++;
                    anim.CurrentFrame %= animData.FrameCount; // Loop op basis van het correcte aantal frames voor déze rij
                }

                // 4. Schrijf de juiste data naar de SpriteComponent voor de RenderSystem

                // UV Breedte en Hoogte van dit specifieke frame
                sprite.uw = animData.FrameUVWidth;
                sprite.vh = animData.FrameUVHeight;

                // U = StartU van de rij + (huidig frame * UV breedte van een frame)
                sprite.u = animData.StartU + (anim.CurrentFrame * animData.FrameUVWidth);

                // V = StartV van de rij (blijft constant per animatiestaat)
                sprite.v = animData.StartV;

                // Geef de pixelafmetingen door zodat de RenderSystem de mesh juist kan schalen
                sprite.CurrentPixelWidth = animData.FramePixelWidth;
                sprite.CurrentPixelHeight = animData.FramePixelHeight;

                sprite.RenderOffsetX = animData.RenderOffsetX;
                sprite.RenderOffsetY = animData.RenderOffsetY;
            }
        }
    };
}

#endif // !_RENDER_SYSTEM_H_