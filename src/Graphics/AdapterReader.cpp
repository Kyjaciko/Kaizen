////////////////////////////////////////////////////////////////////////////////
// Filename: AdapterReader.cpp
////////////////////////////////////////////////////////////////////////////////

#include "AdapterReader.h"

std::vector<DirectX11::AdapterData> DirectX11::AdapterReader::m_Adapters;

namespace DirectX11
{
	AdapterData::AdapterData(IDXGIAdapter* pAdapter)
	{
		m_pAdapter = pAdapter;
		HRESULT hr = m_pAdapter->GetDesc(&m_Description);
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to Get Description for IDXGIAdapter.");
	}
}

namespace DirectX11
{
	std::vector<AdapterData> AdapterReader::GetAdapters()
	{
		// Check if already initialized.
		if (m_Adapters.size() > 0)
			return m_Adapters;

		Microsoft::WRL::ComPtr<IDXGIFactory> p_factory;
		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(p_factory.GetAddressOf()));
		COM_ERROR_IF_FAILED_EXIT(hr, "Failed to create DXGIFactory for enumerating adapters.");

		IDXGIAdapter* p_adapter;
		UINT index = 0;
		while (SUCCEEDED(p_factory->EnumAdapters(index, &p_adapter)))
		{
			m_Adapters.push_back(AdapterData(p_adapter));
			index++;
		}

		return m_Adapters;
	}
}