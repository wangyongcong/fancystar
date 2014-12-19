@echo off
if "%1"=="" (
	echo qt-moc [目录名]
	goto :END
)
echo 正在执行MOC编译...
for /r .\%1 %%I in (*.h) do (
	echo making %%~dpnI_moc.cpp...
	E:\Qt\Qt5.2.1\5.2.1\msvc2010_opengl\bin\moc.exe -nn %%I -o %%~dpnI_moc.cpp
)
echo MOC编译完毕
:END
