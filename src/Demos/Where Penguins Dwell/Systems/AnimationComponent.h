#pragma once

#include <unordered_map>

enum class EntityState
{
	idle,
	walkingLeft,
	walkingRight,
	falling,
	tumbling,
	floating,
	climbingOnTheRight,
	climbingOnTheLeft,
    reading,
    digging,
    supermanLeftHand,
	supermanRightHand,
    zap_walkingLeft,
	zap_walkingRight,
    exit_walkingLeft,
	exit_walkingRight,
    bomber,
    splash,
	exploding
};

struct FrameSize
{
    float w, h;
};

struct AnimRowConfig
{
    FrameSize Size;
    int Columns;
    float FPS = 8.0f;

	// Required if ColliderBox size differs from frame size.
    float OffsetX = 0.0f;
    float OffsetY = 0.0f;
};

struct AnimationData
{
    int FrameCount = 1;
    float TimePerFrame = 0.1f;

    // UV Coördinaten (0.0 tot 1.0) voor de shader
    float StartU = 0.0f;
    float StartV = 0.0f;
    float FrameUVWidth = 1.0f;
    float FrameUVHeight = 1.0f;

    // Pixel afmetingen voor de schaling van de mesh in RenderSystem
    float FramePixelWidth = 32.0f;
    float FramePixelHeight = 32.0f;

    // NIEUW: Bewaar de offset per animatie
    float RenderOffsetX = 0.0f;
    float RenderOffsetY = 0.0f;
};

/*struct AnimationComponent
{
    float Timer = 0.0f;
    int CurrentFrame = 0;
    EntityState CurrentState = EntityState::idle;
    std::unordered_map<EntityState, float> StateSpeeds; // Instellingen per animatie (Staat -> FPS)

    // Helper om makkelijk animaties toe te voegen
    void AddState(EntityState state, float fps)
    {
        StateSpeeds[state] = 1.0f / fps; // We slaan de 'tijd per frame' op
    }

    constexpr void ChangeState(const EntityState newState) noexcept
    {
        if (CurrentState == newState) [[unlikely]]
            return;

        CurrentState = newState;
        CurrentFrame = 0;
        Timer = 0.0f;
    }
};*/

struct AnimationComponent
{
    float Timer = 0.0f;
    int CurrentFrame = 0;
    EntityState CurrentState = EntityState::reading;

    std::unordered_map<EntityState, AnimationData> Animations;

    // Nieuwe automatische setup functie
    void SetupFromConfig(float textureWidth, float textureHeight, const std::vector<AnimRowConfig>& configs)
    {
        float currentPixelY = 0.0f;

        for (size_t i = 0; i < configs.size(); ++i)
        {
            // We gaan er hier vanuit dat de volgorde in de config overeenkomt met je EntityState enum volgorde
            EntityState state = static_cast<EntityState>(i);

            float width = configs[i].Size.w;
            float height = configs[i].Size.h;
            int frames = configs[i].Columns;
            float fps = configs[i].FPS;

            AnimationData data;
            data.FrameCount = frames;
            data.TimePerFrame = 1.0f / fps;

            data.StartU = 0.0f; // We beginnen altijd links op de texture (X = 0)
            data.StartV = currentPixelY / textureHeight; // Automatische Y-berekening

            data.FrameUVWidth = width / textureWidth;
            data.FrameUVHeight = height / textureHeight;

            data.FramePixelWidth = width;
            data.FramePixelHeight = height;

            data.RenderOffsetX = configs[i].OffsetX;
            data.RenderOffsetY = configs[i].OffsetY;

            Animations[state] = data;

            // Schuif de Y-positie op voor de volgende rij in de iteratie
            currentPixelY += height;
        }
    }

    constexpr void ChangeState(const EntityState newState) noexcept
    {
        if (CurrentState == newState) [[unlikely]]
            return;

        CurrentState = newState;
        CurrentFrame = 0;
        Timer = 0.0f;
    }
};