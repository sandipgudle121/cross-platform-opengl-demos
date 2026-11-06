
cl.exe /c /EHsc /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.3\include" /I C:\glew-2.1.0\include Perspective.cpp
rc.exe Perspective.rc
link.exe  Perspective.res Perspective.obj /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.3\lib\x64" /LIBPATH:C:\glew-2.1.0\lib\Release\x64 

Perspective.exe