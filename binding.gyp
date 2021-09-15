{
    "variables": {"PRODUCTION_DIR%": "./windows/"},
    "conditions": [
        [
            'OS=="linux"',
            {
                "variables": {"PRODUCTION_DIR%": "./build/linux"},
                "targets": [
                    {
                        "target_name": "linux-sound-mixer",
                        "cflags!": ["-fno-exceptions"],
                        "cflags_cc!": ["-fno-exceptions"],
                        "include_dirs": [
                            "<!@(node -p \"require('node-addon-api').include\")"
                        ],
                        "libraries": ["-lpulse"],
                        "dependencies": [
                            "<!(node -p \"require('node-addon-api').gyp\")"
                        ],
                        "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
                        "sources": [
                            "cppsrc/main.cpp",
                            "cppsrc/linux/linux-sound-mixer.cpp",
                            "cppsrc/linux/sound-mixer.cpp",
                        ],
                    }
                ],
            },
        ],
        [
            'OS=="win"',
            {
                "variables": {"PRODUCTION_DIR%": "./win/"},
                "targets": [
                    {
                        "msvs_settings": {
                            "VCCLCompilerTool": {
                                "AdditionalOptions": ["-std:c++17"],
                            }
                        },
                        "target_name": "win-sound-mixer",
                        "cflags!": ["-fno-exceptions"],
                        "cflags_cc!": ["-fno-exceptions"],
                        "include_dirs": [
                            "<!@(node -p \"require('node-addon-api').include\")"
                        ],
                        "libraries": [],
                        "dependencies": [
                            "<!(node -p \"require('node-addon-api').gyp\")"
                        ],
                        "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
                        "sources": [
                            "cppsrc/main.cpp",
                            "cppsrc/win/win-sound-mixer.cpp",
                            "cppsrc/win/sound-mixer.cpp",
                        ],
                    }
                ],
            },
        ],
    ],
}
