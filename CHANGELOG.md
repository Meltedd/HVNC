# Changelog

## 2026-05

### Added
- MinGW cross compilation from Linux via the `mingw64-release` CMake preset, with the runtime statically linked ([c6fb2b0], [7dbdb29]).
- Windows CI matrix covering Win32 and x64, Debug and Release ([#32]).

### Changed
- Migrate the build from Visual Studio project files to CMake ([#32]).
- Make the input protocol and socket reads safe for x64 builds ([#32]).
- Allocate the LZNT1 workspace once per session instead of per frame ([171f4ca]).
- Disable Nagle on the HVNC sockets to cut frame and input latency ([900ccdf]).

### Fixed
- Walk to the top level for NC button and drag actions, so Win11 File Explorer's title bar buttons and drag work ([bb6894f]).
- Walk past compositor child windows when routing posted input, so clicks reach Chrome's tabs, omnibox, and dialogs ([6eab04a]).
- Capture DirectComposition content via `PW_RENDERFULLCONTENT`, so Chromium browsers and UWP apps no longer render blank ([0932477]).
- Drop a spurious BitBlt that wiped the captured frame before it could be sent ([717b562]).
- Bail cleanly on LZNT1 setup or compression failures instead of feeding a corrupt buffer downstream ([5324594]).
- Release desktop capture resources on teardown ([#33]).
- Preserve the current frame until the next is ready, instead of clearing it during ingest ([#33]).
- Tear down the desktop stream on an incomplete frame, instead of acknowledging it ([#33]).
- Close accepted worker thread handles on disconnect ([1629254]).
- Bind desktop sockets to HVNC session tokens so concurrent sessions stay isolated ([351b114]).
- Reject malformed server packets before allocating or decompressing them ([#32]).
- Harden HTTP response handling against short reads, integer overflow, and allocation failures ([c28d7f5]).
- Pin the MinGW Client at `-O1` because `-O3` emits a vectorized zero init that hangs `InputThread` ([c682a60]).
- Use `%hs` instead of `%s` in Server `wprintf` so MinGW formats narrow strings correctly ([3ffb31b]).

[#32]: https://github.com/Meltedd/HVNC/pull/32
[#33]: https://github.com/Meltedd/HVNC/pull/33
[c6fb2b0]: https://github.com/Meltedd/HVNC/commit/c6fb2b0
[7dbdb29]: https://github.com/Meltedd/HVNC/commit/7dbdb29
[171f4ca]: https://github.com/Meltedd/HVNC/commit/171f4ca
[900ccdf]: https://github.com/Meltedd/HVNC/commit/900ccdf
[bb6894f]: https://github.com/Meltedd/HVNC/commit/bb6894f
[6eab04a]: https://github.com/Meltedd/HVNC/commit/6eab04a
[0932477]: https://github.com/Meltedd/HVNC/commit/0932477
[717b562]: https://github.com/Meltedd/HVNC/commit/717b562
[5324594]: https://github.com/Meltedd/HVNC/commit/5324594
[1629254]: https://github.com/Meltedd/HVNC/commit/1629254
[351b114]: https://github.com/Meltedd/HVNC/commit/351b114
[c28d7f5]: https://github.com/Meltedd/HVNC/commit/c28d7f5
[c682a60]: https://github.com/Meltedd/HVNC/commit/c682a60
[3ffb31b]: https://github.com/Meltedd/HVNC/commit/3ffb31b
