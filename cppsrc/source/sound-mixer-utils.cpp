#include "../headers/sound-mixer-utils.h"
#include <iostream>
#include <Functiondiscoverykeys_devpkey.h>
#include <string>
#include <wchar.h>

#define CHECK_RES(res, location) if(res != S_OK){throw SoundMixerUtils::SoundMixerException(std::string("failed at ") + std::string(location));}
#define SAFE_RELEASE(ptr) if(ptr != NULL){ptr->Release();ptr=NULL;}

inline std::string toString(LPWSTR str) {
	std::wstring wstr(str);

	return std::string(wstr.begin(), wstr.end());
}

namespace SoundMixerUtils {


	IMMDevice* GetDeviceById(LPWSTR id)
	{
		CoInitialize(NULL);

		IMMDeviceEnumerator* pDeviceEnumerator = nullptr;
		HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pDeviceEnumerator);
		CHECK_RES(result, "create device enumerator");

		IMMDevice* pDevice = nullptr;
		result = pDeviceEnumerator->GetDevice(id, &pDevice);
		if (result == E_NOTFOUND)
			throw SoundMixerException("invalid device id provided");
		else if (result != S_OK && result != E_NOTFOUND)
			CHECK_RES(result, "get device with id");

		SAFE_RELEASE(pDeviceEnumerator);


		CoUninitialize();

