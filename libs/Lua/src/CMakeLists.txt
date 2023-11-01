project(lua)

set(SRC_MAIN
    # Sources
    lapi.c
    lcode.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    llex.c
    lmem.c
    lobject.c
    lopcodes.c
    lparser.c
    lstate.c
    lstring.c
    ltable.c
    ltests.c
    ltm.c
    lundump.c
    lvm.c
    lzio.c

    # Headers
    lapi.h
    lcode.h
    ldebug.h
    ldo.h
    lfunc.h
    lgc.h
    llex.h
    llimits.h
    lmem.h
    lobject.h
    lopcodes.h
    lparser.h
    lstate.h
    lstring.h
    ltable.h
    ltm.h
    lundump.h
    lvm.h
    lzio.h
)

set(SRC_LUA_LIB
    # Sources
    lib/lauxlib.c
    lib/lbaselib.c
    lib/ldblib.c
    lib/liolib.c
    lib/lmathlib.c
    lib/loadlib.c
    lib/lstrlib.c
    lib/ltablib.c

    # Headers
    # N/A
)

set(SRC_LUA_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/lauxlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/lua.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/lualib.h
)

add_library(lua STATIC ${SRC_MAIN} ${SRC_LUA_LIB} ${SRC_LUA_PUBLIC_HEADERS})
target_include_directories(lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
