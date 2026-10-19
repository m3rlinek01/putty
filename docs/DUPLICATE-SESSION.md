# Duplicate Session bez ponownego logowania

W nowej wersji PuTTY zaloguj się raz na serwer SSH-2, a następnie wybierz
**Duplicate Session** z menu okna. Nowe okno otwiera osobną powłokę z tym samym
użytkownikiem na już uwierzytelnionym połączeniu. Nie zapisuje ani nie odtwarza
hasła. Można również duplikować okno będące duplikatem.

Integracja Bash, przeciąganie plików i pobieranie przez `$win` pozostają
dostępne. Każda powłoka ma własny bieżący katalog, zgłaszany osobno przez prompt.
Duplikowanie tworzy nową powłokę; nie kopiuje pamięci procesu, zmiennych ani
uruchomionych programów ze starej konsoli.

Funkcja jest domyślnie włączona w **Connection → SSH → Duplicate logged-in
sessions without authenticating again**, również gdy transfer plików jest
wyłączony. Zmiana wymaga nowego połączenia. Przy pierwszym uruchomieniu tej
wersji należy połączyć się normalnie; okna pozostawione ze starszej wersji
programu nie uzyskują tej funkcji automatycznie.

Okno pierwotne utrzymuje wspólne połączenie SSH. Jego zamknięcie rozłączy
duplikaty (PuTTY pokazuje ostrzeżenie o połączeniach downstream). Zamknięcie
samego duplikatu nie zamyka pozostałych powłok. Jeśli pierwotne połączenie
zniknie w trakcie otwierania duplikatu, program zgłosi błąd zamiast łączyć się
ponownie z innym serwerem lub prosić o hasło dla nowego połączenia.

Dla okna niezalogowanego, zamkniętej sesji, wyłączonej opcji i protokołów innych
niż standardowy SSH-2 zachowane jest zwykłe tworzenie nowego połączenia.

Sprawdzenia automatyczne obejmują kopiowanie i serializację ustawień okna,
zachowanie identyfikatora przy duplikowaniu duplikatu i otwieranie dodatkowych
kanałów SSH bez nowej sesji TCP ani nowego uwierzytelnienia. Ręcznej próby menu
w działającym GUI nie wykonano.
