cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.82.0 REQUIRED COMPONENTS json)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server
	src/main.cpp
	src/http_server.cpp
	src/http_server.hpp
	src/sdk.hpp
	src/model.hpp
	src/model.cpp
	src/tagged.hpp
	src/json_loader.hpp
	src/json_loader.cpp
	src/json_serializer.hpp
	src/json_serializer.cpp
	src/request_handler.cpp
	src/request_handler.hpp
)
target_link_libraries(game_server PRIVATE Threads::Threads ${Boost_LIBRARIES})
