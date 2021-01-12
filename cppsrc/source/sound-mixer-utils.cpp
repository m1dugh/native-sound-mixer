#include "../headers/sound-mixer-utils.h"
#include <iostream>
#include <Functiondiscoverykeys_devpkey.h>

namespace SoundMixerUtils {
	IMMDevice* GetDeviceById(LPWSTR id)
	{

		IMMDeviceEnumerator* enumerator = nullptr;
		HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&enumerator);

		IMMDevice* pDevice = nullptr;
		result = enumerator->GetDevice(id, &pDevice);

		return pDevice;
	}

	std::vector<DeviceDescriptor> getDevices() {
		HRESULT res = CoInitialize(NULL);

		IMMDeviceEnumerator* pDeviceEnumerator;
		res = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pDeviceEnumerator);
		//CHECK_RES(res, "create instance");

		IMMDeviceCollection* pDevices;
		res = pDeviceEnumerator->EnumAudioEndpoints(EDataFlow::eAll, DEVICE_STATE_ACTIVE, &pDevices);
		//CHECK_RES(res, "enum audio endpoints");

		UINT deviceCount;
		res = pDevices->GetCount(&deviceCount);
		//CHECK_RES(res, "get device count");

		IMMDevice* pDevice;
		IMMEndpoint* pEndpoint;

		std::vector<DeviceDescriptor> descriptors(deviceCount);
		for (UINT i = 0; i < deviceCount; i++) {

			DeviceDescriptor descriptor = {};
			HRESULT deviceRes = pDevices->Item(i, &pDevice);
			LPWSTR deviceId;
			if (deviceRes == S_OK && pDevice->GetId(&deviceId) == S_OK) {
				descriptor.id = deviceId;
				IPropertyStore* pProperties;
				if (pDevice->OpenPropertyStore(STGM_READ, &pProperties) == S_OK) {
					PROPVARIANT prop;
					if (pProperties->GetValue(PKEY_DeviceInterface_FriendlyName, &prop) == S_OK
						) {
						descriptor.friendlyName = prop.pwszVal;
					}
					else {
						std::cerr << "failed getting friendly name" << std::endl;
					}

					if (pProperties->GetValue(PKEY_Device_DeviceDesc, &prop) == S_OK) {
						descriptor.description = prop.pwszVal;
					}
					else {
						std::cerr << "failed getting device desc" << std::endl;
					}
				}
				else {
					std::cerr << "failed opening property store " << std::endl;
				}


				if (pDevice->QueryInterface(__uuidof(IMMEndpoint), (LPVOID*)&pEndpoint) == S_OK) {
					EDataFlow flow;
					if (pEndpoint->GetDataFlow(&flow) == S_OK) {
						descriptor.dataFlow = flow;
					}
					else {
						//std::cerr << "failed retrieving data flow for device " << to_string(deviceId) << std::endl;
					}
				}
				else {
					//std::cerr << "failed retrieving IMMEndpoint for device " << to_string(deviceId) << std::endl;
				}

				descriptors.push_back(descriptor);
			}
			else {
				std::cerr << "failed geting item " << i << std::endl;
			}
		}



		CoUninitialize();
		return descriptors;
	}


	IAudioSessionControl2* GetAudioSessionByProcessId(IMMDevice* pDevice, DWORD processId)
	{

		IAudioSessionManager2* pSessionManager = nullptr;
		HRESULT result = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID*)&pSessionManager);
		//CHECK_RES(result)

			IAudioSessionEnumerator* pSessionEnumerator = nullptr;
		result = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		//CHECK_RES(result)

			IAudioSessionControl* pSessionControl = nullptr;
		IAudioSessionControl2* pSessionControl2 = nullptr;
		int size = -1;
		pSessionEnumerator->GetCount(&size);

		for (size_t i = 0; i < size; i++)
		{
			result = pSessionEnumerator->GetSession(i, (IAudioSessionControl**)&pSessionControl);
			if (result == S_OK)
			{

				result = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID*)&pSessionControl2);
				DWORD id = 0;
				result = pSessionControl2->GetProcessId(&id);
				if (result == S_OK && id == processId)
				{
					return pSessionControl2;
				}
			}
		}

		return nullptr;
	}

	IAudioEndpointVolume* GetDeviceEndpointVolume(IMMDevice* pDevice)
	{

		IAudioEndpointVolume* pEndpointVolume = nullptr;
		HRESULT result = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&pEndpointVolume);
		//CHECK_RES(result)

			return pEndpointVolume;
	}

	ISimpleAudioVolume* GetSessionVolume(IAudioSessionControl2* pSessionControl)
	{

		ISimpleAudioVolume* pAudioVolume = nullptr;
		HRESULT result = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (LPVOID*)&pAudioVolume);
		//CHECK_RES(result)

			return pAudioVolume;
	}

	std::vector<IAudioSessionControl2*> GetAudioSessions(IMMDevice* pDevice)
	{

		CoInitialize(NULL);

		std::vector<IAudioSessionControl2*> audioSessions = {};

		IAudioSessionManager2* pSessionManager = NULL;
		HRESULT result = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
		if (result != S_OK)
		{
			return {};
		}

		IAudioSessionEnumerator* pSessionEnumerator = NULL;
		result = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		if (result != S_OK)
		{
			return {};
		}
		pSessionManager->Release();

		IAudioSessionControl* pSessionControl = nullptr;
		IAudioSessionControl2* pSessionControl2 = nullptr;
		int size = -1;
		pSessionEnumerator->GetCount(&size);
		AudioSessionState state;
		for (size_t i = 0; i < size; i++)
		{

			result = pSessionEnumerator->GetSession(i, &pSessionControl);
			if (result == S_OK)
			{
				result = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID*)&pSessionControl2);
				pSessionControl->GetState(&state);

				if (result == S_OK && state == AudioSessionState::AudioSessionStateActive)
					audioSessions.push_back(pSessionControl2);
			}
		}

		pSessionEnumerator->Release();
		CoUninitialize();
		return audioSessions;
	}

}