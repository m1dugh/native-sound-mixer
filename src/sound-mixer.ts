import {
	Device,
	AudioSession,
	DeviceType,
	VolumeScalar
} from "./model/model";

const sMixerModule = require(`${__dirname}/SoundMixer.node`)

class AudioSessionImpl extends AudioSession {

	get mute() {
		return sMixerModule.GetAudioSessionMute(this.deviceId, this.id);
	}

	get volume() {
		return sMixerModule.GetAudioSessionVolume(this.deviceId, this.id);
	}

	set mute(mute: boolean) {
		sMixerModule.SetAudioSessionMute(this.deviceId, this.id, mute);
	}

	set volume(volume: VolumeScalar) {
		sMixerModule.SetAudioSessionVolume(this.deviceId, this.id, volume);
	}

}

class DeviceImpl extends Device {

	get audioSessions() {
		console.log(sMixerModule.GetSessions())
		sMixerModule.GetSessions(this.id).forEach(console.log)
		return sMixerModule.GetSessions(this.id).map(({id, path}: {id: string, path: string}) => new AudioSessionImpl(this.id, id, path));
	}

	get mute() {
		return sMixerModule.GetEndpointMute(this.id);
	}

	get volume() {
		return sMixerModule.GetEndpointVolume(this.id);
	}

	set mute(mute: boolean) {
		sMixerModule.SetEndpointMute(this.id, mute);
	}

	set volume(volume: VolumeScalar) {
		sMixerModule.SetEndpointVolume(this.id, volume);
	}

}

export default class SoundMixer {
	
	static get devices(): Device[] {
		return sMixerModule.GetDevices().map(({id, name, type}: {id: string, name: string, type: number}) => new DeviceImpl(id, name, type))
	}

	static getDefaultDevice(type: DeviceType): Device {
		const {id, name, type: nativeType} = sMixerModule.GetDefaultDevice(type);
		return new DeviceImpl(id, name, nativeType);
	}

}

export {
	Device,
	AudioSession,
	DeviceType
}