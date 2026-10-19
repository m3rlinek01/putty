# Pakiety Windows

Autor modyfikacji i pakowania: **m3rlinek**.

Każda architektura ma instalator EXE i portable ZIP. Wszystkie dziewięć
programów oraz licencje są dołączone. Biblioteki kompilatora są linkowane
statycznie; wymagane są systemowe biblioteki Windows 10/11 (x86: Windows 10).

## Budowanie

W PATH muszą znajdować się LLVM-MinGW UCRT, CMake oraz Inno Setup 7.1.0
(`ISCC.exe`). Kompilatory docelowe x64 i x86 są częścią jednego LLVM-MinGW.
Inno Setup pobierz z [oficjalnej strony](https://jrsoftware.org/isdl.php).

```powershell
.\build-cpp.ps1 -Architecture x64 -Test
.\build-cpp.ps1 -Architecture x86 -Test
.\package-windows.ps1 -Architecture x64
.\package-windows.ps1 -Architecture x86
```

Można podać `-InstallerCompiler C:\Tools\InnoSetup\ISCC.exe`,
`-PackageDirectory`, `-OutputDirectory` i `-Version 0.85-cpp.2`.
Skrypt sprawdza architekturę wszystkich EXE przed pakowaniem.
Paczkuje wyłącznie wskazane pliki, bez podkatalogów innych architektur.
Binaria są w `dist/` (x64) i `dist/x86/`; pakiety w `out/`.

## Instalacja

Instalator ma języki polski i angielski, działa bez uprawnień administratora
i instaluje do `%LOCALAPPDATA%\Programs\PuTTY-m3rlinek-x64` lub `-x86`.
Dodaje skróty w menu Start; skrót pulpitu jest opcjonalny.
Warianty mają osobne identyfikatory instalacji i mogą współistnieć.
Nie zmienia PATH ani zapisanych sesji PuTTY. Deinstalator usuwa własne pliki,
skróty i wpis instalacji, zachowując konfigurację sesji.
Wersje instalacyjne i portable nie mają podpisu cyfrowego.

## Weryfikacja

```powershell
python test/portable_windows.py out/PuTTY-0.85-cpp.2-windows-x86-portable.zip --architecture x86 --readobj C:\Tools\llvm-mingw\bin\llvm-readobj.exe
python test/installer_windows.py out/PuTTY-0.85-cpp.2-windows-x86-setup.exe out/PuTTY-0.85-cpp.2-windows-x86-portable.zip --architecture x86
```

Powtórz z `x64`. Test portable sprawdza nagłówki PE, importy systemowe
i uruchamianie programów bez toolchainu w PATH. Test instalatora sprawdza
cichą instalację w katalogu tymczasowym, zgodność plików z ZIP, ponowną
instalację i deinstalację. Odmawia zastąpienia istniejącej instalacji.
Wersja x86 jest testowana na Windows x64 przez WOW64; osobny system
Windows 10 32-bit nie został przetestowany.
