@set PATCHVER=0,0,1,0
@set PATCHVERSTR="0.0.1.0"
@rc /dPATCHVER=%PATCHVER% /dXXLVER=%XXLVER% resource.rc
@cl %* /c /DPATCHVERSTR=%PATCHVERSTR% /DXXLVER=%XXLVER% /LD /EHsc *.cpp
@link /DLL /DEF:d3d9.def /OUT:d3d9.dll *.obj resource.res user32.lib version.lib inih/inih.lib