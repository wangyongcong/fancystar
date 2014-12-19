:: qt-moc-file [FilePath] [FileBaseName]
@echo off
echo qt-moc %1...
E:\Qt\Qt5.2.1\5.2.1\msvc2010_opengl\bin\moc.exe %1 -o %2_moc.cpp
echo done