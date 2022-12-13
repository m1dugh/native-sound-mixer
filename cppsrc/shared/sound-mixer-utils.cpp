#include "../headers/sound-mixer-utils.hpp"
#include <string>
#include <iostream>

namespace SoundMixerUtils {

    bool deviceEquals(DeviceDescriptor a, DeviceDescriptor b) {
        if(a.fullName != b.fullName)
            return false;
        if(a.id != b.id)
            return false;
        return a.type == b.type;
    }

    uint32_t jenkins_hash(const char *key) {
        size_t i = 0;
        size_t len = strlen(key);
        uint32_t hash = 0;
        while (i != len) {
            hash += key[i++];
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;
        return hash;
    }

    uint32_t hashcode(DeviceDescriptor device) {
        uint32_t name = jenkins_hash(device.fullName.c_str());
        uint32_t id = jenkins_hash(device.id.c_str());
        uint32_t res = device.type << 16;
        res ^= name ^ id;

        return res;
    }

    uint32_t combine_hashes(DeviceDescriptor device, EventType type) {
        uint32_t typeval = 0;
        typeval |= type;
        return hashcode(device) ^ typeval;
    }

    EventPool::EventPool(): m_events(), counter(0) {}

    EventPool::~EventPool() {
        Clear();
    }

    int EventPool::RegisterEvent(DeviceDescriptor device, EventType type, Napi::FunctionReference* func) {
        uint32_t key = combine_hashes(device, type);

        if(m_events.count(key) <= 0) {
            std::map<int, Napi::FunctionReference*> res;
            res[counter] = func;
            m_events[key] = res;
        } else {
            m_events[key][counter] = func;
        }
        return counter++;
    }

    bool EventPool::RemoveEvent(DeviceDescriptor device, EventType type, int id) {
        uint32_t key = combine_hashes(device, type);
        if(m_events.count(key) <= 0)
            return false;

        if(m_events[key].count(id) > 0) {
            delete m_events[key][id];
            return m_events[key].erase(id) > 0;
        } else {
            return false;
        }
    }

    std::vector<Napi::FunctionReference*> EventPool::GetListeners(DeviceDescriptor device, EventType type) {
        uint32_t key = combine_hashes(device, type);
        std::vector<Napi::FunctionReference*> res;
        if(m_events.count(key) <= 0)
            return res;
        std::map<int, Napi::FunctionReference*> contained = m_events[key];
        for (auto it = contained.begin(); it != contained.end(); ++it) {
            res.push_back(it->second);
        }

        return res;
    }

    void EventPool::RemoveAllListeners(DeviceDescriptor device, EventType type) {
        uint32_t key = combine_hashes(device, type);
        std::map<int, Napi::FunctionReference*> contained = m_events[key];
        for (auto it = contained.begin(); it != contained.end(); ++it) {
            delete it->second;
        }
        m_events.erase(key);
    }

    void EventPool::Clear() {
        for(auto it1 = m_events.begin(); it1 != m_events.end(); ++it1) {
            std::map<int, Napi::FunctionReference*> el = it1->second;
            for(auto it2 = el.begin(); it2 != el.end(); ++it2) {
                delete it2->second;
            }
        }
        m_events.clear();
        counter = 0;
    }
}
