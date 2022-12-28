#include "sound-mixer.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{

    SoundMixer::Init(env, exports);

    return exports;
}

NODE_API_MODULE(SoundMixer, InitAll);
