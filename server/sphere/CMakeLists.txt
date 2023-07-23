# Sphere mmo server files.

add_executable(SphereMmoServer
    main.cpp
)

target_include_directories(SphereMmoServer
    PUBLIC
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(SphereMmoServer
    PRIVATE
        SphereMmoCommon
        absl::base
        absl::flags
        absl::flags_parse
        absl::strings
        GameNetworkingSockets::shared
        GameNetworkingSockets::GameNetworkingSockets
)

set_property(TARGET SphereMmoServer PROPERTY FOLDER "SphereMmoServer")
