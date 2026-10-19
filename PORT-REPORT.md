# Port PuTTY 0.85 — Windows x64

Modyfikacje portu i rozszerzenia Windows: **m3rlinek**.

Oficjalne źródła PuTTY 0.85 są kompilowane jako C++20 z rozszerzeniami Clang.
Zachowano model danych, algorytmy i licencję upstream. Metadane archiwum
źródłowego i SHA-256 znajdują się w `UPSTREAM.json`.

Zbudowano dziewięć aplikacji: PuTTY, PuTTYtel, Plink, PSCP, PSFTP, PuTTYgen,
Pageant, Pterm i Psocks. Biblioteki C++ są wbudowane w EXE; pozostałe importy
pochodzą z systemu Windows. Licencję tych bibliotek dołączono do paczki.

## Rozszerzenia

- Transfer SFTP z Eksploratora do katalogu zdalnego promptu.
- Pobieranie przez `cp plik "$win"`, z wyborem lokalnego folderu.
- Integracja interaktywnego Bash przygotowywana automatycznie w pamięci.
- Natywny pasek postępu, anulowanie, Unicode i ochrona przed lokalnym nadpisaniem.
- Duplicate Session korzystające z uwierzytelnionego połączenia bez nowego hasła.

## Walidacja

Pełny zestaw: 13 CTest, w tym 90 testów kryptograficznych i 13 kontroli lokalnego
SSH/SFTP. Osobne testy Bash obejmują 14 kontroli helpera i 5 kontroli startu.
Testy przenośnej paczki sprawdzają importy dziewięciu EXE oraz start wybranych
programów z PATH ograniczonym do Windows System32.

Nie potwierdzono wszystkich rzeczywistych serwerów SSH, pełnej interakcji GUI,
fizycznego portu szeregowego, Kerberos/GSSAPI ani kompilacji innych systemów.
Programy nie mają podpisu cyfrowego oficjalnego wydawcy PuTTY.
