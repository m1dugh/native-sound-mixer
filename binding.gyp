{
    "variables": {
        "module_name%": "sound-mixer",
        "PRODUCTION_DIR%": "./build/"
    },
    "targets": [{
        "target_name": "<(module_name)-win",
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
        "sources": [
            "cppsrc/main.cpp",
            "cppsrc/win/sound-mixer-utils.cpp",
            "cppsrc/win/sound-mixer.cpp"
        ]
    },
    ]
}
