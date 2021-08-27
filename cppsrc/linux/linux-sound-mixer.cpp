#include "./headers/linux-sound-mixer.hpp"

#include <iostream>

#define WAIT(op, ml)                      \
	do                                    \
	{                                     \
		pa_mainloop_iterate(ml, 1, NULL); \
	} while (pa_operation_get_state(op) != PA_OPERATION_DONE);

using std::vector;

// SoundMixer definition
namespace LinuxSoundMixer
{

	void state_cb(pa_context *ctx, int *ready)
	{
		switch (pa_context_get_state(ctx))
		{
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_TERMINATED:
			*ready = -1;
			break;
		case PA_CONTEXT_READY:
			*ready = 1;
			break;
		default:
			*ready = 0;
			break;
		}
	}

	SoundMixer::SoundMixer()
	{
		pa_mainloop *ml = pa_mainloop_new();
		pa_mainloop_api *api = pa_mainloop_get_api(ml);
		pa_context *ctx = pa_context_new(api, "sound-mixer");
		pa = _PAControls{
			ml,
			api,
			ctx,
		};

		if (pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL) <= 0)
		{
			ready = -1;
		}

		pa_context_set_state_callback(ctx, (pa_context_notify_cb_t)state_cb, &ready);

		while (ready <= 0)
		{
			pa_mainloop_iterate(ml, 1, NULL);
		}
	}

	SoundMixer::~SoundMixer()
	{
		pa_context_disconnect(pa.ctx);
		pa_context_unref(pa.ctx);
		pa_mainloop_free(pa.mainloop);
	}

	bool SoundMixer::isReady()
	{
		return (bool)ready;
	}

	typedef struct
	{
		_PAControls controls;
		vector<_Device *> *devices;
	} GetDevicesData;

	void _get_input_devices_cb(pa_context *ctx, pa_sink_info *info, int eol, GetDevicesData *data)
	{
		if (eol)
		{
			return;
		}
		data->devices->push_back(new OutputDevice(data->controls, info->index));
	}

	void _get_output_devices_cb(pa_context *ctx, pa_source_info *info, int eol, GetDevicesData *data)
	{
		if (eol)
		{
			return;
		}
		data->devices->push_back(new InputDevice(data->controls, info->index));
	}

	vector<_Device *> SoundMixer::GetDevices()
	{
		vector<_Device *> result;
		if (!ready)
		{
			return result;
		}

		GetDevicesData data{
			pa,
			&result,
		};

		pa_operation *op = pa_context_get_sink_info_list(pa.ctx, (pa_sink_info_cb_t)_get_input_devices_cb, &data);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);

		op = pa_context_get_source_info_list(pa.ctx, (pa_source_info_cb_t)_get_output_devices_cb, &data);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);

		return result;
	}

	_Device *SoundMixer::GetDeviceByName(std::string name, DeviceType type)
	{
		vector<_Device *> result;
		GetDevicesData data{
			pa,
			&result,
		};
		pa_operation *op;
		if (type == DeviceType::INPUT)
		{
			op = pa_context_get_source_info_by_name(pa.ctx, name.c_str(), (pa_source_info_cb_t)_get_output_devices_cb, &data);
		}
		else
		{
			op = pa_context_get_sink_info_by_name(pa.ctx, name.c_str(), (pa_sink_info_cb_t)_get_input_devices_cb, &data);
		}
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);

		if (result.size() > 0)
		{
			return result.at(0);
		}
		return nullptr;
	}

}

//definition for Device
namespace LinuxSoundMixer
{

	_Device::_Device(_PAControls controls, uint32_t index) : index(index)
	{
		pa = controls;
	}

	_Device::~_Device()
	{
	}

	DeviceDescriptor _Device::toDeviceDescriptor()
	{
		return DeviceDescriptor{
			name(),
			friendlyName(),
			type()};
	}

	// InputDevice

	void _output_device_get_info_cb(pa_context *ctx, pa_source_info *info, int eol, pa_source_info **data)
	{
		if (eol)
		{
			return;
		}

		*data = info;
	}

	pa_source_info *InputDevice::GetInfo(pa_operation **op)
	{
		pa_source_info *info;
		*op = pa_context_get_source_info_by_index(pa.ctx, index, (pa_source_info_cb_t)_output_device_get_info_cb, &info);
		WAIT(*op, pa.mainloop);
		return info;
	}

