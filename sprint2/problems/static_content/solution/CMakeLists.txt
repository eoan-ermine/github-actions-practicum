cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

set(USE_CONAN_V1 ${CONANV1})
set(USE_CONAN_V2 ${CONANV2})

unset(CONANV1 CACHE)
unset(CONANV1)
unset(CONANV2 CACHE)
unset(CONANV2)

if ((DEFINED USE_CONAN_V2) AND (USE_CONAN_V2))
else()
	# Подключаем сгенерированный скрипт conanbuildinfo.cmake, созданный Conan
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	# Выполняем макрос из conanbuildinfo.cmake, который настроит СMake на работу с библиотеками, установленными Conan
	conan_basic_setup()
endif ()

find_package(Boost 1.81.0 REQUIRED COMPONENTS program_options json log)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(src)
add_executable(game_server
	src/main.cpp
	src/http_server.cpp
	src/model/domains/map.cpp
	src/model/domains/game.cpp
	src/model/domains/api.cpp
	src/model/domains/basic.cpp
	src/util/error.cpp
	src/util/filesystem.cpp
	src/util/logging.cpp
	src/util/mime_type.cpp
	src/util/response.cpp
	src/util/ticker.cpp
	src/json_loader.cpp
	src/request_handler.cpp
)
target_link_libraries(game_server PRIVATE Threads::Threads ${Boost_LIBRARIES})
