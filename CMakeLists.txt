cmake_minimum_required(VERSION 3.20)

project(SphereMmo DESCRIPTION "GNS Sphere mmo" LANGUAGES CXX)

# C++ standatd version.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adding subfolder property.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# External packages.
find_package(absl CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)

# Sources.
add_subdirectory(server/sphere)
add_subdirectory(common/gns)
