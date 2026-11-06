nvcc.exe -c -o Sinewave.cu.obj Sinewave.cu

-allow-unsupported-compiler

cl.exe /c /EHsc /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.3\include" /I C:\glew-2.1.0\include Perspective.cpp

rc.exe Perspective.rc

link.exe  Perspective.res Sinewave.cu.obj Perspective.obj /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.3\lib\x64" /LIBPATH:C:\glew-2.1.0\lib\Release\x64 

Perspective.exe
