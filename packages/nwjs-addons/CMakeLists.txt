cmake_minimum_required(VERSION 2.8)
project(nwjs_addons)

# XP-compatible settings for VS2015
if(MSVC)
  # Use static runtime for single-binary distribution
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

  # Disable incremental linking (XP compat)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")

  # Target XP subsystem (5.01 = XP)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.01")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.01")
endif()

# Include cmake-js paths (includes node headers, v8, uv, and nan)
include_directories(${CMAKE_JS_INC})

# addon_api.h abstraction layer — include src/ so all addons can find it
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# Backend selection: USE_NAPI for node-addon-api, USE_NAN (default) for NAN
if(USE_NAPI)
  add_definitions(-DUSE_NAPI=1)
  add_definitions(-DNAPI_DISABLE_CPP_EXCEPTIONS)

  # node-addon-api C++ wrapper headers
  execute_process(COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
  include_directories(${NODE_ADDON_API_DIR})
else()
  add_definitions(-DUSE_NAN=1)
endif()

# Source files - start with just module.cpp, add addons incrementally
set(SOURCES src/module.cpp)

# Clipboard addon
file(GLOB CLIPBOARD_SRC "clipboard/src/*.cpp")
list(APPEND SOURCES ${CLIPBOARD_SRC})

# Folder-dialog addon
file(GLOB FOLDER_DIALOG_SRC "folder-dialog/src/*.cpp")
list(APPEND SOURCES ${FOLDER_DIALOG_SRC})

# IPC addon
file(GLOB IPC_SRC "ipc/src/*.cpp")
list(APPEND SOURCES ${IPC_SRC})

# TinyCC addon — jsbridge abstracted for both NAN and N-API
file(GLOB TINYCC_SRC "tinycc/src/*.cpp")
list(APPEND SOURCES ${TINYCC_SRC})
include_directories("tinycc/src")
include_directories("tinycc/include")
# Vendor lib directory: x86 or x64 based on pointer size
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/tinycc/lib/x64")
else()
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/tinycc/lib/x86")
endif()

# Call-DLL addon
file(GLOB CALLDLL_SRC "call-dll/src/*.cpp")
list(APPEND SOURCES ${CALLDLL_SRC})

# nw-sqlite3 addon
# SQLite compile options for minimal size and XP compatibility
add_definitions(
  -DSQLITE_THREADSAFE=1
  -DSQLITE_ENABLE_JSON1=1
  -DSQLITE_OMIT_DEPRECATED=1
  -DSQLITE_DQS=0
  -DSQLITE_DEFAULT_MEMSTATUS=0
  -DSQLITE_OMIT_PROGRESS_CALLBACK=1
)
set(SQLITE_SRC "nw-sqlite3/src/sqlite3.c")
file(GLOB SQLITE3_CPP_SRC "nw-sqlite3/src/*.cpp")
list(APPEND SOURCES ${SQLITE_SRC} ${SQLITE3_CPP_SRC})
include_directories("nw-sqlite3/src")

# CSV Parser addon
file(GLOB CSVPARSER_SRC "csv-parser/src/*.cpp")
list(APPEND SOURCES ${CSVPARSER_SRC})

# RSS Parser addon
file(GLOB RSSPARSER_SRC "rss-parser/src/*.cpp")
list(APPEND SOURCES ${RSSPARSER_SRC})

# SDL2 Input addon — vendor lib per architecture
file(GLOB SDL2INPUT_SRC "sdl2-input/src/*.cpp")
list(APPEND SOURCES ${SDL2INPUT_SRC})
include_directories("sdl2-input/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl2-input/vendor/SDL2/include")
# Vendor lib directory: x86 or x64 based on pointer size
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl2-input/vendor/SDL2/lib/x64")
else()
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl2-input/vendor/SDL2/lib/x86")
endif()

# Single combined addon
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# .node extension
set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)

# Link cmake-js lib
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

# Windows libraries
if(WIN32)
  target_link_libraries(${PROJECT_NAME}
    Shlwapi.lib
    Shell32.lib
    Ole32.lib
    Gdiplus.lib
    User32.lib
    winmm.lib
    imm32.lib
    version.lib
    setupapi.lib
  )
  target_link_libraries(${PROJECT_NAME} libtcc.lib SDL2-static.lib)
endif()
