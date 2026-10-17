# Mapa kodu

| Obszar | Główne miejsca | Rola |
|---|---|---|
| Build i testy | `CMakeLists.txt`, `cmake/`, `build-cpp.ps1` | Cele, możliwości platformy, CTest |
| Główne interfejsy | `putty.h`, `defs.h`, `network.h`, `marshal.h` | Typy, kontrakty, transport danych |
| Narzędzia wspólne | `utils/`, `misc.h`, `puttymem.h`, `tree234.h` | Bufory, pamięć, drzewa, kodowanie |
| SSH | `ssh/`, `ssh.h`, `sshcr.h` | Pakiety, korutyny, transport, uwierzytelnianie, kanały |
| Kryptografia | `crypto/`, `keygen/`, `mpint.h` | Prymitywy, klucze, implementacje sprzętowe |
| Terminal | `terminal/`, `ldisc.cpp`, `dialog.cpp`, `config.cpp` | Emulacja terminala, wejście, konfiguracja |
| Windows | `windows/window.cpp`, `windows/controls.cpp`, `windows/platform.h` | Okna, rysowanie, uchwyty i API systemu |
| Sesje i ustawienia | `settings.cpp`, `conf.h`, `conf-enums.h`, `windows/storage.cpp` | Konfiguracja i format zapisu |
| Inne protokoły | `otherbackends/`, `proxy/` | Telnet, serial, proxy i pozostałe backendy |
| Aplikacje CLI | `pscp.cpp`, `psftp.cpp`, `windows/plink.cpp` | Transfer plików i konsola |
| Agent i klucze GUI | `pageant.cpp`, `windows/pageant.cpp`, `windows/puttygen.cpp` | Pageant i PuTTYgen |
| Pozostałe systemy | `unix/`, `charset/`, `unicode/` | Źródła platformowe i dane znaków |
| Testy | `test/`, `windows/test/`, testowe cele `utils/` | Testy natywne, crypto i SSH/SFTP |
| Dokumentacja upstream | `doc/`, `README`, `LICENCE` | Oryginalny podręcznik i licencja |

Najważniejsze zależności modernizacji: zarządzanie pamięcią i widoczność obiektów
przechodzą przez interfejsy callbacków, vtable i makra `container_of`. Korutyny
opierają się na etykietach i przechowywanym stanie. Zmiany tych mechanizmów powinny
obejmować mały, dobrze przetestowany fragment, z rozpoznanym właścicielem każdego obiektu.

Pełna lista zależności linkowania jest w CMake. Listę faktycznych jednostek
kompilacji po konfiguracji można odczytać z `build/compile_commands.json`.
