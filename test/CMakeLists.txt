add_executable(test_sort test_sort.cpp)

find_library(TBB_LIBRARY tbb HINTS /usr/lib /usr/lib64)
find_path(TBB_INCLUDE_DIR tbb/tbb.h HINTS /usr/include)
if (NOT TBB_LIBRARY-NOTFOUND AND NOT TBB_INCLUDE_DIR-NOTFOUND)
    target_include_directories(test_sort PUBLIC ${TBB_INCLUDE_DIR})
    target_link_libraries(test_sort PUBLIC ${TBB_LIBRARY})
    target_compile_definitions(test_sort PUBLIC -DBENCH_TBB)
endif()
