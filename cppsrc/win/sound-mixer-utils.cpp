#include <string>
#include "sound-mixer-utils.hpp"

namespace SoundMixerUtils
{

bool deviceEquals(DeviceDescriptor a, DeviceDescriptor b)
{
    if (a.fullName != b.fullName)
        return false;
    if (a.id != b.id)
        return false;
    return a.type == b.type;
}

static uint32_t jenkins_hash(const char *key, char eventType)
{
    size_t i = 0;
    size_t len = strlen(key);
    uint32_t hash = 0;
    while (i != len)
    {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    // adding event type
    hash += eventType;
    hash += hash << 10;
    hash ^= hash >> 6;

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

uint32_t EventPool::getHashCode(DeviceDescriptor device, EventType type)
{
    return jenkins_hash(device.id.c_str(), (char)type);
}

EventPool::EventPool() : counter(0)
{
}

EventPool::~EventPool()
{
    Clear();
}

int EventPool::RegisterEvent(
    DeviceDescriptor device, EventType type, TSFN func)
{
    uint32_t key = getHashCode(device, type);
    if (m_events.count(key) <= 0)
    {
        std::map<int, TSFN> res;
        res[counter] = func;
        m_events[key] = res;
    }
    else
    {
        m_events[key][counter] = func;
    }
    return counter++;
}

bool EventPool::RemoveEvent(DeviceDescriptor device, EventType type, int id)
{
    uint32_t key = getHashCode(device, type);
    if (m_events.count(key) <= 0)
        return false;

    if (m_events[key].count(id) > 0)
    {
        m_events[key][id].Release();
        return m_events[key].erase(id) > 0;
    }
    else
    {
        return false;
    }
}

std::vector<TSFN> EventPool::GetListeners(
    DeviceDescriptor device, EventType type)
{
    uint32_t key = getHashCode(device, type);
    std::vector<TSFN> res;
    if (m_events.count(key) <= 0)
        return res;
    std::map<int, TSFN> contained = m_events[key];
    int i = 0;
    for (auto it = contained.begin(); it != contained.end(); ++it)
    {
        res.push_back(it->second);
    }

    return res;
}

void EventPool::RemoveAllListeners(DeviceDescriptor device, EventType type)
{
    uint32_t key = getHashCode(device, type);
    std::map<int, TSFN> contained = m_events[key];
    for (auto it = contained.begin(); it != contained.end(); ++it)
    {
        it->second.Release();
    }
    m_events.erase(key);
}

void EventPool::Clear()
{
    for (auto it1 = m_events.begin(); it1 != m_events.end(); ++it1)
    {
        std::map<int, TSFN> el = it1->second;
        for (auto it2 = el.begin(); it2 != el.end(); ++it2)
        {
            it2->second.Release();
        }
    }
    m_events.clear();
    counter = 0;
}

void CallJs(Napi::Env env, Napi::Function cb,
    Napi::Reference<Napi::Value> *owner, NotificationHandler *data)
{
    if (env == nullptr || cb == nullptr)
    {
        if (data != nullptr)
            delete data;
        return;
    }

    if (data != nullptr)
    {
        Napi::Value value;
        if (data->flags & DEVICE_CHANGE_MASK_MUTE)
        {
            value = Napi::Boolean::New(env, data->mute);
        }
        else /*if (data->flags & DEVICE_CHANGE_MASK_VOLUME) */
        {
            value = Napi::Number::New(env, data->volume);
        }
        cb.Call(owner->Value(), {value});

        delete data;
    }
}
} // namespace SoundMixerUtils
