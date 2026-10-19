# PuTTY 0.85 — port C++ dla Windows x64

Kompletny pakiet aplikacji PuTTY, przygotowany z oficjalnych źródeł wydania 0.85.
Kod aplikacji kompiluje się jako C++20 przy użyciu LLVM-MinGW Clang. Zachowano
architekturę proceduralną, algorytmy, zasoby, dokumentację i licencję PuTTY.
Jest to nieoficjalna kompilacja lokalna; zainstalowany PuTTY pozostał bez zmian.
Modyfikacje portu C++ i rozszerzenia Windows: **m3rlinek**.

## Start na drugim komputerze

Zacznij od [instrukcji instalacji](docs/GETTING-STARTED.md).
Dokumentacja: [rozwój projektu](docs/DEVELOPMENT.md), [mapa kodu](docs/ARCHITECTURE.md)
i [plan modernizacji](docs/MODERNIZATION.md).

Repozytorium zawiera źródła i dokumentację. Katalogi `build/`, `dist/`
i `.venv/` powstają lokalnie podczas przygotowania środowiska.

## Programy

Nowe przesyłanie plików z Eksploratora do aktywnego katalogu SSH i pobieranie
przez `cp plik "$win"`: [konfiguracja i integracja powłoki](docs/FILE-TRANSFER.md).
Gotowe pliki i helper powłoki znajdziesz po budowaniu w `dist/`.
Nowe interaktywne sesje Bash przygotowują integrację automatycznie w pamięci,
bez instalacji plików na serwerze. Transfer ma natywny pasek postępu.
**Duplicate Session** otwiera nową powłokę na zalogowanym połączeniu SSH bez
ponownego hasła: [opis i zachowanie wspólnego połączenia](docs/DUPLICATE-SESSION.md).

Po zbudowaniu projektu w `dist/` znajdują się:

- `putty.exe` — główny klient z interfejsem graficznym;
- `puttytel.exe` — klient Telnet;
- `plink.exe` — klient wiersza poleceń;
- `pscp.exe` i `psftp.exe` — przesyłanie plików;
- `puttygen.exe` — generator kluczy;
- `pageant.exe` — agent kluczy SSH;
- `pterm.exe` — terminal wykorzystujący Windows ConPTY;
- `psocks.exe` — dodatkowe narzędzie proxy z drzewa źródłowego PuTTY.

Pliki EXE korzystają z systemowych bibliotek Windows/UCRT. Dla Plink, PSCP i PSFTP
sprawdzono uruchomienie z samym katalogiem Windows System32 w PATH. Nie mają podpisu
cyfrowego wydawcy oficjalnego PuTTY.

Biblioteki kompilatora C++ są wbudowane statycznie: instalowanie `libc++.dll`,
`libunwind.dll` ani środowiska deweloperskiego nie jest potrzebne. Paczkę
sprawdzono po rozpakowaniu z PATH ograniczonym do Windows System32.
Wymagany jest Windows 10/11 x64. Licencja wbudowanych bibliotek LLVM:
`THIRD-PARTY-LLVM-LICENSE.txt`.

## Budowanie

Wymagane: Windows 10/11 x64, CMake 3.21+, LLVM-MinGW z `clang`, `clang++`
i `mingw32-make` dostępnymi w PATH. Zweryfikowano Clang 22.1.8 i CMake 4.4.4.

Z katalogu tego projektu:

```powershell
.\build-cpp.ps1
```

Programy zostaną zbudowane w `build/` i skopiowane do `dist/`.
Kompilacja korzysta z rozszerzeń Clang odziedziczonych po kodzie C99, m.in.
zagnieżdżonych i tablicowych inicjalizatorów. Nie zweryfikowano budowania MSVC/GCC.
Niektóre miejsca nadal zgłaszają ostrzeżenia zgodności ze ścisłym ISO C++.
Assercje PuTTY pozostają aktywne także w konfiguracji Release.

## Testy

```powershell
.\build-cpp.ps1 -Test
```

CTest uruchamia 11 zestawów testów natywnych. Jeśli znajdzie Python 3,
uruchamia również zestaw 90 testów kryptograficznych. Lokalny test SSH/SFTP
wymaga dodatkowo Paramiko:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r test\requirements-port.txt
.\build-cpp.ps1 -Test
```

Można wskazać własny interpreter przez `-PythonExecutable C:\sciezka\python.exe`.
Test integracyjny słucha wyłącznie na `127.0.0.1`, używa losowego portu,
tymczasowego klucza i katalogu. Nie wymaga zewnętrznego serwera ani Twoich danych
logowania. Sprawdza SSH, przypięcie klucza hosta i transfer SFTP pliku 1 MiB.

## Zakres przeniesienia i wynik

W drzewie znajduje się 450 plików `.cpp`. Zweryfikowana kompilacja Windows
obejmuje 339 unikalnych plików źródłowych i 406 wpisów kompilacji; kod aplikacji
nie kompiluje żadnego pliku `.c`. CMake zachowuje język C dla testów możliwości
platformy. Pozostałe źródła dla Unix/macOS i dodatków są dołączone, ale ich
kompilacji na innych systemach nie sprawdzono.

Projekt przeszedł **13/13 zestawów CTest**, w tym 90 testów kryptograficznych oraz test
lokalnego SSH/SFTP. Sprawdzono też start procesów GUI PuTTY, PuTTYtel i PuTTYgen.
Pełny zakres wyników i ograniczeń znajduje się w [PORT-REPORT.md](PORT-REPORT.md),
a surowe raporty w `validation/`.

Nie przeprowadzono pełnego testu interaktywnego GUI, wszystkich konfiguracji
serwerów SSH, fizycznego portu szeregowego ani środowisk Kerberos/GSSAPI.

## Źródło i licencja

Oficjalne źródła: https://www.chiark.greenend.org.uk/~sgtatham/putty/releases/0.85.html

Archiwum `putty-src.zip`:
`232c5c286a5b35f445dbbf49e159469acde372a3907aef738d88e28b4b0f6da2` (SHA-256,
sprawdzone z opublikowanym plikiem sum). Metadane: `UPSTREAM.json`.
Oryginalny README zachowano jako `README`, a pełną licencję jako `LICENCE`.

Projekt ma 13 zestawów CTest. Test integracyjny obejmuje także prywatne
współdzielenie sesji, Unicode, transfer katalogów i odmowę ponownego połączenia.
