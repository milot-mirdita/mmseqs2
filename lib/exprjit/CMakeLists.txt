cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(ExprJIT C)

option(EXPRJIT_REGENERATE_DASC "Regenerate DynASM outputs" OFF)
if (EXPRJIT_REGENERATE_DASC)
    find_program(LUA_EXECUTABLE luajit)
    if(NOT LUA_EXECUTABLE)
        message("-- LuaJIT executable not found")
    else()
        set(DASM_DEFS "")
        if (WIN32 AND NOT CYGWIN)
            set(DASM_DEFS "-DWIN64")
        endif()

        add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/jit_amd64.c
                COMMAND ${LUA_EXECUTABLE} ./dynasm/dynasm.lua --nolineno ${DASM_DEFS} -o jit_amd64.c jit_amd64.dasc
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/jit_amd64.dasc)
        add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/jit_arm64.c
                COMMAND ${LUA_EXECUTABLE} ./dynasm/dynasm.lua --nolineno -o jit_arm64.c jit_arm64.dasc
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/jit_arm64.dasc)
        add_custom_target(generated_dasc DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/jit_amd64.c ${CMAKE_CURRENT_SOURCE_DIR}/src/jit_arm64.c)
    endif()
endif()

add_library(exprjit
    "src/exprjit.c"
    "src/exprjit.h"
    "src/tinyexpr-compat.h"
)
target_include_directories(exprjit PUBLIC src)
target_link_libraries(exprjit PUBLIC m)
if (EXPRJIT_REGENERATE_DASC AND LUA_EXECUTABLE)
    add_dependencies(exprjit generated_dasc)
endif()

include(CheckCSourceCompiles)
check_c_source_compiles("
    int main() {
        void *pLabel = &&label;
        goto *pLabel;
        return 1;
        label:
        return 0;
    }"
    HAVE_COMPUTED_GOTO)
if (HAVE_COMPUTED_GOTO)
    target_compile_definitions(exprjit PUBLIC -DHAVE_COMPUTED_GOTO=1)
endif ()
