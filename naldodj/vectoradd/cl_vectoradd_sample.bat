cls
g++ -I../ cl_vectoradd_sample.c C:\Windows\SysWOW64\OpenCL.dll -o cl_vectoradd_sample.exe
upx .\cl_vectoradd_sample.exe
del .\cl_vectoradd_sample.log
.\cl_vectoradd_sample.exe 1> ".\cl_vectoradd_sample.log" 2>&1
.\cl_vectoradd_sample.log