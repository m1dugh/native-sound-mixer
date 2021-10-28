#pragma once

#include <string>

namespace SoundMixerUtils
{

	enum DeviceType
	{
		OUTPUT = 0,
		INPUT = 1,
		ALL = 2
	};

	typedef struct
	{
		std::string fullName;
		std::string id;
		DeviceType type;
	} DeviceDescriptor;

	typedef struct {
		unsigned float right;
		unsigned float left;
		bool stereo;
	} VolumeBalance;
}