		return pDevice;
	}

	DeviceDescriptor GetDevice(EDataFlow dataFlow) {
		HRESULT res = CoInitialize(NULL);

		IMMDeviceEnumerator* pDeviceEnumerator;
		res = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pDeviceEnumerator);
		CHECK_RES(res, "create device enumerator instance");

		IMMDevice* pDevice;


		res = pDeviceEnumerator->GetDefaultAudioEndpoint(dataFlow, ERole::eConsole, &pDevice);
		CHECK_RES(res, "create device enumerator instance");

		LPWSTR id;
		DeviceDescriptor descriptor;
		res = pDevice->GetId(&id);
		CHECK_RES(res, "get device id");

		descriptor.id = toString(id);

		IPropertyStore* pProperties;
		if (pDevice->OpenPropertyStore(STGM_READ, &pProperties) == S_OK) {
			std::string fullName;
			PROPVARIANT prop;
			if (pProperties->GetValue(PKEY_DeviceInterface_FriendlyName, &prop) == S_OK
				) {
				fullName = toString(prop.pwszVal);
			}
			else {
				std::cerr << "failed getting friendly name for device " << toString(id) << std::endl;
			}

			if (pProperties->GetValue(PKEY_Device_DeviceDesc, &prop) == S_OK) {
				fullName += (char)0x20 + toString(prop.pwszVal);
			}
			else {
				std::cerr << "failed getting device desc for device " << toString(id) << std::endl;
			}

			descriptor.fullName = fullName;
		}
		else {
			std::cerr << "failed opening property store for device " << toString(id) << std::endl;
		}
		descriptor.dataFlow = dataFlow;

		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceEnumerator);

		CoUninitialize();


		return descriptor;
	}

	std::vector<DeviceDescriptor> GetDevices() {

		HRESULT res = CoInitialize(NULL);

		IMMDeviceEnumerator* pDeviceEnumerator;
		res = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pDeviceEnumerator);
		CHECK_RES(res, "create device enumerator instance");

		IMMDeviceCollection* pDevices;
		res = pDeviceEnumerator->EnumAudioEndpoints(EDataFlow::eAll, DEVICE_STATE_ACTIVE, &pDevices);
		CHECK_RES(res, "enum audio endpoints");

		UINT deviceCount;
		res = pDevices->GetCount(&deviceCount);
		CHECK_RES(res, "get device count");

		IMMDevice* pDevice;
		IMMEndpoint* pEndpoint;

		std::vector<DeviceDescriptor> descriptors;
		for (UINT i = 0; i < deviceCount; i++) {

			HRESULT deviceRes = pDevices->Item(i, &pDevice);
			LPWSTR id;
			if (deviceRes == S_OK && pDevice->GetId(&id) == S_OK) {
				DeviceDescriptor desc;

				desc.id = toString(id);

				IPropertyStore* pProperties;
				if (pDevice->OpenPropertyStore(STGM_READ, &pProperties) == S_OK) {
					std::string fullName;
					PROPVARIANT prop;
					if (pProperties->GetValue(PKEY_DeviceInterface_FriendlyName, &prop) == S_OK
						) {
						fullName = toString(prop.pwszVal);
					}
					else {
						std::cerr << "failed getting friendly name for device " << toString(id) << std::endl;
					}

					if (pProperties->GetValue(PKEY_Device_DeviceDesc, &prop) == S_OK) {
						fullName += (char)0x20 + toString(prop.pwszVal);
					}
					else {
						std::cerr << "failed getting device desc for device " << toString(id) << std::endl;
					}

					desc.fullName = fullName;
				}
				else {
					std::cerr << "failed opening property store for device " << toString(id) << std::endl;
				}

				if (pDevice->QueryInterface(__uuidof(IMMEndpoint), (LPVOID*)&pEndpoint) == S_OK) {
					EDataFlow flow;
					if (pEndpoint->GetDataFlow(&flow) == S_OK) {
						desc.dataFlow = flow;
					}
					else {
						std::cerr << "failed retrieving data flow for device " << toString(id) << std::endl;
					}
				}
				else {
					std::cerr << "failed retrieving IMMEndpoint for device " << toString(id) << std::endl;
				}

				descriptors.push_back(desc);

			}
			else {
				std::cerr << "failed geting item " << i << std::endl;
			}
		}

		SAFE_RELEASE(pDevices);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pEndpoint);
		SAFE_RELEASE(pDeviceEnumerator);

		CoUninitialize();


		return descriptors;
	}


	IAudioSessionControl2* GetAudioSessionByProcessId(IMMDevice* pDevice, DWORD processId)
	{

		IAudioSessionManager2* pSessionManager = nullptr;
		HRESULT result = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID*)&pSessionManager);
		CHECK_RES(result, "activate device when fetching IAudioSessionManager2");

		IAudioSessionEnumerator* pSessionEnumerator = nullptr;
		result = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		CHECK_RES(result, "getting audio sessions enumerator");

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
					SAFE_RELEASE(pSessionEnumerator);
					SAFE_RELEASE(pSessionManager);
					return pSessionControl2;
				}
			}
		}
		SAFE_RELEASE(pSessionEnumerator);
		SAFE_RELEASE(pSessionManager);
		throw SoundMixerException(std::string("could not found session id for process id ") + std::to_string(processId));
	}

	IAudioEndpointVolume* GetDeviceEndpointVolume(IMMDevice* pDevice)
	{

		IAudioEndpointVolume* pEndpointVolume = nullptr;
		HRESULT result = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&pEndpointVolume);
		CHECK_RES(result, "fetching IAudioEndpointVolume");

		return pEndpointVolume;
	}

	ISimpleAudioVolume* GetSessionVolume(IAudioSessionControl2* pSessionControl)
	{

		ISimpleAudioVolume* pAudioVolume = nullptr;
		HRESULT result = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (LPVOID*)&pAudioVolume);
		CHECK_RES(result, "getting ISimpleAudioVolume");

		return pAudioVolume;
	}

	std::vector<IAudioSessionControl2*> GetAudioSessions(IMMDevice* pDevice)
	{

		CoInitialize(NULL);

		std::vector<IAudioSessionControl2*> audioSessions = {};

		IAudioSessionManager2* pSessionManager = NULL;
		HRESULT result = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
		CHECK_RES(result, "getting IAudioSessionManager2");

		IAudioSessionEnumerator* pSessionEnumerator = NULL;
		result = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		CHECK_RES(result, "getting IAudioSessionEnumerator");

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

		SAFE_RELEASE(pSessionEnumerator);
		SAFE_RELEASE(pSessionManager);
		CoUninitialize();
		return audioSessions;
	}

}