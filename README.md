# gns-sphere-mmo

A simple project to test [Valve GNS library](https://github.com/ValveSoftware/GameNetworkingSockets) with a simple mmo.

## Building

To build the server part and then the client you need to follow the following procedures: 

- You need to install vcpkg;
- You need to create a new triplets for the client to be linked;
- You need to build the server;
- You can now build the client.

### Installing VCPKG

Install VCPKG from the [github repo](https://github.com/microsoft/vcpkg/) or if you have git installed use the following command:

```pwrsh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
```

### Add a new triplet

To make it link with Unreal Engine you will need to pack everything in the
`.lib`. To achieve this you need a new triplet that will tell the system to
build everything in static EXCEPT the CRT.

Add a new file to the triplets directory in the vcpkg directory (that you
just created above) and name it `x64-windows-static-md.cmake`. This file
should contain the following:

```cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
```

### Create the server

Now that you have everything needed you can build the game server! First
download the repo to a local folder. Then create a directory for the build
you are going to perform, then build it!

```pwrsh
git clone https://github.com/anirul/gns-sphere-mmo.git
cd gns-sphere-mmo
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="[path to VCPKG]\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
```

You should now have a `gns-sphere-mmo-server.sln` file in your directory that is the
launcher for the solution for windows (Visual Studio). You can now launch it
and execute the server in Visual Studio.
