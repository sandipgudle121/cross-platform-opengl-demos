

g++ -c -o OGL.o -I /usr/include OGL.cpp 

g++ -o OGL OGL.o -L /usr/lib/x86_64-linux-gnu -L . -lX11 -lGL -lGLEW -lSphere


export LD_LIBRARY_PATH=LD_LIBRARY_PATH:./

