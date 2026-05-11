# HVNC

Standalone HVNC client and server, based on the [TinyNuke](https://github.com/rossja/TinyNuke)
banking trojan's HVNC module. The client opens a hidden desktop on the
operator side, with a title bar menu of commands to launch processes
on the target machine.

> [!WARNING]
> This project is for research and educational purposes only. I do not
> encourage or condone malicious use.

[View demo video](https://vimeo.com/597459719)

![demo](demo.png)

## Features

- Hidden desktop with Explorer
- Run dialog and PowerShell launchers
- Browser launchers: Chrome, Edge, Brave, Firefox, Internet Explorer
- Hidden client console
- Configurable listening port

## Usage

1. Set the server IP and port in `Client/Main.cpp`.
2. Build the client and server: open the folder in Visual Studio, or run `cmake -B build -A x64 && cmake --build build --config Release` (`-A Win32` for 32-bit).
3. Run the server and enter the listening port when prompted.
4. Run the client, then right click the hidden desktop's title bar to
   access commands.

## License

MIT.
