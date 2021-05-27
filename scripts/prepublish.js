const fs = require("fs");
const path = require("path");

const ROOT_DIR = path.resolve(__dirname, "..");


const NATIVE_FILES = ["win", "linux", "macos"].map((platform) => ({
	src: path.resolve(ROOT_DIR, "build", "Release", `sound-mixer-${platform}.node`),
	dest: path.resolve(ROOT_DIR, "dist", "addons", `sound-mixer-${platform}.node`)
}));

try {
	fs.rmdirSync(path.resolve(ROOT_DIR, "dist", "addons"), { recursive: true });
} catch (err) {
	console.error(err)
}
fs.mkdirSync(path.resolve(ROOT_DIR, "dist", "addons"), { recursive: true });


for (const { src, dest } of NATIVE_FILES) {
	try {
		fs.copyFileSync(src, dest, fs.constants.COPYFILE_FICLONE);
	} catch (err) {
		throw new Error(`missing file ${src}`)
	}
}