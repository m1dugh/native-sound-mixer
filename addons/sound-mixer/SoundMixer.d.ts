export function GetDefaultDevice(dataFlow: number): {id: string, name: string, render: boolean};
export function GetDevices(): {id: string, name: string, render: boolean}[];
export function GetSessions(deviceId: string): any[];

export function SetEndpointMute(deviceId: string, mute: boolean): boolean;
export function GetEndpointMute(deviceId: string): boolean;
export function SetEndpointVolume(deviceId: string, volume: number): number;
export function GetEndpointVolume(deviceId: string): number;

export function SetAudioSessionMute(deviceId: string, processId: number, mute: boolean): boolean;
export function GetAudioSessionMute(deviceId: string, processId: number): boolean;
export function SetAudioSessionVolume(deviceId: string, processId: number, volume: number): number;
export function GetAudioSessionVolume(deviceId: string, processId: number): number;

