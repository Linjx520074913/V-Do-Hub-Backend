cmake_minimum_required(VERSION 3.8)

project(V-DoHub-Backend)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE "ReleaseWithDebugInfo")

if(APPLE)
    set(MACOSX_RPATH true)
    # use, i.e. don't skip the full RPATH for the build tree
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    
    # when building, don't use the install RPATH already
    # (but later on when installing)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
    
    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    execute_process(COMMAND brew --prefix libomp
        OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(OpenMP_C_FLAGS
            "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
    set(OpenMP_CXX_FLAGS
            "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
    set(OpenMP_C_LIB_NAMES omp)
    set(OpenMP_CXX_LIB_NAMES omp)
    set(OpenMP_omp_LIBRARY ${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

# 查找 OpenMP
find_package(OpenMP REQUIRED)

# 将 OpenMP 选项添加到编译器标志中
if(OpenMP_CXX_FOUND)
    message(status "OpenMP found")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    if(APPLE)
        set(OPENMP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/deps/libomp/lib/libomp.a")
        set(OPENMP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/libomp/include")
    else()
        set(OPENMP_LIB OpenMP::OpenMP_CXX)
    endif()
else()
    message(FATAL_ERROR "OpenMP not found")
# target_link_libraries(MyTarget PUBLIC OpenMP::OpenMP_CXX)
endif()

# set unicode charset
add_definitions(-DUNICODE -D_UNICODE)

set(OB_XW_WRAPPER_INCLUDE_DIR ./deps/ob-xw-wrapper/include)

set(OPENCV_INCLUDE ./deps/opencv/release/include)
set(OPENCV_LIB ./deps/opencv/release/x64/vc15/lib)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /level='requireAdministrator'")
    set(OB_XW_WRAPPER_LIBS_DIR ./deps/ob-xw-wrapper/lib/windows)
    set(DBGHELP_LIB DbgHelp)
elseif(UNIX AND NOT APPLE)

elseif(APPLE)

endif()

include_directories(
    ./include
    ${OB_XW_WRAPPER_INCLUDE_DIR}
    ${OPENCV_INCLUDE}
)
link_directories(
    ${OB_XW_WRAPPER_LIBS_DIR}
    ${OPENCV_LIB}
)

file(GLOB_RECURSE SRC "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(
    ${PROJECT_NAME}
    ob-xw-wrapper-lib
    ${DBGHELP_LIB}
    opencv_world455
)

if(APPLE)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        BUILD_RPATH "${CMAKE_INSTALL_RPATH}"
        INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
    )
endif()