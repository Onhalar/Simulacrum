**OpenGL 3D viewer**

project doesn't require any external libraries or binaries, you will however require '*cmake*' and any requirements bound with it.

___

**HOW TO COMPILE**

 * get '*git*' for your respective operating system

Windows '*https://git-scm.com/downloads/win*'
debian ```sudo apt install git-all```
RPM ```sudo dnf install git-all```

 * navingate to a directory of your choice right click anywhre and console on Linux, type cmd into the path display on windows

 * run 
```
git clone https://github.com/Onhalar/3DviewerOpenGL.git
cd 3DviewerOpenGL
mkdir build
cd build
cmake ..
make
```

* to run the program

windows ```./bin/simulacrum.exe```
linux   ```./bin/simulacrum```

___

It is possible that you may get shader compilation error, in which case copy the '*src/*' and '*shaders*' folders into the '*build*' folder.

