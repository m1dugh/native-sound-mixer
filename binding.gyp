{
    "variables": {
        "module_name%": "SoundMixer",
        "PRODUCTION_DIR%": "./build/"
    },
    "targets": [{
        "target_name": "<(module_name)",
        "cflags!": ["-fno-exceptions"],
        "cflags_cc!": ["-fno-exceptions"],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        'libraries': [],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS'],
        'conditions': [
            [
                'OS=="win"',
                {
                    "sources": [
                        "cppsrc/win/main.cpp",
                        "cppsrc/win/source/sound-mixer-utils.cpp",
                        "cppsrc/win/source/sound-mixer.cpp"
                    ]
                }
            ]
        ]
    },
    {
        "target_name": "copy",
        "copies": [
            {
                "files": ["<(module_root_dir)/build/Release/<(module_name).node"],
                "destination": "<(module_root_dir)/dist"
            }
        ]
    }]
}
