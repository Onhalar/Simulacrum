**OpenGL 3D viewer**

project doesn't require any external libraries or binaries, you will however need a few tools to compile it on your system:

 * **GCC** 13.0+ or another C/C++ compiler
 * **Cmake** and **Make** if not included with your compiler on *linux*
 * **Git** for copying over the files, step can be avoided by downloading the source code directly

___

**HOW TO COMPILE**

 * navingate to a directory of your choice right click anywhre and console on Linux, type cmd into the path display on windows

 * run the following commands or the included '*.bat*' and '*.sh*' files in the '*setup/*' directory

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
cmake --build . --config Release
```
*Note: replace "MinGW Makerfiles with your generator.*
* to run the program

windows  ```."/bin/simulacrum.exe"```<br>
linux   ```./bin/simulacrum```<br>

___

It is possible that you may get shader compilation error, in which case copy the '*src/*' and '*shaders/*' folders into the '*build/*' folder.

It is also possible that the **GLFW** compiled library won't work on your system, in that case. replace the file library link in '*cmakelists.txt*' with '*glfw*'. You will however need to download the **GLFW** package on your system.

**Used Reources**

* GLAD OpenGL loader
* GLM
* ImGui
* STB image
* Nlohmann's json
* KHR (Khronos)
* GLFW
* ASSIMP (+ libZ)
* Roboto Mono font (Christian Robertson)
* FXAA implementation by McNopper (Github)