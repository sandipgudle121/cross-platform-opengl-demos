
cl.exe /c /EHsc /I C:\glew-2.1.0\include main.cpp
rc.exe main.rc
link.exe  main.res main.obj /LIBPATH:C:\glew-2.1.0\lib\Release\x64

main.exe

