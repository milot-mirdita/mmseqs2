add_library(ryu
        common.h
        d2fixed.cpp
        d2fixed_full_table.h
        d2s_intrinsics.h
        digit_table.h
        ryu2.h
        )
set_target_properties(ryu PROPERTIES COMPILE_FLAGS ${MMSEQS_CXX_FLAGS} LINK_FLAGS ${MMSEQS_CXX_FLAGS})
