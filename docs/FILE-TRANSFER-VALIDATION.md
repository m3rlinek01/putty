# Walidacja transferu — 19 października 2026

## Duplicate Session

Wersja z automatycznym dołączaniem duplikatu do uwierzytelnionego połączenia
przeszła 13/13 CTest w 17,65 s; log: `build/duplicate-session.log`.
Test `conf` sprawdza kopiowanie i rzeczywistą serializację ustawień, zachowanie
endpointu przy duplikowaniu duplikatu, niezmienność oryginału, wyłączone transfery,
wyłączenie funkcji oraz niezalogowane okno i inny protokół.
Lokalna integracja SSH/SFTP ma teraz 13 kontroli: dodano dwie dodatkowe powłoki
bez podawania hasła, z asercjami braku nowej sesji TCP i nowych prób logowania.
Pozostałe testy transferu są wykonywane po tych operacjach na tym samym
połączeniu. Ręcznej próby opcji Duplicate Session w GUI nie wykonano.

## Automatyczny start i natywny postęp

Wbudowano przygotowanie interaktywnego Bash przez strumień startowy SSH,
bez pliku helpera na serwerze i bez edycji profilu. Test
`test/transfer_bootstrap.py` przeszedł 5/5 kontroli: zgodność skryptu z danymi
w EXE, gotowe `$win`/`cp`, zachowane profile/hooki, zwykłe `cp`, zamknięcie fd3
i powrót do zwykłej powłoki w przypadku powłoki innej niż Bash. Test wykonano
lokalnie w Git Bash; zwykła historia `.bash_history` nie jest instalacją helpera.
Dotychczasowy helper przeszedł ponownie 14/14 testów.

Nowy pasek Windows korzysta z rekordów liczbowych PSCP. Test natywny ma obecnie
158 kontroli, w tym fragmentację rekordów, przepełnienie, UTF-8, zerowe pliki
i limity buforów. Integracja SFTP sprawdza także rzeczywiste rekordy postępu.
Pełny build tej wersji przeszedł 13/13 CTest w 17,23 s; log:
`build/automatic-progress-final.log`. Ponieważ użytkownik korzystał z poprzedniego
`dist/putty.exe`, nową paczkę przygotowano w `out/automatic-progress` przez
opcję `build-cpp.ps1 -DistDirectory`, bez zamykania aktywnej sesji.
Nadal nie wykonano ręcznej oceny wyglądu podczas transferu ani testu Zsh.

## Poprawka samowystarczalnej paczki

Po zgłoszeniu braku `libc++.dll` potwierdzono, że pierwsza paczka miała
niezamierzone zależności od `libc++.dll` i `libunwind.dll`. Testy z toolchainem
w PATH nie wykrywały błędu. Włączono statyczne linkowanie bibliotek kompilatora
i dodano odmowę pakowania EXE zależnych od takich DLL.

Nowy build przeszedł ponownie 13/13 CTest (16,90 s). Test
`test/portable_windows.py` sprawdził rzeczywiście rozpakowany ZIP w katalogu
tymczasowym: wszystkie dziewięć EXE importuje tylko biblioteki systemu Windows;
Plink/PSCP/PSFTP oraz ukryty start PuTTY/PuTTYtel/PuTTYgen przeszły z PATH
ograniczonym do Windows System32, bez dodatkowych DLL w paczce.
Nie jest to pełny test interaktywny GUI. Log: `build/portable-fix.log`.
Biblioteki LLVM są wbudowane w EXE; dołączono ich licencję.

Środowisko: Windows x64, LLVM-MinGW Clang 22.1.8 (wydanie 20260616,
SHA-256 zgodne z TOOLCHAIN.md), CMake 4.4.4, Python 3.13.2, Paramiko 5.0.0.

Przed zmianami odtworzono bazowe 12/12 CTest. Po zmianach pełny build
`build-cpp.ps1 -Test` przeszedł **13/13 CTest**, 19,08 s. Dziewięć aplikacji
zbudowano i skopiowano do `dist/`.

- Nowy `file_transfer`: 86 kontroli ścieżek, nazw zastrzeżonych Windows,
  UTF-8 i cytowania argumentów z rzeczywistym parserem Windows.
- Rozszerzony `terminal`: OSC777, BEL/ST, fragmentacja danych, NUL,
  przepełnienie, odzyskanie poprawnego parsowania i brak callbacku.
- `local_ssh_sftp`: 11 kontroli, w tym rekursywny transfer Unicode, literalne
  nawiasy w nazwach, odmowa junction i brak nowego połączenia TCP po błędnym
  identyfikatorze lub zamknięciu właściwej sesji. Oryginalny test odrzucenia
  błędnego klucza hosta zachowano.
- `crypto`: 90 testów upstream.
- Osobny helper Bash: 14/14 testów i sprawdzenie składni `bash -n`.

Lokalne logi budowania: `build/baseline-build.log` i
`build/file-transfer-final.log`; raport integracji: `validation/integration.json`.
Te pliki są lokalne i ignorowane przez Git.

Nie przeprowadzono ręcznego przeciągania z Eksploratora ani pełnej oceny
wizualnej okna postępu i konfiguracji. Zsh nie jest zainstalowany na tej maszynie;
jego ścieżka integracji wymaga sprawdzenia na rzeczywistym serwerze. Podane
wyniki nie oznaczają wykonania tych kontroli.
