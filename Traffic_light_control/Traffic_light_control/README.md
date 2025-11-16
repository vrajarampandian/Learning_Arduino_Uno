****Traffic Control — Arduino + Qt6 Real-Time Emulator****

A real-time traffic light controller that links a physical Arduino Uno (hardware LEDs) with a Qt6 desktop emulator (Windows, Visual Studio 2022 / MSBuild).
The Arduino runs the traffic sequence and streams state over USB serial; the Qt app reads those messages (via QSerialPort) and mirrors the lights and countdown in real time.

**Features**

Physical traffic lights driven by Arduino (Green / Yellow / Red).

Qt6 desktop emulator that mirrors the Arduino within ~100 ms.

Non-blocking firmware (millis() based) and timestamp-corrected GUI.

Simple serial protocol: STATE:<COLOR>:<REMAIN_MS> and REQ request.

Build and run using Qt6 + Visual Studio 2022 (MSBuild).

****Hardware — parts & wiring****

**Parts**

Arduino Uno (or compatible)

3 × LEDs (Green, Yellow, Red)

3 × 220 Ω resistors (recommended; 100 Ω OK but higher current)

Breadboard + jumper wires + USB cable

**Wiring (per LED)**

Arduino pin 8 → 220Ω → Green LED anode (long leg)

Arduino pin 10 → 220Ω → Yellow LED anode

Arduino pin 12 → 220Ω → Red LED anode

All LED cathodes → common breadboard GND rail → Arduino GND

Ensure Arduino GND is connected to the breadboard GND rail (common ground).

****Arduino firmware****

File: Traffic_control.ino

Behavior

Non-blocking state machine:

GREEN = 10 s

YELLOW = 3 s

RED = 5 s


**Serial settings**

Traffic_control.ino

Use Serial.println() so each message is newline-terminated.

Upload using Arduino IDE:

Open traffic_state_serial_periodic.ino in Arduino IDE.

Tools → Board → Arduino Uno (or your board).

Tools → Port → select correct COM port.

Upload.

Close Serial Monitor before using the Qt app.

**Qt Desktop Application (Windows, Qt6 + Visual Studio 2022)******

The Qt app reads serial data, parses STATE:... messages, and updates the UI (three painted lights + countdown). It uses QSerialPort and a QTimer for updates.

**Prerequisites**

Qt 6 (matching MSVC 2022 build, e.g. msvc2022_64) with Qt SerialPort module installed.

Visual Studio 2022 (MSVC toolchain).

Qt Visual Studio Tools extension (recommended) OR qmake (Qt) + MSBuild.

**Build & Run (Visual Studio / MSBuild)
Option A — Recommended: Use Qt Visual Studio Tools (GUI)******

Install Qt Visual Studio Tools extension in VS 2022.

In Visual Studio: Qt VS Tools → Qt Options → Add Qt installation (e.g. C:\Qt\6.10.0\msvc2022_64).

Open the project:

If you have a .sln / .vcxproj generated earlier, open it; or

Create a new VS project and add main.cpp, trafficwidget.* files and configure Qt via the Qt Project Settings (right-click project → Qt Project Settings → select Qt version).

In Solution Explorer → Project → Qt Project Settings → select the msvc kit.

Build: Build → Build Solution (Ctrl+Shift+B).

Run (F5). Make sure the Arduino is connected and the Arduino Serial Monitor is closed.

**Option B — Generate VS project with qmake then build with MSBuild**

Open x64 Native Tools Command Prompt for VS 2022.

cd to qt_app folder (the folder containing TrafficQt.pro or source files).

Run qmake to create a VS project:

"C:\Qt\<version>\msvc2022_64\bin\qmake.exe" -tp vc TrafficQt.pro


This generates .vcxproj / .sln.

Build with MSBuild:

msbuild TrafficQt.vcxproj /p:Configuration=Release


**Run the generated .exe from Release\.
**
Notes

If qmake errors appear, ensure you run it from the project directory inside the MSVC developer prompt.

If SerialPort headers missing: ensure Qt SerialPort is installed via MaintenanceTool.exe.

****Running the system (recommended order)****

Upload Arduino firmware using Arduino IDE and confirm serial output in Serial Monitor (115200). Close Serial Monitor.

Start the Qt app (ensure COM passed to TrafficWidget or auto-detect finds Arduino COM).

If the app shows Serial open failed, check Device Manager and pass the correct COM to main.cpp (e.g., "COM3").

The Qt UI will send REQ\n and sync to the Arduino. Both the physical LEDs and Qt emulator should now change together.

**Troubleshooting**

Serial open failed

Close Arduino Serial Monitor (only one app may open COM).

Verify exact COM port in Device Manager → Ports (COM & LPT) and use that port.

Try different USB cable or port (avoid hubs).

Reinstall drivers if board not detected (CH340 for clones; official Arduino drivers for genuine boards).

**UI doesn’t match LEDs / drift**

Ensure Arduino is broadcasting STATE:<COLOR>:<REMAIN_MS> regularly (e.g., every 100 ms).

Qt uses reported remaining ms and the receive timestamp to compensate for latency — ensure QSerialPort reads full lines (canReadLine()).

**Missing headers / include errors**

Confirm Qt SerialPort installed.

If using VS + CMake, set CMAKE_PREFIX_PATH to C:/Qt/<version>/msvc2022_64/lib/cmake so CMake finds Qt.

If using MSBuild via Qt VS Tools, ensure project Qt kit is the correct msvc build.

License

MIT — see LICENSE for details.

Author
Rajarampandian — Senior Software Engineer
(Use this repo to demo embedded ↔ desktop integration, serial comms, non-blocking state machines, and Qt UI skills in interviews.)
