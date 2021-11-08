
project( fizzbuzz )
cmake_minimum_required( VERSION 3.0 )
enable_language(C CXX ASM)
enable_testing()

add_executable( fizzbuzz.avx2 fizzbuzz.avx2.S )
set_target_properties( fizzbuzz.avx2 PROPERTIES LINKER_LANGUAGE ASM LINK_OPTIONS "-nostdlib" )

add_executable( fizzbuzz fizzbuzz.cpp )
target_link_libraries( fizzbuzz pthread )

add_executable( fizzbuzz.vanilla fizzbuzz.vanilla.cpp )

add_executable( throughput throughput.cpp )

add_custom_command( TARGET fizzbuzz.vanilla 
                    POST_BUILD 
                    BYPRODUCTS golden.txt 
                    COMMAND fizzbuzz.vanilla > golden.txt )

add_executable( fbthread fbthread.cpp )
target_link_libraries( fbthread pthread )

add_executable( fbinterleaved fbinterleaved.cpp )
target_link_libraries( fbinterleaved pthread )

add_executable( testread testread.cpp )
add_executable( testpow10 testpow10.cpp )
add_executable( testblocksize testblocksize.cpp )
