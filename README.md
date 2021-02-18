# Windows Sound Mixer
## Introduction 
### Native sound mixer for windows

This `node.js` project is a sound mixer for node desktop apps providing control over volume for each render/capture device (I/O devices) and for each audio session in an audio device separately.

It's built on top of `windows desktop api` and uses the [windows core audio api](https://docs.microsoft.com/en-us/windows/win32/api/_coreaudio/). The native `c++` code is provided in `cppsrc/` and compiled using [node-addon-api](https://github.com/nodejs/node-addon-api)

postinstall scripts will automatically build bin files


## Install
This is a [Node js](https://nodejs.org) package available through [npm registry](https://npmjs.org)

Once `Node.js` is installed, you can install `native-sound-mixer` with npm : 
```batch
> npm install native-sound-mixer
```
or using yarn :
```batch
> yarn add native-sound-mixer
```


## Features
 - Per-device volume control and monitoring
 - Per-audio session volume control and monitoring within each device
 - Fully compatible with [TypeScript](https://www.typescriptlang.org/)

## Restrictrictions
 - Yet, business logic has only been implemented for Windows 7+


## Documentation
### summary:
1. [terminology](#1-terminology)
	1. [Device](#device)
	1. [AudioSession](#audioSession)
	1. [volume scalar](#volume-scalar)
1. [Information Gathering functions](#2-information-gathering-functions)
	1. [GetDevices](#getdevices)
	1. [GetDefaultRenderDevice](#getdefaultrenderdevice)
	1. [GetDefaultCaptureDevice](#getdefaultcapturedevice)
	1. [GetSessions](#getsessions)
1. [Getters](#3-getters)
	1. Device
		1. [GetDeviceVolume](#getDeviceVolume)
		1. [GetDeviceMute](#getDeviceMute)
	1. AudioSession
		1. [GetAppVolume](#getAppVolume)
		1. [GetAppMute](#getAppMute)
1. [Setters](#4-setters)
	1. Device
		1. [SetDeviceMute](#setDeviceMute)
		1. [SetDeviceVolume](#setDeviceVolume)
		1. [ChangeDeviceVolume](#changeDeviceVolume)
		1. [ToggleDeviceMute](#toggleDeviceMute)
	1. AudioSession
		1. [SetAppMute](#setAppMute)
		1. [SetAppVolume](#setAppVolume)
		1. [ChangeAppVolume](#changeAppVolume)
		1. [ToggleAppMute](#toggleAppMute)

### 1) Terminology
1) #### Device
	- an interface providing infos about a physical I/O sound Device (see [microsoft doc about device](https://docs.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nn-mmdeviceapi-immdevice))
```TypeScript
// @types/model/Device.d.ts

export default interface Device {

	/**
	 * the unique identifier linked to the device
	 */
	readonly id: string;

	/**
	 * the name associated with the device
	 */
	readonly name: string;

	/**
	 * flag determining whether the device render or capture device
	 */
	readonly render: boolean;
}
```

2) #### AudioSession
	- an interface providing infos about an I/O sound stream (see [microsoft doc about AudioSessions](https://docs.microsoft.com/en-us/windows/win32/api/audiopolicy/nn-audiopolicy-iaudiosessioncontrol))
```TypeScript
// @types/model/Device.d.ts

export interface AudioSession {
	/**
	 * the unique id linked to the application emmitting sound
	 */
	readonly id: string;
	/**
	 * the path to the application
	 */
	readonly path: string;
}
```

3) #### volume scalar
	- a float beetween 0 and 1 determining the volume, 1 is 100% of the maximum volume, and 0 is mute.
**N.B: in the app, all volumes are scalars**

### 2) Information gathering functions
*Disclaimer: As of this section, provided documentation will be examples, for further understanding, refer to [C++ sources](https://github.com/romlm/native-sound-mixer/tree/develop/cppsrc).*

1. #### GetDevices
 	- retrieves all I/O sound Devices
 	- **parameters :** none
 	- example: 
	```JavaScript
	const {GetDevices} = require("native-sound-mixer");

	GetDevices(); // returns a List of Devices
	```

1. #### GetDefaultRenderDevice
	- retrieves the default output Device
	- **parameters :** none
	- example: 
	```JavaScript
	const {GetDefaultRenderDevice} = require("native-sound-mixer");

	GetDefaultRenderDevice(); // returns the default output Device
	```
1. #### GetDefaultCaptureDevice
	- retrieves the default input Device
	- **parameters :** none
	- example: 
	```JavaScript
	const {GetDefaultCaptureDevice} = require("native-sound-mixer");

	GetDefaultCaptureDevice(); // returns the default input Device
	```

1. #### GetSessions
	 - retrieves all the audio sessions for a given device
	 - **parameters :**
	 	- deviceId: the id of the device to retrieve the audio sessions
	 - example :
	```TypeScript

	const { GetDefaultRenderDevice, GetSessions } = require("native-sound-mixer");

	// as of this example we'll assume that you yet fetched either the device or its ID
	const device = GetDefaultRenderDevice();

	GetSessions(device.id); // retrieves all active audio sessions linked to this device id
	
	```

### 3) Getters
1. **Device :**
	1. #### GetDeviceVolume:
		- gets the volume scalar of the specified device
		- **parameters :**
			- deviceId: the device id
		- example:
		```JavaScript
			const {GetDeviceVolume} = require("native-sound-mixer")

			...

			GetDeviceVolume(device.id); // gets the current volume scalar
		```

	2. #### GetDeviceMute:
		- returns true if device is muted
		- **partameters :**
			- deviceId: the device id
		- example: 
		```JavaScript
		const {GetDeviceMute} = require("native-sound-mixer");

		...

		GetDeviceMute(device.id); // returns false if device is not muted
		```

2. **Audio Session**
	1. #### GetAppVolume:
		- gets the volume scalar of the specified audio session for a given device
		- **parameters :**
			- deviceId: the device id
			- sessionId: the session id (can be found through GetSessions)
		- example:
		```JavaScript
			const {GetSessions, GetAppVolume} = require("native-sound-mixer")

			...

			// example of how to retrieve audio session id
			// as of this example we'll assume that <session> stands for a valid AudioSession
			const appPath = /*<your app path here>*/
			const session = GetSessions(device.id).find(({path}) => path === appPath);

			if(session)
				GetAppVolume(device.id, session.id); // gets the current volume scalar for the session

		```

		2. #### GetAppMute:
		- returns true if the session is muted
		- **partameters :**
			- deviceId: the device id
			- sessionId: the session id
		- example: 
		```JavaScript
		const {GetAppMute} = require("native-sound-mixer");

		...

		GetAppMute(device.id, session.id); // returns false if session is not muted
		```

### 4) Setters
1. **Device :**
	1. #### SetDeviceMute
		- sets the mute flag for given device
		- returns the new mute flag
		- **parameters :**
			- deviceId: the device id
			- mute: the mute flag (Boolean)
		- example: 
		```JavaScript
		const {SetDeviceMute} = require("native-sound-mixer");

		...

		SetDeviceMute(device.id, true); // mutes the device
		```

	1. #### SetDeviceVolume
		- sets the volume scalar for given device
		- returns the new volume scalar
		- **parameters :**
			- deviceId: the device id
			- volume: the volume scalar
		- example: 
		```JavaScript
		const {SetDeviceVolume} = require("native-sound-mixer");

		...

		SetDeviceVolume(device.id, 0.5); // sets the device at half its max power
		```

	1. #### ToggleDeviceMute
		- toggles the mute flag for given device. a muted device will go unmuted, and an unmuted device will go muted.
		- returns the new mute flag
		- **parameters :**
			- deviceId: the device id
		- example: 
		```JavaScript
		const {ToggleDeviceMute} = require("native-sound-mixer");

		...

		ToggleDeviceMute(device.id); // mutes the device if unmuted
		```

	1. #### ChangeDeviceVolume
		- sets the volume scalar for given device based on a given delta scalar
		- returns the new volume scalar
		- **parameters :**
			- deviceId: the device id
			- deltaVolume: the volume scalar to vary (float from -1.0 to 1.0)
		- example: 
		```JavaScript
		const {ChangeDeviceVolume} = require("native-sound-mixer");

		...

		ChangeDeviceVolume(device.id, 0.5); // sets the device volume scalar to current volume plus half its max power

		ChangeDeviceVolume(device.id, -0.5); // sets the device volume scalar to current volume minus half its max power
		```

1. **Audio Session :**
	1. #### SetAppMute
		- sets the mute flag for given audio session
		- returns the new mute flag
		- **parameters :**
			- deviceId: the device id
			- sessionId: the session id
			- mute: the mute flag (Boolean)
		- example: 
		```JavaScript
		const {SetAppMute} = require("native-sound-mixer");

		...

		SetAppMute(device.id, session.id, true); // mutes the audio session
		```

	1. #### SetAppVolume
		- sets the volume scalar for given audio session
		- returns the new volume scalar
		- **parameters :**
			- deviceId: the device id
			- sessionId: the session id
			- volume: the volume scalar
		- example: 
		```JavaScript
		const {SetAppVolume} = require("native-sound-mixer");

		...

		SetAppVolume(device.id, session.id, 0.5); // sets the audio session to half its max power
		```

	1. #### ToggleAppMute
		- toggles the mute flag for given audio session. a muted audio session will go unmuted, and an unmuted audio session will go muted.
		- returns the new mute flag
		- **parameters :**
			- deviceId: the device id
			- sessionId: the session id
		- example: 
		```JavaScript
		const {ToggleAppMute} = require("native-sound-mixer");

		...

		ToggleAppMute(device.id, session.id); // mutes the audio session if unmuted
		```

	1. #### ChangeAppVolume
		- sets the volume scalar for given audio session based on a given delta scalar
		- returns the new volume scalar
		- **parameters :**
			- deviceId: the device id
			- sessionId: the audio session id
			- deltaVolume: the volume scalar to vary (float from -1.0 to 1.0)
		- example: 
		```JavaScript
		const {ChangeAppVolume} = require("native-sound-mixer");

		...

		ChangeAppVolume(device.id, sesion.id, 0.5); // sets the audio session volume scalar to current volume plus half its max power

		ChangeAppVolume(device.id, session.id, -0.5); // sets the audio session volume scalar to current volume minus half its max power
		```
___

## Contributing
As an open-source project, every one is free to modify the codebase. The [TODO](https://github.com/romlm/native-sound-mixer/blob/develop/TODO.md) file privides all future features with their current development state. Please test your code before committing to this repository.

## License
This project is under [MIT](https://github.com/romlm/native-sound-mixer/blob/develop/LICENSE) license
