cmake_minimum_required(VERSION 3.22)
project(video_stream)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

pkg_check_modules(LIBAV REQUIRED libavcodec libavformat libavutil libswscale)

include_directories(${LIBAV_INCLUDE_DIRS})
link_directories(${LIBAV_LIBRARY_DIRS})

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})

add_executable(video_stream
        src/main.cpp
        src/ice_state_machine.cc
        src/ice_machine_states.cc
        src/video_streamer.cc
        src/video_state_machine.cc
)

target_link_libraries(video_stream ${LIBAV_LIBRARIES} ${OpenCV_LIBS})