Monitor Jakości Powietrza - C++/WinAPI

1. Uruchom Visual Studio
2. Utwórz nowy projekt typu "Windows Desktop Application"
3. Skopiuj pliki:
   - ApiClient.h
   - ApiClient.cpp
   - AirQualityWinGui.cpp
4. Utwórz folder `data/` w katalogu projektu
5. Przez vcpkg zainstaluj:
   - nlohmann-json
   - httplib
6. Skonfiguruj projekt (właściwości → C/C++ → C++17)
7. Zbuduj projekt i uruchom
8. Aplikacja umożliwia:
   - Pobieranie listy stacji i sensorów
   - Pobieranie danych z API GIOŚ
   - Zapisywanie danych do pliku JSON
   - Rysowanie wykresu
   - Obsługę błędów (brak Internetu)
