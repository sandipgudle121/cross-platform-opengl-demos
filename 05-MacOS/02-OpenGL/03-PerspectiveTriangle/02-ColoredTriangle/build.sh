
clang++ -Wno-deprecated-declarations -c -o OGL.o OGL.mm
mkdir -p OGL.app/Contents/MacOS
clang++ -o OGL.app/Contents/MacOS/OGL OGL.o -framework cocoa -framework QuartzCore -framework OpenGL
