cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(ExprJIT C)

add_library(exprjit
    "src/exprjit.c"
    "src/exprjit.h"
    "src/tinyexpr-compat.h"
)
target_include_directories(exprjit PUBLIC src)
target_link_libraries(exprjit PUBLIC m)

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
