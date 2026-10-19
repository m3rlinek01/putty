# Code map

| Area | Main files | Responsibility |
|---|---|---|
| Build | `CMakeLists.txt`, `cmake/`, `build-cpp.ps1` | Targets, platform detection, CTest |
| Packaging | `package-windows.ps1`, `installer/putty.iss` | Portable archives and installers |
| Interfaces | `putty.h`, `defs.h`, `network.h`, `marshal.h` | Types, callbacks, data transport |
| Utilities | `utils/`, `misc.h`, `puttymem.h`, `tree234.h` | Buffers, memory, trees, encoding |
| SSH | `ssh/`, `ssh.h`, `sshcr.h` | Packets, coroutines, authentication, channels |
| Cryptography | `crypto/`, `keygen/`, `mpint.h` | Primitives, keys, hardware implementations |
| Terminal | `terminal/`, `ldisc.cpp`, `dialog.cpp`, `config.cpp` | Emulation, input, configuration |
| Windows UI | `windows/window.cpp`, `windows/controls.cpp`, `windows/platform.h` | Windows, drawing, handles, system APIs |
| Shell transfers | `windows/file-transfer.cpp`, `contrib/putty-transfer.sh` | Transfer UI and shell integration |
| Settings | `settings.cpp`, `conf.h`, `conf-enums.h`, `windows/storage.cpp` | Configuration and storage |
| Other backends | `otherbackends/`, `proxy/` | Telnet, serial, proxies |
| CLI clients | `pscp.cpp`, `psftp.cpp`, `windows/plink.cpp` | File transfer and console clients |
| Agent and keys | `pageant.cpp`, `windows/pageant.cpp`, `windows/puttygen.cpp` | Pageant and PuTTYgen |
| Other platforms | `unix/`, `charset/`, `unicode/` | Platform sources and character data |
| Tests | `test/`, `windows/test/`, utility test targets | Native, crypto and SSH/SFTP tests |
| Upstream docs | `doc/`, `README`, `LICENCE` | Original manual and license |

Object access and ownership pass through callbacks, vtables and `container_of`.
Coroutines use labels and stored state. Check ownership and callback lifetimes
before changing either mechanism.

CMake defines link dependencies. Configured compilation units are listed in
`build/compile_commands.json` and `build-x86/compile_commands.json`.
