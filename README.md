# csc581-engine
Group game engine for CSC 581

# How to Run the Submission
Must have a Windows 10/11 environment with Visual Studio 2022 for C++ projects. 

Everything is in one Source.zip file.

**To configure:**
1. Unzip the folder
2. Extract all files to a location of your choice
3. SDL2's dll (`SDL2-2.30.6\lib\x64`) and libzmq's dll (`ZeroMQ\lib`) should be added to the environment PATH variable. Restart VS as necessary.
4. In Visual Studio, choose open a local folder and select the folder you just extracted
5. The default view of a local folder should be folder view
6. In Solution Explorer (right or left side), select "Switch between solutions and available views"
7. There will be a `game-engine.sln` item and a folder item. Double-click the `.sln` item.
8. Double-check that VS is configured for Debug x64

**For building the Game Engine:**
1. Right click `GameEngineLib` and select Build
2. The Game Engine static library is built! (as GameEngineLib.lib in the `x64\Debug` folder)

**Running the Client-Server:**
1. Right click `GameEngineLib` and select Build
2. Right click `Server` and select Build
3. Right click `Client` and select Build
4. Right click `Server` and select 'Select as Startup Project'
5. Run in Non-Debugger Mode, this starts the server
6. Right click `Client` and select 'Select as Startup Project'
7. Run in non-debugger mode 4 times to add 4 clients. (Do not spam run, server needs time to establish connection with each client)


**Default Game Controls:**  
move left: left arrow  
move right: right arrow  
jump: space bar  
quit: q (graceful disconnect)  
proportional toggle: s  
pause/unpause: p  
speed up / slow down: + / -  
