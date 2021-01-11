#include "../headers/sound-mixer.h"
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

namespace SoundMixer
{

	IMMDevice *GetDevice(EDataFlow dataFlow, ERole role)
	{

		IMMDeviceEnumerator *enumerator = nullptr;
		HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&enumerator);
		CHECK_RES(result)

		IMMDevice *pDevice = nullptr;
		result = enumerator->GetDefaultAudioEndpoint(dataFlow, role, &pDevice);
		CHECK_RES(result)

		return pDevice;
	}

	IAudioSessionControl2 *GetAudioSessionByProcessId(IMMDevice *pDevice, DWORD processId)
	{

		IAudioSessionManager2 *pSessionManager = nullptr;
		HRESULT result = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID *)&pSessionManager);
		CHECK_RES(result)

		IAudioSessionEnumerator *pSessionEnumerator = nullptr;
		result = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		CHECK_RES(result)

		IAudioSessionControl *pSessionControl = nullptr;
		IAudioSessionControl2 *pSessionControl2 = nullptr;
		int size = -1;
		pSessionEnumerator->GetCount(&size);

		for (size_t i = 0; i < size; i++)
		{
			result = pSessionEnumerator->GetSession(i, (IAudioSessionControl **)&pSessionControl);
			if (result == S_OK)
			{

				result = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID *)&pSessionControl2);
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

	IAudioEndpointVolume *GetDeviceEndpointVolume(IMMDevice *pDevice)
	{

		IAudioEndpointVolume *pEndpointVolume = nullptr;
		HRESULT result = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&pEndpointVolume);
		CHECK_RES(result)

		return pEndpointVolume;
	}

	ISimpleAudioVolume *GetSessionVolume(IAudioSessionControl2 *pSessionControl)
	{

		ISimpleAudioVolume *pAudioVolume = nullptr;
		HRESULT result = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (LPVOID *)&pAudioVolume);
		CHECK_RES(result)

		return pAudioVolume;
	}

	std::vector<IAudioSessionControl2 *> GetAudioSessions(IMMDevice *pDevice)
	{

		CoInitialize(NULL);

		std::vector<IAudioSessionControl2 *> audioSessions = {};

		IAudioSessionManager2 *pSessionManager = NULL;
		HRESULT result = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void **)&pSessionManager);
		if (result != S_OK)
		{
			return {};
		}

		IAudioSessionEnumerator *pSessionEnumerator = NULL;
		result = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		if (result != S_OK)
		{
			return {};
		}
		pSessionManager->Release();

		IAudioSessionControl *pSessionControl = nullptr;
		IAudioSessionControl2 *pSessionControl2 = nullptr;
		int size = -1;
		pSessionEnumerator->GetCount(&size);
		AudioSessionState state;
		for (size_t i = 0; i < size; i++)
		{

			result = pSessionEnumerator->GetSession(i, &pSessionControl);
			if (result == S_OK)
			{
				result = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID *)&pSessionControl2);
				pSessionControl->GetState(&state);

				if (result == S_OK && state == AudioSessionState::AudioSessionStateActive)
					audioSessions.push_back(pSessionControl2);
			}
		}

		pSessionEnumerator->Release();
		CoUninitialize();
		return audioSessions;
	}

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		exports.Set("GetSessions", Napi::Function::New(env, GetAudioSessionNames));
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

		if (info.Length() != 2 || !info[0].IsNumber() || !info[1].IsNumber())
		{
			Napi::TypeError::New(env, "expected {dataFlow: number}, {role: number as arguments").ThrowAsJavaScriptException();
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;
		IMMDevice *pDevice = GetDevice((EDataFlow)dataFlow, (ERole)role);

		CHECK_PTR(env, pDevice);
		std::vector<IAudioSessionControl2 *> sessions = GetAudioSessions(pDevice);
		Napi::Array sessionNames = Napi::Array::New(env, sessions.size());
		int i = 0;
		DWORD procId = 0;
		HRESULT result = S_OK;
		for (IAudioSessionControl2 *session : sessions)
		{
			result = session->GetProcessId(&procId);
			if (result == S_OK)
			{
				Napi::Object val = Napi::Object::New(env);
				val.Set(Napi::String::New(env, "id"), procId);
				val.Set(Napi::String::New(env, "path"), GetProcNameFromId(procId));
				sessionNames.Set(i++, val);
			}
		}

		CoUninitialize();
		return sessionNames;
	}

	Napi::Number SetEndpointVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 3 || !info[2].IsNumber())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, -1);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		float volume = info[2].As<Napi::Number>().FloatValue();
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < 0.F)
		{
			volume = 0.F;
		}

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume)

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
		if (info.Length() != 2)
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0.F);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume)

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

		if (info.Length() != 3 || !info[2].IsBoolean())
		{
			Napi::TypeError::New(env, "expected {dataFlow: number}, {role: number},{mute: boolean} as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		bool mute = info[2].As<Napi::Boolean>().Value();

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume)

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
		if (info.Length() != 2)
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioEndpointVolume *pEndpointVolume = GetDeviceEndpointVolume(pDevice);
		CHECK_PTR(env, pEndpointVolume)

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

		if (info.Length() != 4 || !info[2].IsNumber() || !info[3].IsNumber())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, -1);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		DWORD id = info[2].As<Napi::Number>().Int32Value();

		float volume = info[3].As<Napi::Number>().FloatValue();
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < 0.F)
		{
			volume = 0.F;
		}

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByProcessId(pDevice, id);
		CHECK_PTR(env, pSessionControl)

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume)

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
		if (info.Length() != 3 || !info[2].IsNumber())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0.F);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		DWORD id = info[2].As<Napi::Number>().Int32Value();

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByProcessId(pDevice, id);
		CHECK_PTR(env, pSessionControl)

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume)

		float volume = 0.F;
		HRESULT res = pSessionVolume->GetMasterVolume(&volume);
		if (res != S_OK)
		{
			Napi::Error::New(env, "an error occured when setting the volume level for audioSession").ThrowAsJavaScriptException();
		}

		CoUninitialize();

		return Napi::Number::New(env, volume);
	}

	Napi::Boolean SetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 4 || !info[2].IsNumber() || !info[3].IsBoolean())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		DWORD id = info[2].As<Napi::Number>().Int32Value();

		bool mute = info[3].As<Napi::Boolean>().Value();

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByProcessId(pDevice, id);
		CHECK_PTR(env, pSessionControl)

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume)

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

		if (info.Length() != 3 || !info[2].IsString())
		{
			Napi::TypeError::New(env, "expected 3 numbers as arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		CoInitialize(NULL);
		int dataFlow = info[0].As<Napi::Number>().Int32Value();
		dataFlow = dataFlow >= 0 && dataFlow < EDataFlow::EDataFlow_enum_count ? dataFlow : EDataFlow::eRender;

		int role = info[1].As<Napi::Number>().Int32Value();
		role = role >= 0 && role < ERole::ERole_enum_count ? role : ERole::eConsole;

		DWORD id = info[2].As<Napi::Number>().Int32Value();

		IMMDevice *pDevice = GetDevice(dataFlow, role);
		CHECK_PTR(env, pDevice)

		IAudioSessionControl2 *pSessionControl = GetAudioSessionByProcessId(pDevice, id);
		CHECK_PTR(env, pSessionControl)

		ISimpleAudioVolume *pSessionVolume = GetSessionVolume(pSessionControl);
		CHECK_PTR(env, pSessionVolume)

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
