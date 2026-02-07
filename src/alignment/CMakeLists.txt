set(alignment_header_files
        alignment/Alignment.h
        alignment/CompressedA3M.h
        alignment/EvalueComputation.h
        alignment/Matcher.h
        alignment/MsaFilter.h
        alignment/MultipleAlignment.h
        alignment/PSSMCalculator.h
        alignment/StripedSmithWaterman.h
        alignment/BandedNucleotideAligner.h
        alignment/DistanceCalculator.h
        alignment/Fwbw.h
        alignment/UngappedAligner.h
        PARENT_SCOPE
        )

set(alignment_source_files
        alignment/Alignment.cpp
        alignment/CompressedA3M.cpp
        alignment/Main.cpp
        alignment/Matcher.cpp
        alignment/MsaFilter.cpp
        alignment/MultipleAlignment.cpp
        alignment/PSSMCalculator.cpp
        alignment/StripedSmithWaterman.cpp
        alignment/BandedNucleotideAligner.cpp
        alignment/DistanceCalculator.cpp
        alignment/rescorediagonal.cpp
        alignment/Fwbw.cpp
        alignment/UngappedAlignerDefault.cpp
        )

if (X86 OR X64)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-mavx512f -mavx512bw -mavx512dq" HAVE_AVX512_UNGAPPED_FLAGS)
        if (HAVE_AVX512_UNGAPPED_FLAGS)
                set(ungapped_aligner_avx512_src ${CMAKE_CURRENT_SOURCE_DIR}/UngappedAlignerAVX512.cpp)
                list(APPEND alignment_source_files ${ungapped_aligner_avx512_src})
                set_source_files_properties(
                        ${ungapped_aligner_avx512_src}
                        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
                        PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512bw -mavx512dq"
                )
        endif ()
endif ()

set(alignment_source_files ${alignment_source_files} PARENT_SCOPE)
