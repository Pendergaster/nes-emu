# nes-emu
This is implementation of nes emulator which aims to visualize the NES platform. 
This emulator visualizes main memories like prosessors address space, nametables, OAM and patterntables. Also includes 6502-code debugger for cartridges.

This is done mainly as educational project. I have have used existing documentation of 6502 processor and other NES components to implement this.
Other emulators has also been used as an inspiration.

This is not intended to be used as actual platform to play games.

Project uses OpenGL rendering and [Nulkear](https://github.com/Immediate-Mode-UI/Nuklear) library for GUI.
Some basic memory mappers are implemented and more might be coming. Emulator is in basic working stage, not all features are impleneted.
Currently is build as an unity build and only has linux/unix version. Windows could be done following the build.sh logic

# Images

Nestest rom for testing 6502 processor.
![image1](nesemudemo.png)

Gameplay visualisation for [Nova The Squirrel](https://github.com/NovaSquirrel/NovaTheSquirrel)
![image2](nesemudemo2.png)
