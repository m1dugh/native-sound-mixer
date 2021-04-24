import { Device, AudioSession, DeviceType } from "./model/model";
export default class SoundMixer {
    static get devices(): Device[];
    static getDefaultDevice(type: DeviceType): Device;
}
export { Device, AudioSession, DeviceType };