	float InputDevice::GetVolume()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		pa_volume_t volume = pa_cvolume_avg(&(info->volume));
		pa_operation_unref(op);
		return (float)volume / PA_VOLUME_NORM;
	}

	bool InputDevice::GetMute()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		int mute = info->mute;
		pa_operation_unref(op);
		return (bool)mute;
	}

	void InputDevice::SetVolume(float v)
	{
		uint32_t volume = v * PA_VOLUME_NORM;
		pa_operation *op;
		pa_source_info *info = GetInfo(&op);

		pa_cvolume vol = info->volume;
		for (size_t i = 0; i < vol.channels; i++)
		{
			vol.values[i] = volume;
		}
		pa_operation_unref(op);
		op = pa_context_set_source_volume_by_index(pa.ctx, index, &vol, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	void InputDevice::SetMute(bool mute)
	{
		pa_operation *op = pa_context_set_source_mute_by_index(pa.ctx, index, (int)mute, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	struct _AudioSessionData
	{
		uint32_t deviceIndex;
		vector<_AudioSession *> *sessions;
		_PAControls controls;
	};

	void _output_device_get_audio_sessions_cb(pa_context *ctx, pa_source_output_info *info, int eol, struct _AudioSessionData *data)
	{
		if (eol)
		{
			return;
		}

		if (info->source == data->deviceIndex)
		{
			data->sessions->push_back(new InputAudioSession(data->controls, info->index));
		}
	}

	vector<_AudioSession *> InputDevice::GetAudioSessions()
	{
		vector<_AudioSession *> sessions;

		struct _AudioSessionData data
		{
			index,
				&sessions,
				pa,
		};

		pa_operation *op = pa_context_get_source_output_info_list(pa.ctx, (pa_source_output_info_cb_t)_output_device_get_audio_sessions_cb, &data);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);

		return sessions;
	}

	void _output_device_get_props_cb(pa_context *ctx, pa_source_info *info, int eol, pa_proplist **props)
	{
		if (eol)
		{
			return;
		}

		*props = pa_proplist_copy(info->proplist);
	}

	pa_proplist *InputDevice::GetProps()
	{
		pa_proplist *res;
		pa_operation *op = pa_context_get_source_info_by_index(pa.ctx, index, (pa_source_info_cb_t)_output_device_get_props_cb, &res);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
		return res;
	}

	std::string InputDevice::friendlyName()
	{

		auto *props = GetProps();
		std::string name = this->name();
		if (0 && pa_proplist_contains(props, PA_PROP_DEVICE_DESCRIPTION))
		{
			name = pa_proplist_gets(props, PA_PROP_DEVICE_DESCRIPTION);
		}
		pa_proplist_free(props);
		return name;
	}

	DeviceType InputDevice::type()
	{
		return DeviceType::INPUT;
	}

	std::string InputDevice::name()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		std::string name = info->name;
		pa_operation_unref(op);
		return name;
	}

	// OutputDevice

	OutputDevice::OutputDevice(_PAControls controls, uint32_t index) : _Device(controls, index)
	{
	}

	void _input_device_get_info_cb(pa_context *ctx, pa_sink_info *info, int eol, pa_sink_info **data)
	{
		if (eol)
		{
			return;
		}

		*data = info;
	}

	pa_sink_info *OutputDevice::GetInfo(pa_operation **op)
	{
		pa_sink_info *info;
		*op = pa_context_get_sink_info_by_index(pa.ctx, index, (pa_sink_info_cb_t)_input_device_get_info_cb, &info);
		WAIT(*op, pa.mainloop);
		return info;
	}

	float OutputDevice::GetVolume()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		pa_volume_t volume = pa_cvolume_avg(&(info->volume));
		pa_operation_unref(op);
		return (float)volume / PA_VOLUME_NORM;
	}

	bool OutputDevice::GetMute()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		int mute = info->mute;
		pa_operation_unref(op);
		return (bool)mute;
	}

	void OutputDevice::SetVolume(float v)
	{
		uint32_t volume = v * PA_VOLUME_NORM;
		pa_operation *op;
		auto *info = GetInfo(&op);

		pa_cvolume vol = info->volume;
		for (size_t i = 0; i < vol.channels; i++)
		{
			vol.values[i] = volume;
		}
		pa_operation_unref(op);
		op = pa_context_set_sink_volume_by_index(pa.ctx, index, &vol, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	void OutputDevice::SetMute(bool mute)
	{
		pa_operation *op = pa_context_set_sink_mute_by_index(pa.ctx, index, (int)mute, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	void _input_device_get_audio_sessions_cb(pa_context *ctx, pa_sink_input_info *info, int eol, struct _AudioSessionData *data)
	{
		if (eol)
		{
			return;
		}

		if (info->sink == data->deviceIndex)
		{
			data->sessions->push_back(new OutputAudioSession(data->controls, info->index));
		}
	}

	vector<_AudioSession *> OutputDevice::GetAudioSessions()
	{
		vector<_AudioSession *> sessions;

		struct _AudioSessionData data
		{
			index,
				&sessions,
				pa,
		};

		pa_operation *op = pa_context_get_sink_input_info_list(pa.ctx, (pa_sink_input_info_cb_t)_input_device_get_audio_sessions_cb, &data);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);

		return sessions;
	}

	DeviceType OutputDevice::type()
	{
		return DeviceType::OUTPUT;
	}

	std::string OutputDevice::name()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		std::string name = info->name;
		pa_operation_unref(op);
		return name;
	}

	std::string OutputDevice::friendlyName()
	{
		auto *props = GetProps();
		std::string name = this->name();
		if (pa_proplist_contains(props, PA_PROP_DEVICE_DESCRIPTION))
		{
			name = pa_proplist_gets(props, PA_PROP_DEVICE_DESCRIPTION);
		}
		pa_proplist_free(props);
		return name;
	}

	void _input_device_get_props_cb(pa_context *ctx, pa_sink_info *info, int eol, pa_proplist **props)
	{
		if (eol)
		{
			return;
		}

		*props = pa_proplist_copy(info->proplist);
	}

	pa_proplist *OutputDevice::GetProps()
	{
		pa_proplist *res;
		pa_operation *op = pa_context_get_sink_info_by_index(pa.ctx, index, (pa_sink_info_cb_t)_output_device_get_props_cb, &res);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
		return res;
	}

};

// AudioSessions
namespace LinuxSoundMixer
{
	_AudioSession::_AudioSession(_PAControls controls, uint32_t index) : index(index)
	{
		pa = controls;
	}

	_AudioSession::~_AudioSession()
	{
	}

	InputAudioSession::InputAudioSession(_PAControls controls, uint32_t index) : _AudioSession(controls, index)
	{
	}

	template <typename T>
	struct _SessionData
	{
		T **info;
		uint32_t index;
	};

	void _output_session_get_info_cb(pa_context *ctx, pa_source_output_info *info, int eol, struct _SessionData<pa_source_output_info> *data)
	{
		if (eol)
		{
			return;
		}

		*data->info = info;
	}

	pa_source_output_info *InputAudioSession::GetInfo(pa_operation **op)
	{
		pa_source_output_info *info;
		struct _SessionData<pa_source_output_info> data
		{
			&info,
				index
		};
		*op = pa_context_get_source_output_info(pa.ctx, index, (pa_source_output_info_cb_t)_output_session_get_info_cb, &data);
		WAIT(*op, pa.mainloop);

		return info;
	}

	float InputAudioSession::GetVolume()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		pa_volume_t volume = pa_cvolume_avg(&(info->volume));
		pa_operation_unref(op);
		return (float)volume / PA_VOLUME_NORM;
	}

	bool InputAudioSession::GetMute()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		int mute = info->mute;
		pa_operation_unref(op);
		return (bool)mute;
	}

	void InputAudioSession::SetVolume(float v)
	{
		uint32_t volume = v * PA_VOLUME_NORM;
		pa_operation *op;
		auto *info = GetInfo(&op);

		pa_cvolume vol = info->volume;
		for (size_t i = 0; i < vol.channels; i++)
		{
			vol.values[i] = volume;
		}
		pa_operation_unref(op);
		op = pa_context_set_source_output_volume(pa.ctx, index, &vol, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	void InputAudioSession::SetMute(bool mute)
	{
		pa_operation *op = pa_context_set_source_output_mute(pa.ctx, index, (int)mute, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	std::string InputAudioSession::description()
	{
		pa_operation *op;
		auto *s = GetInfo(&op);
		std::string name = s->name;
		pa_operation_unref(op);
		return name;
	}

	void _output_session_get_props_cb(pa_context *ctx, pa_source_output_info *info, int eol, pa_proplist **props)
	{
		if (eol)
		{
			return;
		}

		*props = pa_proplist_copy(info->proplist);
	}

	pa_proplist *InputAudioSession::GetProps()
	{
		pa_proplist *res;
		pa_operation *op = pa_context_get_source_output_info(pa.ctx, index, (pa_source_output_info_cb_t)_output_session_get_props_cb, &res);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
		return res;
	}

	std::string InputAudioSession::appName()
	{
		auto *props = GetProps();
		std::string name = this->description();
		if (pa_proplist_contains(props, PA_PROP_APPLICATION_NAME))
		{
			name = pa_proplist_gets(props, PA_PROP_APPLICATION_NAME);
		}
		pa_proplist_free(props);
		return name;
	}

	// OutputAudioSession
	OutputAudioSession::OutputAudioSession(_PAControls controls, uint32_t index) : _AudioSession(controls, index)
	{
	}

	void _input_session_get_info_cb(pa_context *ctx, pa_sink_input_info *info, int eol, struct _SessionData<pa_sink_input_info> *data)
	{
		if (eol)
		{
			return;
		}
		*data->info = info;
	}

	pa_sink_input_info *OutputAudioSession::GetInfo(pa_operation **op)
	{
		pa_sink_input_info *info;
		struct _SessionData<pa_sink_input_info> data
		{
			&info,
				index
		};
		*op = pa_context_get_sink_input_info(pa.ctx, index, (pa_sink_input_info_cb_t)_input_session_get_info_cb, &data);
		WAIT(*op, pa.mainloop);

		return info;
	}

	float OutputAudioSession::GetVolume()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		pa_volume_t volume = pa_cvolume_avg(&(info->volume));
		pa_operation_unref(op);
		return (float)volume / PA_VOLUME_NORM;
	}

	bool OutputAudioSession::GetMute()
	{
		pa_operation *op;
		auto *info = GetInfo(&op);
		int mute = info->mute;
		pa_operation_unref(op);
		return (bool)mute;
	}

	void OutputAudioSession::SetVolume(float v)
	{
		uint32_t volume = v * PA_VOLUME_NORM;
		pa_operation *op;
		auto *info = GetInfo(&op);

		pa_cvolume vol = info->volume;
		for (size_t i = 0; i < vol.channels; i++)
		{
			vol.values[i] = volume;
		}
		pa_operation_unref(op);
		op = pa_context_set_sink_input_volume(pa.ctx, index, &vol, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	void OutputAudioSession::SetMute(bool mute)
	{
		pa_operation *op = pa_context_set_sink_input_mute(pa.ctx, index, (int)mute, NULL, NULL);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
	}

	std::string OutputAudioSession::description()
	{
		pa_operation *op;
		auto *s = GetInfo(&op);
		std::string name = s->name;
		pa_operation_unref(op);
		return name;
	}

	void _input_session_get_props_cb(pa_context *ctx, pa_sink_input_info *info, int eol, pa_proplist **props)
	{
		if (eol)
		{
			return;
		}

		*props = pa_proplist_copy(info->proplist);
	}

	pa_proplist *OutputAudioSession::GetProps()
	{
		pa_proplist *res;
		pa_operation *op = pa_context_get_sink_input_info(pa.ctx, index, (pa_sink_input_info_cb_t)_input_session_get_props_cb, &res);
		WAIT(op, pa.mainloop);
		pa_operation_unref(op);
		return res;
	}

	std::string OutputAudioSession::appName()
	{
		pa_operation *op;
		auto *props = GetProps();
		std::string name = this->description();
		if (pa_proplist_contains(props, PA_PROP_APPLICATION_NAME))
		{
			name = pa_proplist_gets(props, PA_PROP_APPLICATION_NAME);
		}
		pa_proplist_free(props);
		return name;
	}

};