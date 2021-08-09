#include "./headers/sound-mixer-utils.hpp"
#include <iostream>
#include <Functiondiscoverykeys_devpkey.h>
#include <string>
#include <wchar.h>

inline std::string toString(LPWSTR str)
{
	std::wstring wstr(str);

	return std::string(wstr.begin(), wstr.end());
}

namespace SoundMixerUtils
{

	template <class T>
	void SafeRelease(T **ppT)
	{
		if (ppT && *ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}

	IMMDevice *GetDeviceById(LPWSTR id)
	{

		IMMDeviceEnumerator *pDeviceEnumerator;
		if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pDeviceEnumerator) != S_OK)
		{
			return nullptr;
		}
		IMMDevice *pDevice;
		if (pDeviceEnumerator->GetDevice(id, &pDevice) != S_OK)
		{
			SafeRelease(&pDeviceEnumerator);
			return nullptr;
		}
		SafeRelease(&pDeviceEnumerator);

		return pDevice;
	}

	DeviceDescriptor GetDevice(EDataFlow dataFlow)
	{
		CoInitialize(NULL);

		IMMDeviceEnumerator *pDeviceEnumerator;
		if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pDeviceEnumerator) != S_OK)
		{
			CoUninitialize();
			return DeviceDescriptor{};
		}
		IMMDevice *pDevice;

		if (pDeviceEnumerator->GetDefaultAudioEndpoint(dataFlow, ERole::eConsole, &pDevice) != S_OK)
		{
			SafeRelease(&pDeviceEnumerator);
			CoUninitialize();
			return DeviceDescriptor{};
		}
		SafeRelease(&pDeviceEnumerator);

		LPWSTR id;
		DeviceDescriptor descriptor;
		if (pDevice->GetId(&id) != S_OK)
		{
			CoUninitialize();
			return descriptor;
		}

		descriptor.id = toString(id);
		CoTaskMemFree(id);

		IPropertyStore *pProperties;
		if (pDevice->OpenPropertyStore(STGM_READ, &pProperties) == S_OK)
		{
			std::string fullName;
			PROPVARIANT prop;
			PropVariantInit(&prop);
			if (pProperties->GetValue(PKEY_DeviceInterface_FriendlyName, &prop) == S_OK)
			{
				fullName = toString(prop.pwszVal);
			}
			PropVariantClear(&prop);
			PropVariantInit(&prop);

			if (pProperties->GetValue(PKEY_Device_DeviceDesc, &prop) == S_OK)
			{
				fullName += ' ' + toString(prop.pwszVal);
			}
			PropVariantClear(&prop);

			descriptor.fullName = fullName;
			SafeRelease(&pProperties);
		}
		descriptor.dataFlow = dataFlow;

		SafeRelease(&pDevice);

		CoUninitialize();

		return descriptor;
	}

	std::vector<DeviceDescriptor> GetDevices()
	{

		std::vector<DeviceDescriptor> descriptors;
		CoInitialize(NULL);

		IMMDeviceEnumerator *pDeviceEnumerator;
		if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pDeviceEnumerator) != S_OK)
		{
			return descriptors;
		}

		IMMDeviceCollection *pDevices;
		if (pDeviceEnumerator->EnumAudioEndpoints(EDataFlow::eAll, DEVICE_STATE_ACTIVE, &pDevices) != S_OK)
		{
			SafeRelease(&pDeviceEnumerator);
			return descriptors;
		}
		SafeRelease(&pDeviceEnumerator);

		UINT deviceCount;
		if (pDevices->GetCount(&deviceCount) != S_OK)
		{
			SafeRelease(&pDevices);
			return descriptors;
		}

		LPWSTR id;
		IMMDevice *pDevice;
		IMMEndpoint *pEndpoint;
		IPropertyStore *pProperties;
		for (UINT i = 0; i < deviceCount; i++)
		{

			if (pDevices->Item(i, &pDevice) == S_OK && pDevice->GetId(&id) == S_OK)
			{
				DeviceDescriptor desc;

				desc.id = toString(id);
				CoTaskMemFree(id);
				if (pDevice->OpenPropertyStore(STGM_READ, &pProperties) == S_OK)
				{
					std::string fullName;
					PROPVARIANT prop;
					PropVariantInit(&prop);
					if (pProperties->GetValue(PKEY_DeviceInterface_FriendlyName, &prop) == S_OK)
					{
						fullName = toString(prop.pwszVal);
					}

					PropVariantClear(&prop);
					PropVariantInit(&prop);
					if (pProperties->GetValue(PKEY_Device_DeviceDesc, &prop) == S_OK)
					{
						fullName += ' ' + toString(prop.pwszVal);
					}
					PropVariantClear(&prop);
					desc.fullName = fullName;
				}

				if (pDevice->QueryInterface(__uuidof(IMMEndpoint), (LPVOID *)&pEndpoint) == S_OK)
				{
					EDataFlow flow;
					if (pEndpoint->GetDataFlow(&flow) == S_OK)
					{
						desc.dataFlow = flow;
					}
				}

				descriptors.push_back(desc);
				SafeRelease(&pProperties);
				SafeRelease(&pEndpoint);
			}
			SafeRelease(&pDevice);
		}

		SafeRelease(&pDevices);

		CoUninitialize();

		return descriptors;
	}

	IAudioSessionControl2 *GetAudioSessionByGUID(IMMDevice *pDevice, LPWSTR guid)
	{
		IAudioSessionManager2 *pSessionManager;
		if (pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID *)&pSessionManager) != S_OK)
		{
			return nullptr;
		}

		IAudioSessionEnumerator *pSessionEnumerator;
		if (pSessionManager->GetSessionEnumerator(&pSessionEnumerator) != S_OK)
		{
			SafeRelease(&pSessionManager);
			return nullptr;
		}
		SafeRelease(&pSessionManager);

		IAudioSessionControl *pSessionControl;
		IAudioSessionControl2 *pSessionControl2;
		int size = -1;
		pSessionEnumerator->GetCount(&size);
		LPWSTR id;
		for (size_t i = 0; i < size; i++)
		{
			if (pSessionEnumerator->GetSession(i, &pSessionControl) == S_OK)
			{

				if (pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID *)&pSessionControl2) == S_OK)
				{
					if (pSessionControl2->GetSessionIdentifier(&id) == S_OK && wcscmp(id, guid) == 0)
					{
						CoTaskMemFree(id);
						SafeRelease(&pSessionEnumerator);
						SafeRelease(&pSessionControl);
						return pSessionControl2;
					}
					CoTaskMemFree(id);
					SafeRelease(&pSessionControl2);
				}
				SafeRelease(&pSessionControl);
			}
		}
		SafeRelease(&pSessionEnumerator);
		return nullptr;
	}

	IAudioEndpointVolume *GetDeviceEndpointVolume(IMMDevice *pDevice)
	{

		IAudioEndpointVolume *pEndpointVolume;
		if (pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&pEndpointVolume) != S_OK)
		{
			return nullptr;
		}

		return pEndpointVolume;
	}

	ISimpleAudioVolume *GetSessionVolume(IAudioSessionControl2 *pSessionControl)
	{

		ISimpleAudioVolume *pAudioVolume;
		if (pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (LPVOID *)&pAudioVolume) != S_OK)
		{
			return nullptr;
		}

		return pAudioVolume;
	}

	std::vector<IAudioSessionControl2 *> GetAudioSessions(IMMDevice *pDevice)
	{

		std::vector<IAudioSessionControl2 *> audioSessions;

		IAudioSessionManager2 *pSessionManager;
		if (pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void **)&pSessionManager) != S_OK)
		{
			return audioSessions;
		}

		IAudioSessionEnumerator *pSessionEnumerator;
		if (pSessionManager->GetSessionEnumerator(&pSessionEnumerator) != S_OK)
		{
			SafeRelease(&pSessionManager);
			return audioSessions;
		}

		IAudioSessionControl *pSessionControl = nullptr;
		IAudioSessionControl2 *pSessionControl2 = nullptr;
		int size = -1;
		pSessionEnumerator->GetCount(&size);
		for (size_t i = 0; i < size; i++)
		{
			if (pSessionEnumerator->GetSession(i, &pSessionControl) == S_OK)
			{
				if (pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID *)&pSessionControl2) == S_OK)
					audioSessions.push_back(pSessionControl2);
				SafeRelease(&pSessionControl);
			}
		}

		SafeRelease(&pSessionEnumerator);
		SafeRelease(&pSessionManager);
		return audioSessions;
	}

} // namespace SoundMixerUtils