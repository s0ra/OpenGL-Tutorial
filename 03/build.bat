@echo off

cl WinMain.cpp /link user32.lib kernel32.lib ole32.lib shell32.lib gdi32.lib opengl32.lib 