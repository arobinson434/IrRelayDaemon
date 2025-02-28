cmake_minimum_required(VERSION 3.16)

project(IrRelayDaemon VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(
    PROTO_SRC PROTO_HDR
    messages/presence_notification.proto
    messages/ir_command.proto
)

set(IRD_SRCS
    "main.cpp"
    "${PROTO_SRC}"
    "services/presence_notifier.cpp"
    "services/learning_service.cpp"
    "services/listening_service.cpp"
)

add_executable(IrRelayDaemon ${IRD_SRCS})
target_include_directories(IrRelayDaemon PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(IrRelayDaemon PRIVATE ${PROTOBUF_LIBRARY} gpiodcxx)

