cmake_minimum_required (VERSION 3.10)

project (BladeRf)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories(/usr/local/include)
include_directories(/opt/local/include)
link_directories(/usr/local/lib)
link_directories(/opt/local/lib)

add_executable(tuntap_over_bladerf 	${PROJECT_SOURCE_DIR}/src/main.c
	${PROJECT_SOURCE_DIR}/src/tuntap.c
	${PROJECT_SOURCE_DIR}/src/bladerf_configs.c
	${PROJECT_SOURCE_DIR}/src/ofdm_flexframe.c	
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

target_link_libraries(tuntap_over_bladerf bladeRF m c liquid pthread) #fec tuntap



