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

	SoundMixer::SoundMixer(on_device_changed_cb_t cb) : deviceCallback(cb)
	{
		CoInitialize(NULL);
		HRESULT ok = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pEnumerator);
	}

	SoundMixer::~SoundMixer()
	{
		SafeRelease(&pEnumerator);
		CoUninitialize();
	}

    void SoundMixer::filterDevices() {
        for(auto it = devices.begin(); it != devices.end(); ++it) {
            std::string key = it->first;
            if(devices[key]->Update() == false) {
                delete devices[key];
                devices.erase(key);
            }
        }
    }

	vector<Device *> SoundMixer::GetDevices()
	{
        filterDevices();
		IMMDeviceCollection *pDevices;
		HRESULT ok = pEnumerator->EnumAudioEndpoints(EDataFlow::eAll, DEVICE_STATE_ACTIVE, &pDevices);

		std::vector<Device *> res;
		IMMDevice *dev = NULL;
		UINT count = 0;
		pDevices->GetCount(&count);
		for (UINT i = 0; i < count; i++)
		{
            pDevices->Item(i, &dev);
            LPWSTR winId = NULL;
            dev->GetId(&winId);
            if(winId == NULL)
                continue;
            std::string key = toString(winId);
            if(devices.count(key) <= 0) {
                devices[key] = new Device(dev, deviceCallback);
            }
            res.push_back(devices[key]);
            CoTaskMemFree(winId);
        }
        return res;
    }

    Device *SoundMixer::GetDefaultDevice(DeviceType type)
    {
        IMMDevice *dev;
        if (type == DeviceType::OUTPUT)
            pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &dev);
        else
            pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eCapture, ERole::eConsole, &dev);
        return new Device(dev, deviceCallback);
    }

    Device::Device(IMMDevice *dev, on_device_changed_cb_t cb):
        device(dev),
        endpoint(NULL),
        endpointVolume(NULL),
        _deviceCallback(cb)
    {
        LPWSTR winId;
        device->GetId(&winId);
        desc = DeviceDescriptor{};
        desc.id = toString(winId);

        valid = Update();

        CoTaskMemFree(winId);
    }

    IMMDeviceEnumerator *Device::GetEnumerator() {

        IMMDeviceEnumerator *enumerator = NULL;
        HRESULT result =
            CoCreateInstance(__uuidof(MMDeviceEnumerator),
                    NULL,
                    CLSCTX_ALL,
                    __uuidof(IMMDeviceEnumerator),
                    (LPVOID *)&enumerator
                    );
        if(result != S_OK)
            return NULL;
        return enumerator;
    }

    bool Device::IsValid() {
        return valid;
    }

    bool Device::Update() {
        LPCWSTR id = toLPWSTR(desc.id);
        IMMDeviceEnumerator *enumerator = GetEnumerator();
        if(enumerator == NULL) {
            valid = false;
            return false;
        }

        IMMDevice *dev = NULL;
        HRESULT res = enumerator->GetDevice(id, &dev);
        if(res != S_OK)
            return false;
        if(dev == device)
            return true;
        if(device != NULL)
            SafeRelease(&device);
        device = dev;

        device->QueryInterface(&endpoint);
        IPropertyStore *store;
        device->OpenPropertyStore(STGM_READ, &store);
        PROPVARIANT var;
        PropVariantInit(&var);

        store->GetValue(PKEY_Device_FriendlyName, &var);
        EDataFlow flow;
        endpoint->GetDataFlow(&flow);

        desc.fullName = toString(var.pwszVal);
        desc.type = (DeviceType) flow;

        PropVariantClear(&var);
        SafeRelease(&store);

        if(endpointVolume != NULL)
            SafeRelease(&endpointVolume);

        device->Activate(
                __uuidof(IAudioEndpointVolume),
                CLSCTX_ALL,
                NULL,
                (LPVOID *)&endpointVolume
                );

        // TODO: properly implement callbacks.
        endpointVolume->RegisterControlChangeNotify(
                new SoundMixerAudioEndpointVolumeCallback(this)
                );

        SafeRelease(&enumerator);
        return true;
    }

    Device::~Device()
    {
        SafeRelease(&endpointVolume);
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
        endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
    }

    void Device::SetMute(bool volume)
    {
        endpointVolume->SetMute((BOOL)volume, NULL);
    }

    float Device::GetVolume()
    {
        float volume;
        endpointVolume->GetMasterVolumeLevelScalar(&volume);
        return volume;
    }

    bool Device::GetMute()
    {
        BOOL mute;
        endpointVolume->GetMute(&mute);
        return (bool)mute;
    }

    void Device::SetVolumeBalance(const VolumeBalance &balance)
    {
        UINT count = 0;
        HRESULT res = endpointVolume->GetChannelCount(&count);
        if (res != S_OK)
        {
            return;
        }
        else if (count <= 1)
        {
            return;
        }

        endpointVolume->SetChannelVolumeLevelScalar(RIGHT, balance.right, NULL);
        endpointVolume->SetChannelVolumeLevelScalar(LEFT, balance.left, NULL);

    }

    VolumeBalance Device::GetVolumeBalance()
    {
        VolumeBalance result = {
            0.F,
            0.F,
            false};
        UINT count = 0;
        HRESULT res = endpointVolume->GetChannelCount(&count);
        if (res != S_OK)
        {
            return result;
        }
        else if (count <= 1)
        {
            return result;
        }
        result.stereo = true;
        endpointVolume->GetChannelVolumeLevelScalar(RIGHT, &result.right);
        endpointVolume->GetChannelVolumeLevelScalar(LEFT, &result.left);

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


    IFACEMETHODIMP SoundMixerAudioEndpointVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
        if(device->_deviceCallback == NULL)
            return S_OK;
        device->_deviceCallback(device->Desc(), 0, pNotify);
        return S_OK;
    }

    SoundMixerAudioEndpointVolumeCallback::SoundMixerAudioEndpointVolumeCallback(Device *dev): device(dev) {}
    IFACEMETHODIMP SoundMixerAudioEndpointVolumeCallback::QueryInterface(const IID& iid, void **ppUnk) {
        return S_OK;
    }

    IFACEMETHODIMP_(ULONG) SoundMixerAudioEndpointVolumeCallback::AddRef() {
        return 0;
    }
    IFACEMETHODIMP_(ULONG) SoundMixerAudioEndpointVolumeCallback::Release() {
        return 0;
    }

} // namespace SoundMixerUtils
