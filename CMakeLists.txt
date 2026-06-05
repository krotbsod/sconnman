cmake_minimum_required(VERSION 3.16)
project(sconnman
    VERSION 0.0.1
    LANGUAGES C CXX
)

set(BUILD_EXAMPLE OFF)

set(MESSAGE_LOG "🌐 ${PROJECT_NAME}: ")

message(${MESSAGE_LOG} "PROJECT_VERSION=${PROJECT_VERSION}")

add_library(${PROJECT_NAME} STATIC)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_sources(${PROJECT_NAME} 
    PRIVATE src/core/netlink.cpp
    PRIVATE src/utils/command_utils.cpp
    PRIVATE src/components/interface.cpp
    PRIVATE src/components/address.cpp
    PRIVATE src/components/route.cpp
    PRIVATE src/components/connection.cpp
    PRIVATE src/concrete/wired_connection.cpp
    PRIVATE src/concrete/udhcpc_dhcp_client.cpp
    PRIVATE src/managers/interface_manager.cpp
    PRIVATE src/managers/address_manager.cpp
    PRIVATE src/managers/route_manager.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC include/
)

target_link_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_INSTALL_LIBDIR}
)

if(BUILD_EXAMPLE)
    add_subdirectory(example example/)
endif()

install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

message(${MESSAGE_LOG} "🛠")
