cmake_minimum_required(VERSION 3.16)
project(sconnman-example
    VERSION 0.0.1
    LANGUAGES C CXX
)

set(EYEAPP_MESSAGE_LOG "🌐 ${PROJECT_NAME}: ")

message(${EYEAPP_MESSAGE_LOG} "PROJECT_VERSION=${PROJECT_VERSION}")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} 
    PRIVATE src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE sconnman
)

target_link_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

message(${EYEAPP_MESSAGE_LOG} "🛠")
