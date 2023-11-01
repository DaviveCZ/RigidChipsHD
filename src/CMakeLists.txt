project(RigidChipsHD)
set(ProjectOutDir "${CMAKE_SOURCE_DIR}/game")

set(CMAKE_CXX_STANDARD 98)

set(SRC_MAIN
    # Sources
    Rigidmain.cpp
    d3dapp.cpp
    d3dfile.cpp
    d3dfont.cpp
    d3dutil.cpp
    diutil.cpp
    dmutil.cpp
    dsutil.cpp
    dxutil.cpp
    GRigid.cpp
    GSocket.cpp
    GVector.cpp
    lua_5_3_compat.c
    luaScript.cpp
    luaSub.cpp
    luaSystem.cpp
    #mathf.c
    readData.cpp
    readLand.cpp
    readSub.cpp
    script.cpp

    # Headers
    Rigidmain.h
    d3dapp.h
    d3dfile.h
    d3dfont.h
    d3dres.h
    d3dutil.h
    diutil.h
    dmutil.h
    dsutil.h
    dxutil.h
    GDPlay.hpp
    GLand.hpp
    GParticle.hpp
    GPlayers.h
    GRigid.hpp
    GSocket.hpp
    GVector.hpp
    Land.h
    lua_5_3_compat.h
    luaScript.hpp
    luaSub.hpp
    luaSystem.hpp
    readData.hpp
    readLand.hpp
    readSub.hpp
)

add_executable(RigidChipsHD WIN32 ${SRC_MAIN})

# DirectX
target_include_directories(RigidChipsHD PRIVATE ${CMAKE_SOURCE_DIR}/libs/dx9sdk/include)
if (CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
    target_link_directories(RigidChipsHD PRIVATE ${CMAKE_SOURCE_DIR}/libs/dx9sdk/lib/x64)
else()
    target_link_directories(RigidChipsHD PRIVATE ${CMAKE_SOURCE_DIR}/libs/dx9sdk/lib/x86)
endif()
target_link_libraries(RigidChipsHD PRIVATE dinput8 dxerr8 d3dx8dt d3d8 d3dxof dxguid)
target_link_libraries(RigidChipsHD PRIVATE dsound imm32 winmm legacy_stdio_definitions)

# Lua
target_link_libraries(RigidChipsHD PRIVATE lua)

# Winsock
target_link_libraries(RigidChipsHD PRIVATE ws2_32)

add_dependencies(RigidChipsHD lua)

# Set output directory
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_DEBUG "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELEASE "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${ProjectOutDir}")
set_property(TARGET ${PROJECT_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${ProjectOutDir}")

# Set output name
if (DEFINED CUSTOM_SUFFIX)
    set(CUSTOM_SUFFIX_REAL _${CUSTOM_SUFFIX})
else()
    set(CUSTOM_SUFFIX_REAL "")
endif()
if (CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_RELEASE ${PROJECT_NAME}64${CUSTOM_SUFFIX_REAL})
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_DEBUG ${PROJECT_NAME}64_DEBUG${CUSTOM_SUFFIX_REAL})
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_MINSIZEREL ${PROJECT_NAME}64_MINSIZEREL${CUSTOM_SUFFIX_REAL})
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_RELWITHDEBINFO ${PROJECT_NAME}64_RELWITHDEBINFO${CUSTOM_SUFFIX_REAL})
else()
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_RELEASE ${PROJECT_NAME}${CUSTOM_SUFFIX_REAL})
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_DEBUG ${PROJECT_NAME}_DEBUG${CUSTOM_SUFFIX_REAL})
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_MINSIZEREL ${PROJECT_NAME}_MINSIZEREL${CUSTOM_SUFFIX_REAL})
    set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_RELWITHDEBINFO ${PROJECT_NAME}_RELWITHDEBINFO${CUSTOM_SUFFIX_REAL})
endif()