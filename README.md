# CAN Message Parser

This is a C++ project for parsing CAN frames from a text file (`transcript.txt`).  
It implements ISO-TP parsing logic, including Single Frames (SF), First Frames (FF), Consecutive Frames (CF), and Flow Control (FC) frames.  

The project is implemented using **C++17** and tested in **VSCode on Windows**.

## Project Structure

- `CANMessageParser.h` – Header file defining the `CANMessageParser` class and CAN frame/message structures.  
- `CANMessageParser.cpp` – Implementation of the parser, including frame parsing, message handling, and validation.  
- `main.cpp` – Entry point of the program; reads a text file line by line and feeds it to the parser.  
- `transcript.txt` – Example input file containing raw CAN frames in hexadecimal format.  
- `.vscode/` – Contains VSCode configuration files:
  - `c_cpp_properties.json` – Configures include paths and compiler path for IntelliSense.  
  - `tasks.json` – Defines the build task and the compiler to use.  
  - `launch.json` – Debugging configuration for VSCode.

## Requirements

1. **Visual Studio Code** installed.  
2. **MinGW-w64** installed (default path used in this project: `C:/mingw64/bin/g++.exe`).  
3. A C++17 compatible compiler. If using another compiler or path, see instructions below.  

## Build and Run Instructions

1. Open the project folder in VSCode.  
2. Check the compiler path in the `.vscode` configuration files:
   - `c_cpp_properties.json` – Update `"compilerPath"` to your compiler location.  
   - `tasks.json` – Update the `"command"` to point to your compiler.  
   - `launch.json` – Update `"miDebuggerPath"` if using debugging.  
3. Build the project:
   - Press `Ctrl+Shift+B` or run the build task in VSCode.  
   - The executable will be generated in the configured folder (usually the project root).  
4. Run the program:
   - Make sure `transcript.txt` is in the same folder as the executable (or adjust the path in `main.cpp`).  
   - Execute the program; it will read the file and print parsed CAN frames to the console.  

Example command from terminal (if compiled as `CANParser.exe`):

```bash
./CANParser.exe
