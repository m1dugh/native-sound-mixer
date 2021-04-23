
export enum DeviceType {
	RENDER = 0,
	CAPTURE = 1,
	ALL = 2
}

/** a float representing the volume (1.0 is full volume, .0 is no volume) */
export type VolumeScalar = number;

export abstract class AudioSession {

	constructor(private readonly deviceId: string, public readonly id: string, public readonly path: string) { }

	abstract setVolume(volume: VolumeScalar): void;
	abstract getVolume(): VolumeScalar;
	abstract changeVolume(deltaVolume: VolumeScalar): VolumeScalar;

	abstract getMute(): boolean;
	abstract setMute(mute: boolean);
	abstract toggleMute(): boolean;

}

export abstract class Device {
	constructor(public readonly id: string, public readonly name: string, public readonly type: DeviceType) { }

	abstract getAudioSessions(): AudioSession[];

	abstract getVolume(): VolumeScalar;
	abstract setVolume(volumeToSet: VolumeScalar): void;
	abstract changeVolume(deltaVolume: VolumeScalar): VolumeScalar;

	abstract getMute(): boolean;
	abstract setMute(mute: boolean);
	abstract toggleMute(): boolean;
}