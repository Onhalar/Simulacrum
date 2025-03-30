**OpenGL 3D viewer**

project doesn't require any external libraries or binaries, you will however need a few tools:

 * **GCC** 13.0+ or another C/C++ compiler
 * **Cmake** and **Make** if not included with your compiler
 * **Git** for copying over the files, step can be avoided by downloading the source code directly

___

**HOW TO COMPILE**

 * navingate to a directory of your choice right click anywhre and console on Linux, type cmd into the path display on windows

 * run the following commands or the included '*.bat*' and '*.sh*' files

*Linux*
```
git clone https://github.com/Onhalar/3DviewerOpenGL.git
cd 3DviewerOpenGL
mkdir build
cd build
cmake ..
make
```
*Windows*
```
git clone https://github.com/Onhalar/3DviewerOpenGL.git
cd 3DviewerOpenGL
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```
*Note: replac e"MinGW Makerfiles with your generator.*
* to run the program

windows ```."/bin/simulacrum.exe"```<br>
linux   ```./bin/simulacrum```<br>

___

It is possible that you may get shader compilation error, in which case copy the '*src/*' and '*shaders/*' folders into the '*build/*' folder.

