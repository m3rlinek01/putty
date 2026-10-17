# Kolejność modernizacji

To plan dalszej pracy. Nie opisuje zmian, które zostały już wykonane.
Priorytetem jest zachowanie działania i małe, przeglądalne kroki.

## 0. Odtworzenie baseline

Uruchomić `build-cpp.ps1 -Test`, potwierdzić 12/12 zestawów i zapisać toolchain.
Rozróżnić ostrzeżenia istniejące przed zmianą od nowych. Przy zmianie kompilatora
porównać osobno wyniki starego i nowego środowiska.

## 1. Małe zmiany zgodności C++

Dobry pierwszy etap: jeden moduł `utils/` z czytelnym kontraktem i testami.
Poprawić const correctness, usunąć niepotrzebne powtórzenia castów i nadać
konwersjom jawny sens. Nie rozszerzać tego automatycznie na całe drzewo.
Akceptacja: kompilacja, odpowiedni test modułu i pełne 12 zestawów bez regresji.

## 2. Ograniczanie rozszerzeń C99

Zamieniać małymi grupami inicjalizatory tablicowe/zagnieżdżone i porządkować
designatory. Zachować zera domyślnych pól, liczbę elementów oraz bajty stałych
kryptograficznych. Celem etapu jest redukcja zależności od rozszerzeń Clang.

## 3. Własność zasobów na granicach

Zacząć od małych wrapperów zasobów WinAPI, np. uchwytów zamykanych jednym
ustalonym wywołaniem. Dopiero po rozpoznaniu własności rozważyć RAII dla buforów
i obiektów. Sprawdzić ścieżki błędów, asynchroniczne callbacki, czas życia i sprzątanie.

## 4. Build i testy przenośności

Gdy kod będzie wystarczająco zgodny ze standardem, zweryfikować drugi kompilator.
Osobno zaplanować Linux/macOS i źródła obecnie nieobjęte buildem Windows.
CI można dodać jako oddzielny etap z ustalonym toolchainem i pełnym zestawem testów.

## 5. Modernizacja interfejsu użytkownika

Najpierw ustalić z właścicielem kierunek wizualny i oczekiwane zmiany zachowania.
Rozważać oddzielnie skalowanie DPI, dostępność, ergonomię sesji i wygląd.
Każdą zmianę GUI sprawdzać interaktywnie; sam start procesu nie pokrywa interakcji.

## Obszary wymagające szerszych testów przy zmianach

Formaty PPK i ustawień, protokół SSH, prymitywy kryptograficzne, korutyny,
zerowanie pamięci z sekretami i ABI callbacków. Ich zmiana wymaga testów
wykraczających poza prostą poprawną kompilację.
