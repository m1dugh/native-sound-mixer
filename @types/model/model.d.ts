export declare enum DeviceType {
    RENDER = 0,
    CAPTURE = 1,
    ALL = 2
}
/** a float representing the volume (1.0 is full volume, .0 is no volume) */
export declare type VolumeScalar = number;
export declare abstract class AudioSession {
    protected readonly deviceId: string;
    readonly id: string;
    readonly path: string;
    constructor(deviceId: string, id: string, path: string);
    abstract set volume(volume: VolumeScalar);
    abstract get volume(): VolumeScalar;
    abstract get mute(): boolean;
    abstract set mute(mute: boolean);
}
export declare abstract class Device {
    readonly id: string;
    readonly name: string;
    readonly type: DeviceType;
    constructor(id: string, name: string, type: DeviceType);
    abstract get audioSessions(): AudioSession[];
    abstract get volume(): VolumeScalar;
    abstract set volume(volumeToSet: VolumeScalar);
    abstract get mute(): boolean;
    abstract set mute(mute: boolean);
}
