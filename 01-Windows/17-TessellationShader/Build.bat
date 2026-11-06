cls

cl.exe /c /EHsc /I C:\glew-2.1.0\include Perspective.cpp
rc.exe Perspective.rc
link.exe  Perspective.res Perspective.obj /LIBPATH:C:\glew-2.1.0\lib\Release\x64

Perspective.exe
