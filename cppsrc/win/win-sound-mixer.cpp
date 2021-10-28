#include "./headers/win-sound-mixer.hpp"
#include <Functiondiscoverykeys_devpkey.h>
#include <WinUser.h>
#include <string>
#include <wchar.h>

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
	}

	SoundMixer::~SoundMixer()
	{
		SafeRelease(&pEnumerator);
		CoUninitialize();
	}

	vector<Device *> SoundMixer::GetDevices()
	{
		CoInitialize(NULL);
		IMMDeviceCollection *pDevices;
		HRESULT ok = pEnumerator->EnumAudioEndpoints(EDataFlow::eAll, DEVICE_STATE_ACTIVE, &pDevices);

		std::vector<Device *> devices;
		IMMDevice *dev;
		UINT count = 0;
		pDevices->GetCount(&count);
		for (UINT i = 0; i < count; i++)
		{
			pDevices->Item(i, &dev);
			devices.push_back(new Device(dev));
		}
		return devices;
	}

	Device *SoundMixer::GetDefaultDevice(DeviceType type)
	{
		IMMDevice *dev;
		if (type == DeviceType::OUTPUT)
			pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &dev);
		else
			pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eCapture, ERole::eConsole, &dev);
		return new Device(dev);
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
	}

	Device::~Device()
	{
		SafeRelease(&endpoint);
		SafeRelease(&device);
	}

	vector<AudioSession *> Device::GetAudioSessions()
	{

		IAudioSessionManager2 *manager;
		device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID *)&manager);

		IAudioSessionEnumerator *pEnumerator;
		manager->GetSessionEnumerator(&pEnumerator);
		SafeRelease(&manager);

		std::vector<AudioSession *> sessions;

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

		return sessions;
	}

	IAudioEndpointVolume *Device::getAudioEndpointVolume()
	{
		IAudioEndpointVolume *volume;
		device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&volume);
		return volume;
	}

	void Device::SetVolume(float volume)
	{
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < .0F)
		{
			volume = .0F;
		}
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

	void Device::SetVolumeBalance(const VolumeBalance &balance)
	{
		IAudioEndpointVolume *pVolume = getAudioEndpointVolume();
		UINT count = 0;
		HRESULT res = pVolume->GetChannelCount(&count);
		if (res != S_OK)
		{
			return;
		}
		else if (count <= 1)
		{
			return;
		}

		pVolume->SetChannelVolumeLevelScalar(RIGHT, balance.right, NULL);
		pVolume->SetChannelVolumeLevelScalar(LEFT, balance.left, NULL);

		SafeRelease(&pVolume);
	}

	VolumeBalance Device::GetVolumeBalance()
	{
		IAudioEndpointVolume *pVolume = getAudioEndpointVolume();
		VolumeBalance result = {
			0.F,
			0.F,
			false};
		UINT count = 0;
		HRESULT res = pVolume->GetChannelCount(&count);
		if (res != S_OK)
		{
			return result;
		}
		else if (count <= 1)
		{
			return result;
		}
		result.stereo = true;
		pVolume->GetChannelVolumeLevelScalar(RIGHT, &result.right);
		pVolume->GetChannelVolumeLevelScalar(LEFT, &result.left);
		SafeRelease(&pVolume);

		return result;
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

	AudioSessionState AudioSession::state()
	{
		AudioSessionState state;
		control->GetState(&state);
		return state;
	}

	std::string AudioSession::id()
	{

		LPWSTR guid;
		std::string result;
		if (control->GetSessionInstanceIdentifier(&guid) == S_OK)
		{
			result = toString(guid);
			CoTaskMemFree(guid);
		}

		return result;
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

	void AudioSession::SetVolumeBalance(const VolumeBalance &balance)
	{
		IChannelAudioVolume *channelVolume;
		control->QueryInterface(__uuidof(IChannelAudioVolume), (LPVOID *)&channelVolume);
		UINT count = 0;
		HRESULT res = channelVolume->GetChannelCount(&count);
		if (res != S_OK)
		{
			return;
		}
		else if (count <= 1)
		{
			return;
		}

		channelVolume->SetChannelVolume(RIGHT, balance.right, NULL);
		channelVolume->SetChannelVolume(LEFT, balance.left, NULL);

		SafeRelease(&channelVolume);
	}

	VolumeBalance AudioSession::GetVolumeBalance()
	{

		IChannelAudioVolume *channelVolume;
		control->QueryInterface(__uuidof(IChannelAudioVolume), (LPVOID *)&channelVolume);
		VolumeBalance result = {
			0.F,
			0.F,
			false};
		UINT count = 0;
		HRESULT res = channelVolume->GetChannelCount(&count);
		if (res != S_OK)
		{
			return result;
		}
		else if (count <= 1)
		{
			return result;
		}
		result.stereo = true;
		channelVolume->GetChannelVolume(RIGHT, &result.right);
		channelVolume->GetChannelVolume(LEFT, &result.left);
		SafeRelease(&channelVolume);

		return result;
	}

	void AudioSession::SetVolume(float vol)
	{
		if (vol > 1.F)
		{
			vol = 1.F;
		}
		else if (vol < .0F)
		{
			vol = .0F;
		}
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