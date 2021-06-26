const fs = require("fs");
const path = require("path");

const ROOT_DIR = path.resolve(__dirname, "..");

const SRC_DIR = path.resolve(ROOT_DIR, "build", "Release");
const DEST_DIR = path.resolve(ROOT_DIR, "dist", "addons");

const NATIVE_FILES = fs.readdirSync(SRC_DIR)
	.filter((file) => file.split(".").pop() === "node")
	.map((file) => ({
		src: path.resolve(SRC_DIR, file),
		dest: path.resolve(DEST_DIR, file)
	}))

try {
	fs.rmdirSync(DEST_DIR, { recursive: true });
} catch (err) {
	console.error(err)
}
fs.mkdirSync(DEST_DIR, { recursive: true });


for (const { src, dest } of NATIVE_FILES) {
	try {
		fs.copyFileSync(src, dest, fs.constants.COPYFILE_FICLONE);
		console.log(`successfully copied ${src.replace(/\\/g, "/").split("/").pop()}`)
	} catch (err) {
		throw new Error(`missing file ${src}`)
	}
}