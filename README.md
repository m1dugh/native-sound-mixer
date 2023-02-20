# Native Sound Mixer
## Introduction 
### Native cross-platform sound mixer

This `node.js` project is a sound mixer for node desktop apps providing control over volume for each render/capture device (I/O devices) and for each audio session in an audio device separately.

The native `c++` code is provided in `cppsrc/` and compiled using [node-addon-api](https://github.com/nodejs/node-addon-api)

postinstall scripts will automatically build bin files


## Install
This is a [Node js](https://nodejs.org) package available through [npm registry](https://npmjs.org).

### prerequisites
#### Linux 
 - install `pulseaudio` server (already installed on most user-friendly distros)

#### Windows
 - no prerequisites are required for windows.

### Install

```
> npm install native-sound-mixer
```
or using yarn :
```
> yarn add native-sound-mixer
```


## Features
 - Per-device volume control and monitoring
 - Per-audio session volume control and monitoring within each device
 - stereo volume control for session and device
 - Fully compatible with [TypeScript](https://www.typescriptlang.org/)


-----

## DOCUMENTATION
### Summary :

1. [SoundMixer](#1-SoundMixer): factory, default export
	- [(static Attribute) devices](#get-devices): `readonly`
	- [(static Method) getDefaulDevice](#getDefaultDevice)
2. [Device](#2-Device): Represents a physical/virtual device with channels and volume controls
	- [(Attribute) sessions](#get-sessions): `readonly`
	- [(Attribute) mute](#device-mute): `read-write`
	- [(Attribute) volume](#device-volume): `read-write`
	- [(Attribute) balance](#device-balance): `read-write`

3. [AudioSession](#3-AudioSession): Represents an app-linked audio channel with volume controls
	- [(Attribute) mute](#session-mute): `read-write`
	- [(Attribute) volume](#session-volume): `read-write`
	- [(Attribute) balance](#session-balance): `read-write`
	- [(Attribute) state](#session-state): `readonly`

4. [Data Structures](#4-Data-Structures)
	- [Volume Scalar](#volumescalar)
	- [Volume Balance](#volumebalance)
	- [AudioSessionState](#audiosessionstate)
	- [DeviceType](#devicetype)



### 1) SoundMixer
- ### get devices 
this function returns all the [`devices`](#2-Device) found by the system.
   ```TypeScript
   import SoundMixer, {Device} from "native-sound-mixer";

   const devices: Device[] = SoundMixer.devices;
   ```
- ### getDefaultDevice
returns the default device for the specified [`DeviceType`](#DeviceType), if none found returns undefined.
```TypeScript
import SoundMixer, {Device, DeviceType} from "native-sound-mixer";

const device: Device | undefined = SoundMixer.getDefaultDevice(DeviceType.RENDER);
```



### 2) Device

```Typescript
class Device {
	private constructor(); // Device instantiation is disallowed
	public volume: VolumeScalar;
	public mute: boolean;
	public readonly name: string;
	public readonly type: DeviceType;
	public readonly sessions: AudioSession[];
}
```


 - ### get sessions
returns all the [`AudioSessions`](#3-AudioSession) linked to the `Device`.

```TypeScript
// import ...

let device: Device;
// set device to any valid Device object.

const sessions: AudioSession[] = device.sessions;
```

 - ### device mute
gets and sets `mute` value for the device. 
```TypeScript
// import ...

// retrieving the mute flag 
const mute: boolean = device.mute;

// toggling mute
device.mute = !mute;
```

 - ### device volume
gets and sets the [`volume scalar`](#volumescalar) for the device. 
```TypeScript
// import ...

// retrieving the volume 
const volume: VolumeScalar = device.volume;

// adding 10% to volume
device.volume += .1;
```

 - ### device balance
gets and sets the [`volume balance`](#volumebalance) for the device.

```TypeScript
// import ...

// retrieving the volume 
const balance: VolumeBalance = device.balance;

// sets right VolumeScalar to 1 and left VolumeScalar to .5
// by default, left and right are equal to the VolumeScalar of the device
device.balance = {right: 1, left: .5};
```


### 3) AudioSession

```Typescript
// class declaration
class AudioSession {
	private constructor(); // AudioSession instantiation is disallowed

	public volume: VolumeScalar;
	public mute: boolean;
	public readonly name: string;
	public readonly appName: string;
}
```


 - ### session mute
sets and gets the mute flag for the `AudioSession`.

```TypeScript
// import ...

let session: AudioSession;
// set session to a valid session object
const mute: boolean = session.mute;
// toggling mute 
session.mute = !mute;
```
 - ### session volume
sets and gets the [`VolumeScalar`](#volumescalar) for the `AudioSession`.

```TypeScript
// import ...

let session: AudioSession;
// set session to a valid session object
const volume: VolumeScalar = session.volume;
// adding 10% to volume
session.volume += .1;
```
 - ### session balance
gets and sets the [`volume balance`](#volumebalance) for the session.

```TypeScript
// import ...

// retrieving the volume
let session: AudioSession;
const balance: VolumeBalance = session.balance;

// sets right VolumeScalar to 1 and left VolumeScalar to .5
// by default, left and right are equal to the VolumeScalar of the session
session.balance = {right: 1, left: .5};
```
 
 - ### session state
gets the [`AudioSessionState`](#audiosessionstate) for the session.

```TypeScript
// import ...

// retrieving the state
let session: AudioSession;
const state: AudioSessionState = session.state;

if (state === AudioSessionState.ACTIVE) {
    // do something...
}
```


### 4) Data Structures

 - ### VolumeScalar
 a clamped float betwen 0 and 1 representing the power of the volume, 1 is max power and 0 is no output.

 - ### VolumeBalance
 a structure representing the stereo balance for a device and an audio session
```Typescript
interface VolumeBalance {
	right: Number; // float
	left: Number; // float
	stereo: Boolean; // only for Device::balance
}
```

 - ### AudioSessionState
 an enumeration representing the state of the audio session. Possible values are
```TypeScript
import {AudioSessionState} from "native-sound-mixer";

AudioSessionState.INACTIVE; // session is incative but valid
AudioSessionState.ACTIVE; // session is active
AudioSessionState.EXPIRED; // session no longer exists or is no longer available
```
 - ### DeviceType
an enumeration representing the type of the device. Possible values are : 

```TypeScript
import {DeviceType} from "native-sound-mixer";

DeviceType.RENDER; // device type is output
DeviceType.CAPTURE; // device type is input
DeviceType.ALL; // device type is both input and output

```
-----

## Contributing
As an open-source project, every one is free to modify the codebase. Feel free to open any discussion or issue or PR to improve this project.

-----

## License
This project is under [MIT](https://github.com/romlm/native-sound-mixer/blob/develop/LICENSE) license
