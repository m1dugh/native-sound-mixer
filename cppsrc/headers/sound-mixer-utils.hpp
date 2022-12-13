#pragma once

#include <string>
#include <napi.h>
#include <map>
#include <vector>

#define VALID_VOLUME_BALANCE(balance) ((balance.right < 1.F && balance.right > 0.F) && (balance.left < 1.F && balance.left > 0.F))

namespace SoundMixerUtils
{

	enum DeviceType
	{
		OUTPUT = 0,
		INPUT = 1,
		ALL = 2
	};

    enum EventType
    {
        VOLUME = 0,
        MUTE = 1,
        COUNT = 2
    };

	typedef struct
	{
		std::string fullName;
		std::string id;
		DeviceType type;
	} DeviceDescriptor;

    bool deviceEquals(DeviceDescriptor a, DeviceDescriptor b);
    uint32_t hashcode(DeviceDescriptor device);

	typedef struct
	{
		float right;
		float left;
		bool stereo;
	} VolumeBalance;

    class EventPool {
        public:
            EventPool();
            virtual ~EventPool();

            int RegisterEvent(DeviceDescriptor device, EventType type, Napi::FunctionReference* value);
            bool RemoveEvent(DeviceDescriptor device, EventType type, int id);
            std::vector<Napi::FunctionReference*> GetListeners(DeviceDescriptor dev, EventType type);
            void RemoveAllListeners(DeviceDescriptor device, EventType type);
            void Clear();

        private:
            int counter = 0;
            std::map<uint32_t, std::map<int, Napi::FunctionReference*>> m_events;
    };
}
