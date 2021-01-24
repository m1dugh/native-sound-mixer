import {copyFileSync} from 'fs';
import {resolve, dirname} from 'path';
import {fileURLToPath} from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const src = resolve(__dirname, "../build/Release/SoundMixer.node");
const dest = resolve(__dirname, "../addons/sound-mixer/SoundMixer.node");

try {
	copyFileSync(src, dest);
	console.log("successfully copied bin file to addons/sound-mixer");
}catch(err) {
	throw err;
}

