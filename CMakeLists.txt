cmake_minimum_required(VERSION 3.29)

project(YannEngine LANGUAGES CXX)

set(CMAKE_SUPPRESS_REGENERATION OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== Dependencies (vcpkg) =====
find_package(assimp CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)

# ===== Source Files =====
file(GLOB_RECURSE ENGINE_SRC  CONFIGURE_DEPENDS src/Engine/*.cpp)
file(GLOB_RECURSE CODEGEN_SRC CONFIGURE_DEPENDS src/CodeGen/*.cpp)
file(GLOB_RECURSE CLIENT_SRC  CONFIGURE_DEPENDS src/Client/*.cpp)

file(GLOB_RECURSE SCRIPT_SRC  CONFIGURE_DEPENDS src/Scripts/*.cpp)
list(REMOVE_ITEM SCRIPT_SRC
    "${CMAKE_SOURCE_DIR}/src/Scripts/CScriptMgr.cpp"
)

file(GLOB_RECURSE ENGINE_FX CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/Engine/*.fx
    ${CMAKE_SOURCE_DIR}/src/Engine/*.hlsl
)

# ===== Output Directories =====
set(OUTPUT_BIN_DIR     ${CMAKE_BINARY_DIR}/OutputFile/bin)
set(OUTPUT_CONTENT_DIR ${CMAKE_SOURCE_DIR}/OutputFile/content)
set(OUTPUT_SHADER_DIR  ${OUTPUT_CONTENT_DIR}/shader)

# ===== FontEngine (third-party prebuilt, NOT vcpkg) =====
set(FONTENGINE_DIR ${CMAKE_SOURCE_DIR}/External/Library/FontEngine)
message(STATUS "FONTENGINE_DIR = ${FONTENGINE_DIR}")

add_library(FW1FontWrapper SHARED IMPORTED)
set_target_properties(FW1FontWrapper PROPERTIES
    IMPORTED_IMPLIB_DEBUG            ${FONTENGINE_DIR}/FW1FontWrapper_debug.lib
    IMPORTED_LOCATION_DEBUG          ${FONTENGINE_DIR}/FW1FontWrapper.dll
    IMPORTED_IMPLIB_RELEASE          ${FONTENGINE_DIR}/FW1FontWrapper.lib
    IMPORTED_LOCATION_RELEASE        ${FONTENGINE_DIR}/FW1FontWrapper.dll
    IMPORTED_IMPLIB_RELWITHDEBINFO   ${FONTENGINE_DIR}/FW1FontWrapper.lib
    IMPORTED_LOCATION_RELWITHDEBINFO ${FONTENGINE_DIR}/FW1FontWrapper.dll
    IMPORTED_IMPLIB_MINSIZEREL       ${FONTENGINE_DIR}/FW1FontWrapper.lib
    IMPORTED_LOCATION_MINSIZEREL     ${FONTENGINE_DIR}/FW1FontWrapper.dll
)

# ===== Engine =====
add_library(Engine STATIC ${ENGINE_SRC} ${ENGINE_FX})

target_compile_definitions(Engine PUBLIC UNICODE _UNICODE)

target_include_directories(Engine PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/External/Include
    ${CMAKE_SOURCE_DIR}/External/Include/FontEngine
)

set_source_files_properties(${ENGINE_FX} PROPERTIES
    HEADER_FILE_ONLY TRUE
    VS_TOOL_OVERRIDE  "None"
)

target_link_libraries(Engine PUBLIC
    d3d12
    dxgi
    dxguid
    assimp::assimp
    Microsoft::DirectXTex
    FW1FontWrapper
)

# ===== CodeGen =====
add_executable(CodeGen ${CODEGEN_SRC})
target_include_directories(CodeGen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(CodeGen PUBLIC UNICODE _UNICODE)

# ===== CodeGen: copy exeptlist.txt next to the executable =====
add_custom_command(TARGET CodeGen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/src/CodeGen/exeptlist.txt
        $<TARGET_FILE_DIR:CodeGen>/exeptlist.txt
    COMMENT "Copying exeptlist.txt to CodeGen output directory..."
    VERBATIM
)

# ===== Generated files (CScriptMgr.h / CScriptMgr.cpp) =====
set(GENERATED_SCRIPTMGR_H   ${CMAKE_SOURCE_DIR}/src/Scripts/CScriptMgr.h)
set(GENERATED_SCRIPTMGR_CPP ${CMAKE_SOURCE_DIR}/src/Scripts/CScriptMgr.cpp)

file(GLOB SCRIPT_HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/Scripts/*.h)
list(REMOVE_ITEM SCRIPT_HEADERS "${GENERATED_SCRIPTMGR_H}")

add_custom_command(
    OUTPUT  ${GENERATED_SCRIPTMGR_H} ${GENERATED_SCRIPTMGR_CPP}
    COMMAND $<TARGET_FILE:CodeGen>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:CodeGen>
    DEPENDS CodeGen ${SCRIPT_HEADERS}
    COMMENT "Running CodeGen to generate CScriptMgr.h / CScriptMgr.cpp..."
    VERBATIM
)

set_source_files_properties(
    ${GENERATED_SCRIPTMGR_H}
    ${GENERATED_SCRIPTMGR_CPP}
    PROPERTIES GENERATED TRUE
)

# ===== Scripts =====
add_library(Scripts STATIC
    ${SCRIPT_SRC}
    ${GENERATED_SCRIPTMGR_CPP}
    ${GENERATED_SCRIPTMGR_H}
)

target_include_directories(Scripts PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Engine
    ${CMAKE_SOURCE_DIR}/src/Scripts
)

target_link_libraries(Scripts PUBLIC Engine)
add_dependencies(Scripts CodeGen)

# ===== Client =====
add_executable(Client WIN32 ${CLIENT_SRC})

target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(Client PRIVATE
    Engine
    Scripts
)

set_target_properties(Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY      ${OUTPUT_BIN_DIR}
    VS_DEBUGGER_WORKING_DIRECTORY ${OUTPUT_BIN_DIR}
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT Client
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src/Engine  PREFIX "Engine"  FILES ${ENGINE_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR}/src/Scripts PREFIX "Scripts" FILES ${SCRIPT_SRC})
source_group("Engine\\Shaders" FILES ${ENGINE_FX})
source_group("Scripts\\Generated" FILES
    ${GENERATED_SCRIPTMGR_H}
    ${GENERATED_SCRIPTMGR_CPP}
)

# ===== Post-Build =====
add_custom_command(TARGET Client POST_BUILD
    COMMENT "Post-build: copying shaders and DLLs..."

    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_SHADER_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ENGINE_FX}
        ${OUTPUT_SHADER_DIR}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:FW1FontWrapper>
        ${OUTPUT_BIN_DIR}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:assimp::assimp>
        ${OUTPUT_BIN_DIR}

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${OUTPUT_CONTENT_DIR}
        ${OUTPUT_BIN_DIR}/../content
)