# Native Sound Mixer
## Introduction 
### Native cross-platform sound mixer

This `node.js` project is a sound mixer for node desktop apps providing control
over volume for each render/capture device (I/O devices) and for each audio
session in an audio device separately.

The native `c++` code is provided in `cppsrc/` and compiled using `cmake-js`
and [node-addon-api](https://github.com/nodejs/node-addon-api)


## Install
This is a [Node js](https://nodejs.org) package available through
[npm registry](https://npmjs.org).

### prerequisites
#### Linux 
 - install `pulseaudio` server (already installed on most user-friendly
distros)

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

## Beta-Phase Features
 - Event monitoring for volume changes and mute flag change (Windows only).

## Upcoming Features
 - Event monitoring for volume changes and mute flag for Linux.
 - Device connection and disconnection events.
 - Audio Session event volume changes and mute flage events listening.


The documentation to the latest release can be found
[here](https://m1dugh.github.io/native-sound-mixer/master).

The documentation to the current develop version can be found
[here](https://m1dugh.github.io/native-sound-mixer/dev).

## Contributing
As an open-source project, every one is free to modify the codebase.
Feel free to open any discussion or issue or PR to improve this project.

-----

## License
This project is under
[MIT](https://github.com/romlm/native-sound-mixer/blob/develop/LICENSE) license
