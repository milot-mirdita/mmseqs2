set(compat_source_files
    sys/mman.h
    mman.cpp
    regex.c
    regex.h
)

add_library(compat ${compat_source_files})
target_link_libraries(compat)
