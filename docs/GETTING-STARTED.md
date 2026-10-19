# Uruchomienie na drugim komputerze

Zweryfikowana platforma to Windows 10/11 x64. Repozytorium jest prywatne:
na drugim komputerze zaloguj GitHub CLI na konto mające do niego dostęp.
Nie kopiuj tokenów ani plików logowania z pierwszego komputera do repozytorium.

## 1. Git, GitHub CLI, CMake i Python

Jeśli narzędzi brakuje, w PowerShell:

```powershell
winget install --exact --id Git.Git
winget install --exact --id GitHub.cli
winget install --exact --id Kitware.CMake
winget install --exact --id Python.Python.3.13
```

Po instalacji otwórz nowy PowerShell. Potwierdź `git --version`, `gh --version`,
`cmake --version` i `py -3.13 --version`.

## 2. Kompilator LLVM-MinGW

Użyj zestawu UCRT x86_64 z LLVM-MinGW, a nie samego `clang-cl` z Visual Studio.
Dokładnie zweryfikowane wydanie opisano w [TOOLCHAIN.md](TOOLCHAIN.md).

Pobierz i rozpakuj:
[llvm-mingw-20260616-ucrt-x86_64.zip](https://github.com/mstorsjo/llvm-mingw/releases/download/20260616/llvm-mingw-20260616-ucrt-x86_64.zip).
Dodaj jego katalog `bin` do PATH w bieżącej sesji, np.:

```powershell
# Dopasuj tę ścieżkę do miejsca rozpakowania na swoim komputerze.
$env:PATH = 'C:\Tools\llvm-mingw-20260616-ucrt-x86_64\bin;' + $env:PATH
clang++ --version
clang++ -dumpmachine
mingw32-make --version
```

Oczekiwany target to `x86_64-w64-windows-gnu`. Nowszy pakiet WinGet
`MartinStorsjo.LLVM-MinGW.UCRT` można sprawdzić osobno; nie jest on identyczny
z przypiętym, przetestowanym wydaniem 22.1.8.

## 3. Klonowanie i środowisko testowe

```powershell
gh auth login
gh repo clone m3rlinek01/putty
Set-Location putty
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r test\requirements-port.txt
.\build-cpp.ps1 -Test
```

Jeśli masz już działające logowanie GitHub, krok `gh auth login` można pominąć.
Jeśli `py` nie jest dostępny, użyj pełnej ścieżki do zainstalowanego Pythona 3.
Projekt buduje się samodzielnie z dołączonych źródeł.

Skrypt wybiera `.venv\Scripts\python.exe` z katalogu projektu. Możesz wskazać
inny interpreter przez `-PythonExecutable C:\sciezka\python.exe`.

## 4. Potwierdzenie wyniku

```powershell
ctest --test-dir build -N
ctest --test-dir build --output-on-failure
.\dist\plink.exe -V
```

Oczekiwany wynik: **13 testów CTest i 13/13 PASS**. Zestaw `crypto` zawiera
90 testów kryptograficznych, a `local_ssh_sftp` 13 kontroli klientów.

Programy EXE znajdziesz w `dist/`. Główny klient to `dist\putty.exe`.
Oryginalna aplikacja zainstalowana w systemie jest niezależna od tego katalogu.

## Typowe problemy

- **11 zamiast 13 testów:** CMake nie znalazł Pythona.
- **12 zamiast 13:** wybrany Python nie ma Paramiko. Zainstaluj wymagania
  wskazanym interpreterem i ponownie uruchom `build-cpp.ps1 -Test`.
- **Wybrany niewłaściwy Clang:** sprawdź `Get-Command clang, clang++, mingw32-make`.
  Wszystkie powinny pochodzić z LLVM-MinGW. Umieść jego `bin` na początku PATH.
- **Błąd CMake po przeniesieniu katalogu:** skonfiguruj nowy katalog budowania;
  cache zawiera ścieżki absolutne i nie powinien być przenoszony między komputerami.
- **Brak `pterm.exe`:** sprawdź wynik wykrywania ConPTY i właściwy Windows toolchain;
  zweryfikowana konfiguracja buduje również Pterm.
