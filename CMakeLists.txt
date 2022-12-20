cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)


add_definitions(-DNAPI_VERSION=7)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_POSITION_INDEPENDENT_CODE on)

include_directories(${CMAKE_JS_INC})

project (native-sound-mixer)

execute_process(
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND node -p "require('node-addon-api').include_dir"
    OUTPUT_VARIABLE napi_includes
)

include_directories(${napi_includes})
option(ENABLE_SHARED "" 0)

set(
    CMAKE_LIBRARY_OUTPUT_DIRECTORY "../dist/addons/"
)

project (linux)


set(LINUX_HEADER_FILES
    "cppsrc/linux/sound-mixer.hpp"
    "cppsrc/linux/sound-mixer-utils.hpp"
    "cppsrc/linux/linux-sound-mixer.hpp"
)

set(LINUX_SOURCE_FILES
    "cppsrc/linux/main.cpp"
    "cppsrc/linux/sound-mixer.cpp"
    "cppsrc/linux/sound-mixer-utils.cpp"
    "cppsrc/linux/linux-sound-mixer.cpp"
)

add_library(${PROJECT_NAME} SHARED ${LINUX_SOURCE_FILES} ${LINUX_HEADER_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX "-sound-mixer.node")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} -lpulse)

if(GNU AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAG})
endif()

project(win)

set(WIN_HEADER_FILES
    "cppsrc/win/sound-mixer.hpp"
    "cppsrc/win/sound-mixer-utils.hpp"
    "cppsrc/win/win-sound-mixer.hpp"
)

set(WIN_SOURCE_FILES
    "cppsrc/win/main.cpp"
    "cppsrc/win/sound-mixer.cpp"
    "cppsrc/win/sound-mixer-utils.cpp"
    "cppsrc/win/win-sound-mixer.cpp"
)

add_library(${PROJECT_NAME} SHARED ${WIN_SOURCE_FILES} ${WIN_HEADER_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX "-sound-mixer.node")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAG})
endif()
