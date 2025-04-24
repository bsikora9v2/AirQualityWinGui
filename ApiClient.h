#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

/// Klasa obsługująca komunikację z API GIOŚ
class ApiClient {
public:
    /// Pobiera listę stacji pomiarowych
    static nlohmann::json fetchStations();

    /// Pobiera czujniki (stanowiska) dla danej stacji
    static nlohmann::json fetchSensors(int stationId);

    /// Pobiera dane pomiarowe dla stanowiska
    static nlohmann::json fetchMeasurements(int sensorId);

    /// Zapisuje dane do pliku JSON
    static void saveToFile(const std::string& filename, const nlohmann::json& data);

    /// Ładuje dane z pliku JSON
    static nlohmann::json loadFromFile(const std::string& filename);
};
