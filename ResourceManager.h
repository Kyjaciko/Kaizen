////////////////////////////////////////////////////////////////////////////////
// Filename: ResourceManager.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _RESOURCE_MANAGER_H_
#define _RESOURCE_MANAGER_H_

//////////////
// INCLUDES //
//////////////

#include <string>
#include <memory>
#include <unordered_map>
#include <d3d11.h>
#include <wrl/client.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Constants.h"
#include "Types.h"


#include <filesystem>
static std::filesystem::path GetExecutableDir2()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

////////////////////////////////////////////////////////////////////////////////
// Class name: ResourceManager
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Resources
{
    template<typename Resource>
    class ResourceManager
    {
    public:
        ResourceManager(Microsoft::WRL::ComPtr<ID3D11Device> device) : m_pDevice(device) {}

        void Unload(Kaizen::Types::ResourceID id)
        {
            auto it = m_IdToPath.find(id);
            m_Resources.erase(id);

            if (it == m_IdToPath.end())
                return;

            m_PathToId.erase(it->second);
            m_IdToPath.erase(it);
        }

        Resource* GetTexture(Kaizen::Types::ResourceID id) const
        {
            auto it = m_Resources.find(id);
            return (it != m_Resources.end()) ? it->second.get() : nullptr;
        }

    protected:
        Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
        Kaizen::Types::ResourceID m_NextID = Kaizen::Constant::RESOURCE_FIRST_ID;

        // 3 maps for O(1) speed.
        std::unordered_map<Kaizen::Types::ResourceID, std::unique_ptr<Resource>> m_Resources;
        std::unordered_map<std::string, Kaizen::Types::ResourceID>               m_PathToId;
        std::unordered_map<Kaizen::Types::ResourceID, std::string>               m_IdToPath;
    };

    #include "Graphics/Texture.h"

    class TextureManager : public ResourceManager<DirectX11::Texture>
    {
    public:
        Kaizen::Types::ResourceID Load(const std::string& filePath)
        {
            // Check for doubles.
            auto it = m_PathToId.find(filePath);
            if (it != m_PathToId.end())
                return it->second;

            // Load file.
            std::unique_ptr<DirectX11::Texture> texture = std::make_unique<DirectX11::Texture>(m_pDevice.Get(), filePath, aiTextureType_DIFFUSE);

            Kaizen::Types::ResourceID id = m_NextID++;
            m_Resources[id] = std::move(texture);
            m_PathToId[filePath] = id;
            m_IdToPath[id] = filePath;

            return id;
        }
    };

    class VertexShaderManager : public ResourceManager<DirectX11::VertexShader>
    {
    public:
        template<typename VertexType>
        Kaizen::Types::ResourceID Load(Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& shaderPath)
        {
            // Check for doubles.
            auto it = m_PathToId.find(shaderPath);
            if (it != m_PathToId.end())
                return it->second;

            // Load file.
            std::filesystem::path shader_folder = GetExecutableDir2();
#pragma region DetermineShaderPath
            /*if (IsDebuggerPresent())
            {
#ifdef _DEBUG // Debug mode
#ifdef _WIN64 // x64
                shader_folder = L"x64\\Debug\\";
#else // x86 (Win32)
                shader_folder = L"Debug\\";
#endif
#else // Release mode
#ifdef _WIN64 // x64
                shader_folder = L"x64\\Release\\";
#else // x86 (Win32)
                shader_folder = L"Release\\";
#endif
#endif
            }*/

            std::unique_ptr<DirectX11::VertexShader> vertex_shader = std::make_unique<DirectX11::VertexShader>();
            vertex_shader->Init<VertexType>(device, shader_folder / StringHelper::StringToWide(shaderPath));

            Kaizen::Types::ResourceID id = m_NextID++;
            m_Resources[id] = std::move(vertex_shader);
            m_PathToId[shaderPath] = id;
            m_IdToPath[id] = shaderPath;

            return id;
        }
    };

    class PixelShaderManager : public ResourceManager<DirectX11::PixelShader>
    {
    public:
        Kaizen::Types::ResourceID Load(Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& shaderPath)
        {
            // Check for doubles.
            auto it = m_PathToId.find(shaderPath);
            if (it != m_PathToId.end())
                return it->second;

            // Load file.
            std::filesystem::path shader_folder = GetExecutableDir2();
#pragma region DetermineShaderPath
            /*if (IsDebuggerPresent())
            {
#ifdef _DEBUG // Debug mode
#ifdef _WIN64 // x64
                shader_folder = L"x64\\Debug\\";
#else // x86 (Win32)
                shader_folder = L"Debug\\";
#endif
#else // Release mode
#ifdef _WIN64 // x64
                shader_folder = L"x64\\Release\\";
#else // x86 (Win32)
                shader_folder = L"Release\\";
#endif
#endif
            }*/

            std::unique_ptr<DirectX11::PixelShader> pixel_shader = std::make_unique<DirectX11::PixelShader>();
            pixel_shader->Init(device, shader_folder / StringHelper::StringToWide(shaderPath));

            Kaizen::Types::ResourceID id = m_NextID++;
            m_Resources[id] = std::move(pixel_shader);
            m_PathToId[shaderPath] = id;
            m_IdToPath[id] = shaderPath;

            return id;
        }
    };
}

#endif // !_RESOURCE_MANAGER_H_