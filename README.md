# Pty-Qt - C++ library for work with PseudoTerminals

todo place for Travis-Build-Info

Pty-Qt is small library for access to console applications by pseudo-terminal interface on Mac, Linux and Windows. On Mac and Linux you can use standard PseudoTerminal API and on Windows you can use WinPty or ConPty.

## Pre-Requirements
  - ConPty part works only on Windows 10 >= 1903 (build > 18309) and can be built only with Windows SDK >= 10.0.18346.0 (maybe >= 17134, but not sure)
  - WinPty part requires winpty sdk for build and winpty.dll with winpty-agent.exe for deployment with target application. WinPty can work on Windows XP and later (depended on used build SDK: vc140 / vc140_xp). You can't link WinPty libraries inside your App, because it use cygwin for build.
  - UnixPty part can work on both Linux/Mac versions, because it based on standard POSIX pseudo terminals API
  - target platforms: x86 or x64
  - Required Qt >= 5.10

## Build on Windows
...

## Build on Mac/Ubuntu
...

## Usage
For use this library, just build it, link with 'pyqt' library and include 'ptyqt.h' into your project

## Run tests
...

## Run examples
...

## More information
This library based on:
  - https://github.com/Microsoft/node-pty
  - https://github.com/Microsoft/console
  - https://github.com/rprichard/winpty
  - https://github.com/xtermjs/xterm.js
  - https://github.com/lxqt/qterminal
