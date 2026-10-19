# Rozwój projektu

Modyfikacje portu C++ i rozszerzenia Windows: **m3rlinek**.

Projekt bazuje na oficjalnych źródłach PuTTY 0.85. Autorstwo, licencja i dane
wydania źródłowego pozostają w `LICENCE` i `UPSTREAM.json`.

## Kompilacja

Windows 10/11 x64, LLVM-MinGW UCRT (Clang 22.1.8), CMake 3.21+ i C++20.
Zweryfikowane narzędzia opisuje `TOOLCHAIN.md`; instalację `GETTING-STARTED.md`.
Kod wykorzystuje rozszerzenia Clang dla inicjalizatorów odziedziczonych z C99.
Nie potwierdzono kompilacji MSVC/GCC ani portów Unix/macOS.

```powershell
.\build-cpp.ps1 -Test
```

Biblioteki kompilatora są linkowane statycznie. Skrypt odrzuca pakowanie
programów wymagających dodatkowych bibliotek runtime kompilatora. Binaria
i cache nie są przechowywane w historii źródeł.

## Testy i zmiany

- Pełny zestaw ma 13 CTest, w tym 90 testów kryptograficznych oraz lokalne SSH/SFTP.
- Python i Paramiko z `test/requirements-port.txt` są wymagane dla pełnego zestawu.
- `test/transfer_shell.py` i `test/transfer_bootstrap.py` sprawdzają integrację Bash.
- `test/portable_windows.py` sprawdza rozpakowaną paczkę i start bez toolchainu w PATH.
- Assercje pozostają aktywne w Release; strict aliasing jest wyłączony.
- Zmiany protokołów, formatów kluczy, zerowania pamięci i korutyn wymagają
  odpowiednich testów regresji.
- Testy GUI obejmują start procesów; rzeczywistą interakcję należy sprawdzać osobno.

Mapa kodu: `ARCHITECTURE.md`. Kierunki dalszych zmian: `MODERNIZATION.md`.
Funkcje użytkowe: `FILE-TRANSFER.md` i `DUPLICATE-SESSION.md`.
