@echo off
@echo Document Generation v1.0
@echo+

:CONTINUE

@echo File name, please (enter "exit" to quit):
SET /P FILENAME=
if /I "%FILENAME%"=="exit" goto END

@echo+
@echo Building %FILENAME%...
doxygen %FILENAME%
@echo Build OK !
@echo+

goto CONTINUE

:END :: ½áÊø³ÌÐò

@echo exit...

