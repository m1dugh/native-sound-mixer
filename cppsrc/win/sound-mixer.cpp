#include "../headers/sound-mixer.hpp"
#include "./headers/sound-mixer-utils.hpp"

#include <iostream>

#define CHECK_RES(res)  \
	if (res != S_OK)    \
	{                   \
		return nullptr; \
	}

#define CHECK_PTR(env, ptr)                                                                   \
	if (ptr == nullptr)                                                                       \
	{                                                                                         \
		Napi::Error::New(env, "an error occured, null pointer").ThrowAsJavaScriptException(); \
	}

std::string GetProcNameFromId(DWORD id)
{
	if (id == 0)
	{
		return std::to_string(id);
	}
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, id);
	LPWSTR processName = (LPWSTR)malloc(256);
	DWORD size = 256;
	QueryFullProcessImageNameW(handle, 0, processName, &size);

	std::wstring name = std::wstring(processName);
	return std::string(name.begin(), name.end());
}

inline LPWSTR toLPWSTR(std::string str)
{

	size_t size = str.length() + 1;

	LPWSTR chars = (LPWSTR)malloc(size * sizeof(wchar_t));
	std::mbstowcs(chars, str.c_str(), size);
	return (LPWSTR)chars;
}

inline std::string toString(LPWSTR str)
{
	std::wstring wstr(str);

	return std::string(wstr.begin(), wstr.end());
}

inline Napi::Object &deviceToObject(SoundMixerUtils::DeviceDescriptor const &desc, Napi::Object &obj)
{
	obj.Set("id", desc.id);
	obj.Set("name", desc.fullName);
	obj.Set("type", (int)desc.dataFlow);

	return obj;
}

using namespace SoundMixerUtils;

namespace SoundMixer
{
	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		exports.Set("GetSessions", Napi::Function::New(env, GetAudioSessionNames));
		exports.Set("GetDevices", Napi::Function::New(env, GetEndpoints));
		exports.Set("GetDefaultDevice", Napi::Function::New(env, GetDefaultEndpoint));

		exports.Set("SetEndpointVolume", Napi::Function::New(env, SetEndpointVolume));
		exports.Set("GetEndpointVolume", Napi::Function::New(env, GetEndpointVolume));
		exports.Set("SetEndpointMute", Napi::Function::New(env, SetEndpointMute));
		exports.Set("GetEndpointMute", Napi::Function::New(env, GetEndpointMute));

		exports.Set("SetAudioSessionVolume", Napi::Function::New(env, SetAudioSessionVolume));
		exports.Set("GetAudioSessionVolume", Napi::Function::New(env, GetAudioSessionVolume));
		exports.Set("SetAudioSessionMute", Napi::Function::New(env, SetAudioSessionMute));
		exports.Set("GetAudioSessionMute", Napi::Function::New(env, GetAudioSessionMute));

