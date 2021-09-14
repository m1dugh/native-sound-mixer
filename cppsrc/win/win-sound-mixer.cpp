#include "./headers/win-sound-mixer.hpp"
#include <Functiondiscoverykeys_devpkey.h>
#include <WinUser.h>
#include <string>
#include <wchar.h>

#include <iostream>
#include <sstream>

std::string
GetProcNameFromId(DWORD id)
{
	if (id == 0)
	{
		return std::to_string(id);
	}
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, id);
	LPWSTR processName = (LPWSTR)CoTaskMemAlloc(256);
	DWORD size = 256;
	QueryFullProcessImageNameW(handle, 0, processName, &size);

	std::wstring name = std::wstring(processName);
	CoTaskMemFree(processName);
	return std::string(name.begin(), name.end());
}

inline LPWSTR toLPWSTR(std::string str)
{

	size_t size = str.length() + 1;

	LPWSTR chars = (LPWSTR)CoTaskMemAlloc(size * sizeof(wchar_t));
	std::mbstowcs(chars, str.c_str(), size);
	return (LPWSTR)chars;
}

inline std::string toString(LPWSTR str)
{
	std::wstring wstr(str);

	return std::string(wstr.begin(), wstr.end());
}

HWND GetWindowHandle(DWORD id)
{

	HWND prevWindow = 0;

	for (;;)
	{
		HWND desktopWindow = GetDesktopWindow();
		if (!desktopWindow)
			break;

		HWND nextWindow = FindWindowEx(desktopWindow, prevWindow, NULL, NULL);

		if (!nextWindow)
			break;
		DWORD windowId;
		GetWindowThreadProcessId(nextWindow, &windowId);

		if (windowId == id && IsWindowVisible(nextWindow) && !GetParent(nextWindow) && GetWindowTextLength(nextWindow))
		{
			return nextWindow;
		}

		prevWindow = nextWindow;
	}

	return 0;
}

std::string GetWindowNameFromProcId(DWORD id)
{
	HWND handle = GetWindowHandle(id);
	if (handle == 0)
	{
		return std::string();
	}
	// adding +1 for null character
	size_t length = GetWindowTextLengthW(handle) + 1;
	LPWSTR name = (LPWSTR)CoTaskMemAlloc(length * sizeof(WCHAR));
	GetWindowTextW(handle, name, length);

	std::string result = toString(name);
	CoTaskMemFree(name);
	return result;
}

using std::vector;

