set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -O3")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
find_package(MKL REQUIRED)
include_directories(${MKL_INCLUDE_DIR})
find_package(MPI REQUIRED)
include_directories(${MPI_C_INCLUDE_PATH})

set(SOURCE_FILES Board.c DistributedBoard.c runSeq.c runOMP.c runPthreads.c runMpiSeq.c runMpiOMP.c utils.c utils.h Timer.c Timer.h runFuncs.h)
add_library(liblife SHARED ${SOURCE_FILES})
target_link_libraries(liblife ${MKL_LIBRARIES} ${MPI_C_LIBRARIES} ${HWLOC_LIBRARIES} m)
set_property(TARGET liblife PROPERTY C_STANDARD 99)






