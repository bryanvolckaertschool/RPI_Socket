cmake_minimum_required(VERSION 3.10)
# set the project name
project(http_server)
#add the libraries
link_libraries(libtlpi.a)
# add the executable
add_executable(http_server http_server.c)


target_link_libraries(http_server PJ_RPI)