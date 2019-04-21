cmake_minimum_required (VERSION 3.10)

project (BladeRf)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories(/usr/local/include)
include_directories(/opt/local/include)
link_directories(/usr/local/lib)
link_directories(/opt/local/lib)

add_executable(bladeRF_rt 	${PROJECT_SOURCE_DIR}/src/main.c
	${PROJECT_SOURCE_DIR}/src/tun_tap.c
	${PROJECT_SOURCE_DIR}/src/blade_rf.c
	${PROJECT_SOURCE_DIR}/src/utils.c
	${PROJECT_SOURCE_DIR}/src/ofdmflexframe.c
	
	
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

target_link_libraries(bladeRF_rt bladeRF m c liquid pthread) #fec tuntap



