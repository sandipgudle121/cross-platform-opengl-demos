clang -c -o Window.o Window.m
mkdir -p Window.app/Contents/MacOS
clang -o Window.app/Contents/MacOS/Window Window.o -framework cocoa