namespace WinSoundMixer
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

	SoundMixer::SoundMixer()
	{
		CoInitialize(NULL);
		HRESULT ok = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pEnumerator);
		Reload();
	}

	SoundMixer::~SoundMixer()
	{
		for (auto dev : devices)
		{
			delete dev;
		}
		SafeRelease(&pEnumerator);
		CoUninitialize();
	}

	void SoundMixer::Reload()
	{

		IMMDeviceCollection *pDevices;
		HRESULT ok = pEnumerator->EnumAudioEndpoints(EDataFlow::eAll, DEVICE_STATE_ACTIVE, &pDevices);

		if (defaultOutputDevice != nullptr)
		{
			delete defaultOutputDevice;
		}
		if (defaultInputDevice != nullptr)
		{
			delete defaultInputDevice;
		}

		IMMDevice *dev;
		ok = pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eCapture, ERole::eConsole, &dev);
		defaultInputDevice = new Device(dev);
		ok = pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &dev);
		defaultOutputDevice = new Device(dev);

		devices.clear();

		UINT count = 0;
		pDevices->GetCount(&count);
		for (UINT i = 0; i < count; i++)
		{
			pDevices->Item(i, &dev);
			devices.push_back(new Device(dev));
		}
	}

	vector<Device *> SoundMixer::GetDevices()
	{
		return devices;
	}

	Device *SoundMixer::GetDefaultDevice(DeviceType type)
	{
		if (type == DeviceType::OUTPUT)
			return defaultOutputDevice;
		else
			return defaultInputDevice;
	}

	Device *SoundMixer::GetDeviceById(std::string id)
	{
		for (auto dev : devices)
		{
			if (id == dev->Desc().id)
			{
				return dev;
			}
		}

		return nullptr;
	}

	Device::Device(IMMDevice *dev) : device(dev)
	{
		device->QueryInterface(&endpoint);
		IPropertyStore *store;
		device->OpenPropertyStore(STGM_READ, &store);
		LPWSTR winId;
		PROPVARIANT var;
		PropVariantInit(&var);
		device->GetId(&winId);
		store->GetValue(PKEY_Device_FriendlyName, &var);
		EDataFlow flow;
		endpoint->GetDataFlow(&flow);

		desc = DeviceDescriptor{
			toString(var.pwszVal),
			toString(winId),
			(DeviceType)flow};

		PropVariantClear(&var);
		SafeRelease(&store);
		CoTaskMemFree(winId);
		Reload();
	}

	void Device::Reload()
	{
		IAudioSessionManager2 *manager;
		device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID *)&manager);

		IAudioSessionEnumerator *pEnumerator;
		manager->GetSessionEnumerator(&pEnumerator);

		sessions.clear();

		int count;
		pEnumerator->GetCount(&count);
		IAudioSessionControl *control;
		IAudioSessionControl2 *control2;
		for (int i = 0; i < count; i++)
		{
			if (pEnumerator->GetSession(i, &control) == S_OK &&
				control->QueryInterface(&control2) == S_OK)
			{
				sessions.push_back(new AudioSession(control2));
			}
			SafeRelease(&control);
		}
		SafeRelease(&pEnumerator);
		SafeRelease(&manager);
	}

	Device::~Device()
	{
		for (auto s : sessions)
		{
			delete s;
		}
		SafeRelease(&endpoint);
		SafeRelease(&device);
	}

	vector<AudioSession *> Device::GetAudioSessions()
	{
		return sessions;
	}

	AudioSession *Device::GetAudioSessionById(std::string id)
	{
		for (auto s : sessions)
		{
			if (s->id() == id)
				return s;
		}
		return nullptr;
	}

	IAudioEndpointVolume *Device::getAudioEndpointVolume()
	{
		IAudioEndpointVolume *volume;
		device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&volume);
		return volume;
	}

	void Device::SetVolume(float volume)
	{
		IAudioEndpointVolume *endpointVol = getAudioEndpointVolume();
		endpointVol->SetMasterVolumeLevelScalar(volume, NULL);
		SafeRelease(&endpointVol);
	}

	void Device::SetMute(bool volume)
	{
		IAudioEndpointVolume *endpointVol = getAudioEndpointVolume();
		endpointVol->SetMute((BOOL)volume, NULL);
		SafeRelease(&endpointVol);
	}

	float Device::GetVolume()
	{
		IAudioEndpointVolume *endpointVol = getAudioEndpointVolume();
		float volume;
		endpointVol->GetMasterVolumeLevelScalar(&volume);
		SafeRelease(&endpointVol);
		return volume;
	}

	bool Device::GetMute()
	{
		IAudioEndpointVolume *endpointVol = getAudioEndpointVolume();
		BOOL mute;
		endpointVol->GetMute(&mute);
		SafeRelease(&endpointVol);
		return (bool)mute;
	}

	AudioSession::AudioSession(IAudioSessionControl2 *control) : control(control)
	{
	}

	AudioSession::~AudioSession()
	{
		SafeRelease(&control);
	}

	std::string AudioSession::name()
	{
		DWORD pId;
		std::string result;
		if (control->GetProcessId(&pId) == S_OK)
		{
			if ((result = GetWindowNameFromProcId(pId)).size() <= 0)
			{

				std::stringstream stream(GetProcNameFromId(pId));
				while (getline(stream, result, '\\'))
					;
				getline(std::stringstream(result), result, '.');
			}
		}
		return result;
	}

	AudioSessionState AudioSession::state() {
		AudioSessionState state;
		control->GetState(&state);
		return state;
	}

	std::string AudioSession::id()
	{

		LPWSTR guid;
		std::string result;
		if (control->GetSessionIdentifier(&guid) == S_OK)
		{
			result = toString(guid);
			CoTaskMemFree(guid);
		}

		return result + name();
	}

	std::string AudioSession::path()
	{
		DWORD pId;
		if (control->GetProcessId(&pId) == S_OK)
		{
			return GetProcNameFromId(pId);
		}
		return "";
	}

	ISimpleAudioVolume *AudioSession::getAudioVolume()
	{
		ISimpleAudioVolume *volume;
		control->QueryInterface(__uuidof(ISimpleAudioVolume), (LPVOID *)&volume);
		return volume;
	}

	void AudioSession::SetVolume(float vol)
	{
		ISimpleAudioVolume *volume = getAudioVolume();
		volume->SetMasterVolume(vol, NULL);
		SafeRelease(&volume);
	}

	void AudioSession::SetMute(bool mute)
	{
		ISimpleAudioVolume *volume = getAudioVolume();
		volume->SetMute(mute, NULL);
		SafeRelease(&volume);
	}

	bool AudioSession::GetMute()
	{
		ISimpleAudioVolume *volume = getAudioVolume();
		BOOL mute;
		volume->GetMute(&mute);
		SafeRelease(&volume);
		return (bool)mute;
	}

	float AudioSession::GetVolume()
	{
		ISimpleAudioVolume *volume = getAudioVolume();
		float vol;
		volume->GetMasterVolume(&vol);
		SafeRelease(&volume);
		return vol;
	}

} // namespace SoundMixerUtils