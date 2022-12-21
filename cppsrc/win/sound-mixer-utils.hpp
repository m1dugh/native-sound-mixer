#pragma once

#include <map>
#include <napi.h>
#include <string>
#include <vector>

#define VALID_VOLUME_BALANCE(balance)             \
    ((balance.right < 1.F && balance.right > 0.F) \
        && (balance.left < 1.F && balance.left > 0.F))

#define DEVICE_CHANGE_MASK_MUTE 1
#define DEVICE_CHANGE_MASK_VOLUME 2 * DEVICE_CHANGE_MASK_MUTE
#define DEVICE_CHANGE_MASK_CHANNEL_COUNT 2 * DEVICE_CHANGE_MASK_VOLUME

namespace SoundMixerUtils
{

/**
 *  \struct NotificationHandler
 *  \brief  A struct containing necessary information when an audio event is
 *  triggered.
 */
typedef struct
{
    int flags;  /*! The flags indicating which variable was changed. It can
                  be DEVICE_CHANGE_MASK_MUTE of DEVICE_CHANGE_MASK_VOLUME. */
    float volume;   /*! The value of the new volume if changed. */
    bool mute;      /*! The value of the new mute flag if changed. */
} NotificationHandler;

void CallJs(Napi::Env env, Napi::Function cb,
    Napi::Reference<Napi::Value> *context, NotificationHandler *data);

using TSFN = Napi::TypedThreadSafeFunction<Napi::Reference<Napi::Value>,
    NotificationHandler, CallJs>;

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

    int RegisterEvent(DeviceDescriptor device, EventType type, TSFN value);
    bool RemoveEvent(DeviceDescriptor device, EventType type, int id);
    std::vector<TSFN> GetListeners(DeviceDescriptor dev, EventType type);
    void RemoveAllListeners(DeviceDescriptor device, EventType type);
    void Clear();

  private:
    static uint32_t getHashCode(DeviceDescriptor, EventType type);

  private:
    std::map<uint32_t, std::map<int, TSFN>> m_events;
    int counter = 0;
};
} // namespace SoundMixerUtils
