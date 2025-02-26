# The PowershellHost Sample

![Build Status](https://github.com/your-username/your-repo-name/actions/workflows/msbuild.yml/badge.svg)

Welcome to the PowershellHost repo. This repo contains several types of samples for Hosting PowerShell Core:
## Build
### Requirements
- **Operating System**: Windows
- **C++ Compiler**: Visual Studio 2022
- **VCPKG**: Package manager for C++ libraries
- **Git**: Version control system to clone the VCPKG repository
- **PowerShell 7.5** or higher, Install from this URL :<br>
https://learn.microsoft.com/en-us/powershell/scripting/install/installing-powershell-on-windows?view=powershell-7.5
- **WebView2** Install from this URL :<br>
https://developer.microsoft.com/en-us/microsoft-edge/webview2/consumer?form=MA13LH

### Clone the VCPKG Repository
Open a terminal or command prompt and run the following command to clone the VCPKG repository:
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg
vcpkg integrate install
```
## Build the Project
1. Open the `PowerShellHost.sln` file in Visual Studio 2022.
2. Select the configuration (Release) and platform (x64).
3. Build the solution by clicking on `Build` > `Build Solution` or pressing `Ctrl + Shift + B`.

# Usage

## HostConsole
The HostConsole project is designed to host and interact with PowerShell from a native C++ application.<br>
It leverages the .NET runtime to execute PowerShell scripts and commands, providing a bridge between native C++ code and managed PowerShell code.
### Command Line Options
```powershell
HostConsole --file <path to script> --executionPolicy <execution policy>

Exemple:
HostConsole --file WordAutomation.ps1 --executionPolicy 0
```
## MonacoEditor

The MonacoEditor project integrates the Monaco Editor, a powerful code editor that powers Visual Studio Code,
into a native C++ application using the WebView2 control.<br>
This project allows users to edit and run PowerShell scripts within a native application environment.

### Features
- **Code Editing**: Users can write and edit PowerShell scripts using the Monaco Editor.
- **Script Execution**: Users can execute PowerShell scripts directly from the editor.
- **WebView2 Integration**: The Monaco Editor is hosted within a WebView2 control, providing a modern and responsive user interface.
- **AI Integration**: The Monaco Editor can be integrated with AI services to provide code translation beetween VBScript to PowerShell

## Run
### Requirements
- **WebView2** Install from this URL :<br>
- https://developer.microsoft.com/en-us/microsoft-edge/webview2/consumer?form=MA13LH
- **PowerShell 7.5** or higher, Install from this URL :<br>
- https://learn.microsoft.com/en-us/powershell/scripting/install/installing-powershell-on-windows?view=powershell-7.5
- **VC Runtime** Install from this URL :<br>
- https://learn.microsoft.com/en-us/cpp/windows/latest/release-notes/visual-cpp-redistributable-versions?view=msvc-160

