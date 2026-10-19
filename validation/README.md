# Wyniki testów

Pełny zestaw uruchamia `build-cpp.ps1 -Test`. Aktualny log CTest znajduje się
w `build/Testing/Temporary/LastTest.log`. Test lokalnego SSH/SFTP zapisuje raport
`validation/integration.json`, ignorowany przez Git.

Raporty lokalne zawierają ścieżki środowiska testowego i nie są częścią kodu
źródłowego ani paczki dystrybucyjnej. Zakres sprawdzeń opisuje
`docs/FILE-TRANSFER-VALIDATION.md`.
