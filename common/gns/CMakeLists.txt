# Sphere Mmo common.

add_library(SphereMmoCommon
    STATIC
        game_networking_sockets.cpp
        game_networking_sockets.h
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

