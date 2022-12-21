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
                  be DEVICE_CHANGE_MASK_MUTE or DEVICE_CHANGE_MASK_VOLUME. */
    float volume;   /*! The value of the new volume if changed. */
    bool mute;      /*! The value of the new mute flag if changed. */
} NotificationHandler;

/**
 *  \brief  The function called by the `node-addon-api` callbacks in the
 *  background.
 *  \fn void CallJs(Napi::Env env, Napi::Function cb,
 *  Napi::Reference<Napi::Value> *context, NotificationHandler *data);
 *  \param env  The environment to call in.
 *  \param cb   The actual callback to be ran.
 *  \param context  A pointer to the owner of the callback.
 *  \param data A pointer to a NotificationHandler struct indicating the
 *  changes.
 */
void CallJs(Napi::Env env, Napi::Function cb,
    Napi::Reference<Napi::Value> *context, NotificationHandler *data);

/**
 *  \brief  The type used to enclose various `node-addon-api` callbacks.
 *  A type enclosing a `Napi::TypedThreadSafeFunction`
 */
using TSFN = Napi::TypedThreadSafeFunction<Napi::Reference<Napi::Value>,
    NotificationHandler, CallJs>;

/**
 *  \enum DeviceType
 *  \brief An enum representing the type of Device.
 *  \remarks A device can be either an output device, an input device, or both.
 */
enum DeviceType
{
    OUTPUT = 0, /*!< Enum value OUTPUT */
    INPUT = 1,  /*!< Enum value INPUT */
    ALL = 2     /*!< Enum value ALL */
};

/**
 *  \enum   EventType
 *  \brief  An enum representing the type of events triggered in a device
 *  or in an audio session.
 */
enum EventType
{
    VOLUME = 0, /*!< Enum value VOLUME */
    MUTE = 1,   /*!< Enum value MUTE */
    COUNT = 2
};

/**
 *  \struct DeviceDescriptor
 *  \brief  Reprensents an audio device.
 */
typedef struct
{
    std::string fullName;   /*! The name of the Device */
    std::string id;         /*! The unique id of the Device */
    DeviceType type;        /*! The DeviceType of the Device */
} DeviceDescriptor;

/**
 *  \fn bool deviceEquals(DeviceDescriptor a, DeviceDescriptor b);
 *  \brief checks if a and b params are equal.
 *  \param a
 *  \param b
 *  \return true if the two DeviceDescriptor are equal.
 */
bool deviceEquals(DeviceDescriptor a, DeviceDescriptor b);

/**
 *  \fn uint32_t hashcode(DeviceDescriptor device);
 *  \brief  return the hashcode of the device.
 *  \remarks    Although a same DeviceDescriptor will always return the same
 *  hashcode, there might be primary collisions in the hashes, that is,
 *  two DeviceDescriptor with different values but with the same hashcode.
 *  \param device   The DeviceDescriptor.
 *  \return The hashcode of the DeviceDescriptor.
 */
uint32_t hashcode(DeviceDescriptor device);

/**
 *  \struct VolumeBalance
 *  \brief The right-left volume balance in a stereo device.
 *  \remarks Both the provided ratio are based on max volume, that is,
 *  the actual volume is the average of right and left.
 */
typedef struct
{
    float right;    /*! The right volume ratio. */
    float left;     /*! The left volume ratio. */
    bool stereo;    /*! A flag indicating whether the device is
                      stereo or not */
} VolumeBalance;


/**
 *  \class EventPool
 *  A pool storing all the registered events bound to the corresponding 
 *  DeviceDescriptor.
 */
class EventPool {
    public:
        /**
         *  A basic contructor.
         */
        EventPool();

        /**
         *  A basic destructor.
         */
        virtual ~EventPool();

        /**
         *  \fn int RegisterEvent(DeviceDescriptor device, EventType type,
         *  TSFN callback);
         *  \brief  Registers and event for the specified device and event.
         *  \param device   The device to bind the callback to.
         *  \param type     The event type to bind the callback to.
         *  \param callback The effective callback.
         *  \return The handle of the registered event if succeeded, -1 
         *  otherwise.
         *  \see RemoveEvent
         */
        int RegisterEvent(DeviceDescriptor device, EventType type, TSFN value);

        /**
         *  \fn bool RemoveEvent(DeviceDescriptor device, EventType type,
         *  int handle);
         *  \brief  Removes an event based on the given device and type.
         *  \param device   The specified device.
         *  \param type The specified event type.
         *  \param handle   The given callback handle.
         *  \return true if removed, false otherwise.
         *  \see    RegisterEvent
         */
        bool RemoveEvent(DeviceDescriptor device, EventType type, int id);

        /**
         *  \fn std::vector<TSFN> GetListeners(DeviceDescriptor device,
         *  EventType type);
         *  \brief returns a vector of listeners corresponding to given device
         *  and event type.
         *  \param device   The specified device.
         *  \param type The specified event type.
         *  \return The list of found event callbacks.
         *  \see RegisterEvent
         */
        std::vector<TSFN> GetListeners(DeviceDescriptor dev, EventType type);

        /**
         *  \fn void RemoveAllListeners(DeviceDescriptor device, EventType
         *  type);
         *  \brief  Removes all listeners for a given device and event type.
         *  \param device The given device.
         *  \param type The given event type.
         *  \see    RemoveEvent
         *  \see    Clear
         */
        void RemoveAllListeners(DeviceDescriptor device, EventType type);

        /**
         *  \fn void Clear()
         *  \brief   Removes all listeners from all devices.
         *  \see RemoveAllListeners
         */
        void Clear();

    private:
        static uint32_t getHashCode(DeviceDescriptor, EventType type);

    private:
        std::map<uint32_t, std::map<int, TSFN>> m_events;
        int counter = 0;
};
} // namespace SoundMixerUtils
