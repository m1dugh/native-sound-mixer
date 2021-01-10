# Windows Sound Mixer

## native sound mixer for windows

this `node.js` project is a sound mixer for node desktop apps providing control over volume for each render/capture device (I/O devices) and for each audio session in an audio device separately.

It's built on top of `windows desktop api` and uses the [windows core audio api](https://docs.microsoft.com/en-us/windows/win32/api/_coreaudio/). The native `c++` code is provided in `cppsrc/` and compiled using [node-addon-api](https://github.com/nodejs/node-addon-api)

Please rebuild before use, using `npm run build`

for documentation, please referer to their `JavaScript` implementation in `mixerWrapper.js`