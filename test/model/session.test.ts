import { random } from "lodash";
import SoundMixer, {AudioSession, Device} from "../../";

describe("audio session", () => {

	let device: Device;
	let session: AudioSession;

	beforeEach(() => {
		const devices = SoundMixer.devices.filter(({ sessions }) => sessions.length > 0);
		device = devices[random(0, devices.length - 1)]
		session = device.sessions[random(0, device.sessions.length - 1)]
	});

	describe("get volume", () => {
		it("should return a clamped volume", () => {
			expect(session.volume).toBeGreaterThanOrEqual(0);
			expect(session.volume).toBeLessThanOrEqual(1.);
		})

		it("should be consistent", () => {
			expect(session.volume).toBe(session.volume);
		})
	})

	describe("set volume", () => {
		let volume: number;
		beforeAll(() => {
			volume = session.volume
		})

		it("should not exceed 1.", () => {
			session.volume = 100;
			expect(session.volume).toBe(1);
		});

		it("should not be less than 0", () => {
			session.volume = -100;
			expect(session.volume).toBe(0);
		})


		afterAll(() => {
			session.volume = volume;
		})
	});

	describe("get mute", () => {
		it("should be consitent", () => {
			expect(session.mute).toBe(session.mute)
		});
	});

	describe("set mute", () => {
		let mute: boolean;
		beforeAll(() => {
			mute = session.mute
		})

		it("should change the mute flag", () => {
			const mute = Boolean(random(0, 1)).valueOf()
			session.mute = mute;
			expect(session.mute).toBe(mute);

			session.mute = !mute;
			expect(session.mute).toBe(!mute);
		})


		afterAll(() => {
			session.mute = mute;
		})
	});
});