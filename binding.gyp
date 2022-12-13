{
    "conditions": [
        [
            'OS=="linux"',
            {
                "targets": [
                    {
                        "target_name": "linux-sound-mixer",
                        "cflags": ["-fexceptions"],
                        "cflags_cc": ["-fexceptions", "-std=c++17"],
                        "include_dirs": [
							"<!(node -p \"require('node-addon-api').include_dir\")"
						],
                        "libraries": ["-lpulse"],
                        "defines": ["NAPI_CPP_EXCEPTIONS"],
                        "sources": [
							"cppsrc/main.cpp",
                            "cppsrc/shared/sound-mixer-utils.cpp",
                            "cppsrc/linux/sound-mixer.cpp",
                            "cppsrc/linux/linux-sound-mixer.cpp"
                        ],
                    }
                ],
            },
        ],
        [
            'OS=="win"',
            {
                "targets": [
                    {
                        "msvs_settings": {
                            "VCCLCompilerTool": {
                                "AdditionalOptions": ["-std:c++17"],
								"ExceptionHandling": 1
                            }
                        },
                        "target_name": "win-sound-mixer",
                        "include_dirs": [
                            "<!(node -p \"require('node-addon-api').include_dir\")"
                        ],
                        "ldflags": [
                            "-Wl,-z,defs"
                        ],
                        "libraries": [],
                        "defines": ["NAPI_CPP_EXCEPTIONS", "_HAS_EXCEPTIONS=1"],
                        "sources": [
							"cppsrc/main.cpp",
                            "cppsrc/shared/sound-mixer-utils.cpp",
                            "cppsrc/win/sound-mixer.cpp",
                            "cppsrc/win/win-sound-mixer.cpp"
                        ],
                    }
                ],
            },
        ],
    ],
}
