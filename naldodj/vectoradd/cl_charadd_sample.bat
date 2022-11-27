cls
g++ -I../ cl_charadd_sample.c C:\Windows\SysWOW64\OpenCL.dll -o cl_charadd_sample.exe
upx .\cl_charadd_sample.exe
del .\cl_charadd_sample.log
.\cl_charadd_sample.exe 1> ".\cl_charadd_sample.log" 2>&1
.\cl_charadd_sample.log