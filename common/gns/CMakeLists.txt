# Sphere Mmo common.

add_library(SphereMmoCommon
    STATIC
        game_networking_sockets.cpp
        game_networking_sockets.h
        sphere_client.cpp
        sphere_client.h
)

target_include_directories(SphereMmoCommon
  PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(SphereMmoCommon
    PRIVATE
        absl::base
        absl::flags
        absl::flags_parse
        absl::strings
        GameNetworkingSockets::shared
        GameNetworkingSockets::GameNetworkingSockets
)

set_property(TARGET SphereMmoCommon PROPERTY FOLDER "SphereMmoCommon")

