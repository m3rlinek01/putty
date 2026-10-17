# Zweryfikowane środowisko

| Element | Wersja / wariant |
|---|---|
| Platforma | Windows x64 |
| Język aplikacji | C++20 z rozszerzeniami Clang dla inicjalizatorów C99 |
| Kompilator | LLVM-MinGW UCRT x86_64, Clang 22.1.8 |
| Wydanie kompilatora | `20260616` |
| Generator | MinGW Makefiles / `mingw32-make` |
| CMake | 4.4.4, projekt wymaga minimum 3.21 |
| Python | 3.13.15 na pierwszym komputerze |
| Paramiko | 5.0.0, wymaganie zapisane w `test/requirements-port.txt` |

Oficjalne wydanie toolchainu:
https://github.com/mstorsjo/llvm-mingw/releases/tag/20260616

Archiwum: `llvm-mingw-20260616-ucrt-x86_64.zip`.
SHA-256 z metadanych wydania GitHub:

```text
b9b68a4d276e16fa25802aaba458e4638f64b3884c290aaccdc2d87083b6ca35
```

Sprawdzenie pobranego pliku:

```powershell
Get-FileHash .\llvm-mingw-20260616-ucrt-x86_64.zip -Algorithm SHA256
```

To nie jest toolchain MSVC ani `clang-cl`; wybrano kompilator kierujący na
`x86_64-w64-windows-gnu`. Projekt nie ma jeszcze potwierdzonej zgodności
ze ścisłym ISO C++20, MSVC, GCC ani buildami Unix/macOS.

Testy kryptograficzne używają biblioteki standardowej Pythona. Paramiko jest
potrzebne jedynie do tymczasowego serwera w teście integracyjnym. Żadne testy
nie wymagają danych logowania do rzeczywistych serwerów.
