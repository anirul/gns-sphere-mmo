# Sphere mmo client files.

add_executable(SphereMmoClient
    main.cpp
    sphere_mmo_client.cpp
    sphere_mmo_client.h
)

target_include_directories(SphereMmoClient
  PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(SphereMmoClient
    PRIVATE
        SphereMmoCommon
        absl::base
        absl::flags
        absl::flags_parse
        absl::strings
        GameNetworkingSockets::shared
        GameNetworkingSockets::GameNetworkingSockets
)

set_property(TARGET SphereMmoClient PROPERTY FOLDER "SphereMmoClient")
