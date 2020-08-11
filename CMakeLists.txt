cmake_minimum_required(VERSION 3.5)
project(httpserver)

set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS
        -Wall
        -g
        -std=c++11
	-lpthread
        -Wno-unused-parameter
        -O3
        )

link_libraries(pthread)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

set(SOURCE_FILES
        http/HttpRequest.cpp
        http/HttpData.cpp
        http/HttpParse.cpp
        http/HttpResponse.cpp
        socket/Socket.cpp
        threadpool/ThreadPool.cpp
        Util/Timer.cpp
        Util/Util.cpp
        Epoll/Epoll.cpp
        config/Config.cpp
	webserver/Server.cpp
        main.cpp
        )

add_executable(httpserver  ${SOURCE_FILES})
