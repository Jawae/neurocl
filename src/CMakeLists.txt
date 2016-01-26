#The MIT License
#
#Copyright (c) 2015-2016 Albert Murienne
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.

cmake_minimum_required (VERSION 3.0)
project (neurocl)

if (APPLE)
    set (extra_includes "/opt/X11/include")
    set (extra_link_paths "/usr/X11/lib")
    set (extra_link_libs X11)
endif ()

include_directories(
    .
    "${CMAKE_SOURCE_DIR}/../vexcl"
    "${CMAKE_SOURCE_DIR}/../CImg"
    ${extra_includes}
)

link_directories(${extra_link_paths})

include("${CMAKE_SOURCE_DIR}/../vexcl/cmake/opencl/FindOpenCL.cmake")

set (sources_list
samples_manager.cpp
network_file_handler.cpp
network_manager.cpp
network_vexcl.cpp
network_bnu.cpp
)

set (headers_list
samples_manager.h
network_sample.h
network_exception.h
network_file_handler.h
network_manager.h
network_interface.h
network_vexcl.h
network_bnu.h
)

add_library(neurocl SHARED ${sources_list} ${headers_list})

target_link_libraries(neurocl
boost_thread-mt
boost_system-mt
boost_chrono-mt
boost_filesystem-mt
boost_serialization-mt
${OpenCL_LIBRARIES}
${extra_link_libs}
)

target_compile_features(neurocl PRIVATE cxx_auto_type)