		return exports;
	}

	Napi::Array GetAudioSessionNames(Napi::CallbackInfo const &info)
	{

		Napi::Env env = info.Env();

		if (info.Length() != 1 || !info[0].IsString())
		{
			std ::cout << "throwing error " << std::endl;
			Napi::TypeError::New(env, "expected string as device id as only parameter").ThrowAsJavaScriptException();
			return Napi::Array::New(env);
		}

		CoInitialize(NULL);
		std::string id = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(id));

		CHECK_PTR(env, pDevice);
		std::vector<IAudioSessionControl2 *> sessions = GetAudioSessions(pDevice);
		Napi::Array sessionNames = Napi::Array::New(env, sessions.size());
		int i = 0;
		DWORD procId = 0;
		LPWSTR guid;
		AudioSessionState state;
		for (IAudioSessionControl2 *session : sessions)
		{
			if (session->GetProcessId(&procId) == S_OK && session->GetSessionIdentifier(&guid) == S_OK && session->GetState(&state) == S_OK)
			{
				Napi::Object val = Napi::Object::New(env);
				val.Set("id", toString(guid));
				val.Set("path", GetProcNameFromId(procId));
				val.Set("state", (int)state);
				sessionNames.Set(i++, val);
			}
		}

		CoUninitialize();
		return sessionNames;
	}

	Napi::Array GetEndpoints(Napi::CallbackInfo const &info)
	{

		Napi::Env env = info.Env();

		std::vector<DeviceDescriptor> devices = GetDevices();
		Napi::Array result = Napi::Array::New(env, devices.size());

		int i = 0;
		for (DeviceDescriptor const &desc : devices)
		{
			Napi::Object obj = Napi::Object::New(env);
			result.Set(i++, deviceToObject(desc, obj));
		}
		return result;
	}

	Napi::Object GetDefaultEndpoint(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		Napi::Object obj = Napi::Object::New(env);
		if (info.Length() != 1 || !info[0].IsNumber())
		{
			Napi::Error::New(env, "wrong argument passed, expected number").ThrowAsJavaScriptException();
			return obj;
		}

		int val = info[0].As<Napi::Number>().Int32Value();
		if (val < 0 || val >= EDataFlow::EDataFlow_enum_count)
		{
			Napi::Error::New(env, "illegal argument passed, expected number ranged from 0 to 3").ThrowAsJavaScriptException();
			return obj;
		}

		EDataFlow dataFlow = (EDataFlow)val;
		try
		{
			DeviceDescriptor desc = GetDevice(dataFlow);
			deviceToObject(desc, obj);
		}
		catch (SoundMixerException const &e)
		{
			Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
			return obj;
		}

		return obj;
	}

	Napi::Number SetEndpointVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 2 || !info[1].IsNumber() || !info[0].IsString())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, -1);
		}

		CoInitialize(NULL);

		float volume = info[1].As<Napi::Number>().FloatValue();
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < 0.F)
		{
			volume = 0.F;
		}

		std::string id = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(id));
		CHECK_PTR(env, pDevice);

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume);

		HRESULT res = pEndpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when setting the volume level").ThrowAsJavaScriptException();
		}

		CoUninitialize();

		return Napi::Number::New(env, volume);
	}

	Napi::Number GetEndpointVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 1 || !info[0].IsString())
		{
			Napi::TypeError::New(env, "expected string as device id").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0.F);
		}

		CoInitialize(NULL);

		std::string id = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(id));
		CHECK_PTR(env, pDevice);

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume);

		float levelScalar = 0.F;
		HRESULT res = pEndpointVolume->GetMasterVolumeLevelScalar(&levelScalar);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when getting the volume level").ThrowAsJavaScriptException();
		}

		CoUninitialize();
		return Napi::Number::New(env, levelScalar);
	}

	Napi::Boolean SetEndpointMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 2 || !info[1].IsBoolean() || !info[0].IsString())
		{
			Napi::TypeError::New(env, "expected {deviceId: string},{mute: boolean} as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);

		std::string id = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(id));
		CHECK_PTR(env, pDevice);

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume);

		bool mute = info[1].As<Napi::Boolean>().Value();

		HRESULT res = pEndpointVolume->SetMute(mute, NULL);
		if (res != S_OK && res != S_FALSE)
		{
			Napi::Error::New(env, "an error occured when setting mute").ThrowAsJavaScriptException();
		}

		CoUninitialize();

		return Napi::Boolean::New(env, mute);
	}

	Napi::Boolean GetEndpointMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 1 || !info[0].IsString())
		{
			Napi::TypeError::New(env, "device id as only argument").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);

		std::string id = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(id));
		CHECK_PTR(env, pDevice);

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume);

		bool mute = false;
		HRESULT res = pEndpointVolume->GetMute(&(BOOL)mute);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when getting the volume level").ThrowAsJavaScriptException();
		}

		CoUninitialize();
		return Napi::Boolean::New(env, mute);
	}

	Napi::Number SetAudioSessionVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsNumber())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, -1);
		}

		CoInitialize(NULL);

		std::string id = info[1].As<Napi::String>().Utf8Value();

		float volume = info[2].As<Napi::Number>().FloatValue();
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < 0.F)
		{
			volume = 0.F;
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(deviceId));
		CHECK_PTR(env, pDevice);

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByGUID(pDevice, toLPWSTR(id));
		CHECK_PTR(env, pSessionControl);

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume);

		HRESULT res = pSessionVolume->SetMasterVolume(volume, NULL);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when setting the volume level for audioSession").ThrowAsJavaScriptException();
		}

		CoUninitialize();

		return Napi::Number::New(env, volume);
	}

	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 2 || !info[1].IsString() || !info[0].IsString())
		{
			Napi::TypeError::New(env, "expected {<deviceId>: string}, {<audio session id>: string} as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0.F);
		}

		CoInitialize(NULL);

		std::string id = info[1].As<Napi::String>().Utf8Value();

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(deviceId));
		CHECK_PTR(env, pDevice);

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByGUID(pDevice, toLPWSTR(id));
		CHECK_PTR(env, pSessionControl);

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume);

		float volume = 0.F;
		HRESULT res = pSessionVolume->GetMasterVolume(&volume);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when getting the volume level for audioSession").ThrowAsJavaScriptException();
			return Napi::Number::New(env, -1.F);
		}

		CoUninitialize();

		return Napi::Number::New(env, volume);
	}

	Napi::Boolean SetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsBoolean())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);

		std::string id = info[1].As<Napi::String>().Utf8Value();

		bool mute = info[2].As<Napi::Boolean>().Value();

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(deviceId));
		CHECK_PTR(env, pDevice);

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByGUID(pDevice, toLPWSTR(id));
		CHECK_PTR(env, pSessionControl);

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume);

		HRESULT res = pSessionVolume->SetMute(mute, NULL);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when muting audioSession").ThrowAsJavaScriptException();
		}

		CoUninitialize();

		return Napi::Boolean::New(env, mute);
	}

	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);

		std::string id = info[1].As<Napi::String>().Utf8Value();

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		IMMDevice *pDevice = GetDeviceById(toLPWSTR(deviceId));
		CHECK_PTR(env, pDevice);

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByGUID(pDevice, toLPWSTR(id));
		CHECK_PTR(env, pSessionControl);

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume);

		bool mute = false;
		HRESULT res = pSessionVolume->GetMute(&(BOOL)mute);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when getting mute audioSession").ThrowAsJavaScriptException();
		}

		CoUninitialize();

		return Napi::Boolean::New(env, mute);
	}

} // namespace SoundMixer
