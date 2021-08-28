const path = require("path");
const { existsSync } = require("fs");

const ADDONS_DIR = path.resolve(__dirname, "..", "dist", "addons");


const binaryExists = (platform, arch = undefined) => existsSync(path.resolve(ADDONS_DIR, `${platform}-sound-mixer${arch ? "_" + arch : ""}.node`));

const supportedDependencies = {
	"win": [undefined],
	"linux": [undefined]
}

for (let platform of Object.keys(supportedDependencies)) {
	for (let arch of supportedDependencies[platform]) {
		if (!binaryExists(platform, arch)) {
			throw new Error(`missing binary for ${platform}${arch ? '-' + arch : ''}`)
		}
	}
}