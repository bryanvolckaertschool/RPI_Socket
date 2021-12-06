cmake_minimum_required(VERSION 3.10)
# set the project name
project(ClientServerUDP)
#add the libraries
link_libraries(libtlpi.a)
# add the client executable
add_executable(Client ud_ucase_cl.c)
# add the server executable
add_executable(Server ud_ucase_sv.c)

target_link_libraries(Server PJ_RPI)