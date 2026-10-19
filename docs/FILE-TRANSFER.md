# Przesyłanie plików między Windows a powłoką SSH

Ta funkcja wymaga zmodyfikowanego `putty.exe` i dołączonego `pscp.exe` oraz
serwera udostępniającego SFTP. Bash jest przygotowywany automatycznie przy
nowym połączeniu; nie trzeba kopiować plików na Linux ani wykonywać `source`.

## Automatyczne przygotowanie powłoki

Uruchom nową wersję PuTTY i połącz się ponownie z serwerem. Przy domyślnych
ustawieniach `cp plik "$win"` i przeciąganie plików działają już w pierwszej
powłoce Bash. Integracja jest wbudowana w EXE i przekazywana przy starcie SSH
przez strumień `/dev/fd/3`. Działa w pamięci tej powłoki. PuTTY nie zapisuje
helpera ani nie zmienia `.bashrc`/`.profile` na serwerze.

Standardowe profile logowania Bash są odczytywane przed dodaniem integracji.
Ustawiany jest `$win`, raportowanie katalogu i funkcja `cp`; istniejące hooki
promptu pozostają zachowane. Normalna historia Bash działa tak jak dotychczas.
Wcześniej ręcznie dodany `source` można usunąć ze swoich plików startowych,
ale ponowne załadowanie integracji nie dubluje hooków.

Automatyczny start dotyczy interaktywnych sesji z Bash jako `$SHELL`, bez
własnego polecenia zdalnego, subsystemu i bez wyłączonego PTY. Dla innych
powłok uruchamiana jest zwykła powłoka z informacją, że automatyczna integracja
wymaga Bash. Stary helper nadal jest dostępny jako opcjonalne narzędzie dla
Zsh i konfiguracji niestandardowych; nie jest wymagany w zwykłej sesji Bash.

W ustawieniach PuTTY przejdź do **Connection → SSH → Files**. Funkcja jest
domyślnie włączona; pusty katalog pobierania oznacza systemowy folder Pobrane.
Możesz ją wyłączyć dla danej zapisanej sesji albo wybrać własny katalog.
Osobna opcja **Prepare Bash automatically** pozwala wyłączyć automatyczne
przygotowanie, np. dla serwera z ograniczonymi poleceniami SSH.
Włączenie funkcji tworzy prywatne współdzielenie połączenia SSH, przez które
dołączony `pscp.exe` przesyła pliki bez ponownego wpisywania hasła. Zastępuje
to zwykłe ustawienia współdzielenia połączenia w tej sesji GUI. Po zmianie
ustawień trzeba ponownie połączyć sesję.
Zalogowane okno można duplikować bez ponownego hasła; szczegóły dotyczące
osobnej powłoki i czasu życia wspólnego połączenia są w `DUPLICATE-SESSION.md`.

## Użycie

Po pojawieniu się promptu przeciągnij pliki lub foldery z Eksploratora Windows
na okno PuTTY. Celem jest katalog zgłoszony przez ostatni prompt powłoki.
Poczekaj na prompt po `cd`; nie upuszczaj plików podczas wykonywania poleceń
lub działania programu pełnoekranowego.

Pobranie pliku i katalogu z serwera:

```sh
cp aaa.zip "$win"
cp "plik ze spacjami.zip" "$win"
cp -r katalog "$win"
cp -r katalog/ "$win"
cp -r . "$win"
cp jeden.zip drugi.zip "$win"
cp -- -nazwa-z-myslnikiem.zip "$win"
```

Funkcja obsługuje specjalny cel `$win` oraz opcje `-r`, `-R`, `-i` i `--`. Opcja
`-i` odpowiada obowiązkowemu potwierdzeniu pobrania w Windows, dzięki czemu
działa również typowy alias `cp='cp -i'`. Pozostałe
wywołania `cp` wykonują zwykły systemowy program. `command cp` zawsze omija
funkcję. Nie zastępuj `$win` prawdziwym katalogiem na serwerze.

Kod wyjścia funkcji potwierdza zgłoszenie żądania. Właściwy wynik przesyłania
pokazuje Windows; operacja jest asynchroniczna. Domyślny katalog docelowy to
systemowy folder Pobrane (Downloads); można wybrać własny katalog w opcjach
PuTTY. Żądanie pochodzące z terminala wymaga lokalnego potwierdzenia, domyślnie
z wybraną odpowiedzią „Nie”. Jedno polecenie może zgłosić maksymalnie 16 źródeł.

Pobieranie nie nadpisuje istniejących elementów: w razie kolizji dobierany jest
wolny sufiks nazwy. Dane najpierw trafiają do tymczasowego katalogu
`.putty-transfer-*`, a po sukcesie otrzymują nazwę docelową. Anulowanie albo
błąd może pozostawić taki katalog z niekompletnym transferem; można go usunąć
po zamknięciu operacji.

## Zakres i ograniczenia

Okno transferu pokazuje natywny pasek postępu, nazwę bieżącego pliku,
procent, przesłane MiB, prędkość i pozostały czas. Dla folderów są to dane
bieżącego pliku, nie całego drzewa. Przycisk Cancel przerywa transfer.
Szczegóły tekstowe są dostępne po błędzie, zamiast zastępować pasek postępu.

- Bieżący katalog pochodzi z ostatniego promptu; PuTTY nie odczytuje ekranu ani
  nie zgaduje ścieżki z tekstu promptu.
- Transfer używa użytkownika i hosta pierwotnej sesji SSH. Nie przejmuje
  uprawnień `sudo`/`su` ani połączenia otwartego poleceniem `ssh` wewnątrz sesji.
  Używaj integracji w powłoce zalogowanego użytkownika na pierwotnym serwerze.
- `tmux` i `screen` mogą filtrować sekwencje sterujące; przekazywanie ich nie
  jest automatycznie konfigurowane przez ten helper.
- Nazwy plików mogą zawierać spacje i znaki Unicode; ścieżki kodowane są jako
  bajty UTF-8 w zapisie szesnastkowym. Lokalny moduł transferu odrzuca ścieżki
  niewłaściwe dla Windows. Protokół dopuszcza maksymalnie 950 bajtów ścieżki.
- Helper nie obsługuje dowiązań symbolicznych jako źródeł pobierania ani
  zaawansowanych opcji `cp` przy specjalnym celu `$win`. Katalogi, w tym `.`
  i `..`, są zamieniane na fizyczną ścieżkę absolutną; katalog główny `/` jest
  odrzucany. Ścieżki ze znakami sterującymi również są odrzucane. Nieprawidłowy
  raport katalogu czyści poprzedni cel, aby przeciągnięcie nie trafiło do
  wcześniej odwiedzonego katalogu.

## Protokół integracji

Helper emituje `OSC 777`, z terminatorem BEL albo ST:

```text
ESC ] 777 ; putty-transfer;cwd;<hex ścieżki absolutnej> BEL
ESC ] 777 ; putty-transfer;download;<hex ścieżki absolutnej> BEL
```

Terminal przekazuje wyłącznie kompletne sekwencje, bez NUL i bez przepełnienia
bufora, do opcjonalnego callbacku frontendu Windows. Inne frontendy ignorują
ten protokół. Dane z serwera są niezaufane: callback sprawdza sesję SSH,
strukturę komunikatu i ścieżki, a pobieranie wymaga potwierdzenia na Windows.
