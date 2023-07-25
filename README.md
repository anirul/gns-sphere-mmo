# gns-sphere-mmo

A simple project to test [Valve GNS library](https://github.com/ValveSoftware/GameNetworkingSockets) with a simple mmo.

## Building

To build the server part and then the client you need to follow the following procedures: 

- You need to install vcpkg;
- You need to build the server;
- You can now build the client.

### Installing VCPKG

Install VCPKG from the [github repo](https://github.com/microsoft/vcpkg/) or if you have git installed use the following command:

```pwrsh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
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
cmake .. -DCMAKE_TOOLCHAIN_FILE="[path to VCPKG]\scripts\buildsystems\vcpkg.cmake"
```

You should now have a `gns-sphere-mmo-server.sln` file in your directory that is the
launcher for the solution for windows (Visual Studio). You can now launch it
and execute the server in Visual Studio.
