#pragma once

#include <string>

#define VALID_VOLUME_BALANCE(balance) ((balance.right < 1.F && balance.right > 0.F) && (balance.left < 1.F && balance.left > 0.F))

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

	typedef struct
	{
		float right;
		float left;
		bool stereo;
	} VolumeBalance;
}
