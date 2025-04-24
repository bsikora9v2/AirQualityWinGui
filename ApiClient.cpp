#include "ApiClient.h"
#include <httplib.h>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

json ApiClient::fetchStations() {
    httplib::Client cli("http://api.gios.gov.pl");
    auto res = cli.Get("/pjp-api/rest/station/findAll");
    if (res && res->status == 200)
        return json::parse(res->body);
    throw std::runtime_error("Błąd podczas pobierania stacji.");
}

json ApiClient::fetchSensors(int stationId) {
    httplib::Client cli("http://api.gios.gov.pl");
    auto res = cli.Get(("/pjp-api/rest/station/sensors/" + std::to_string(stationId)).c_str());
    if (res && res->status == 200)
        return json::parse(res->body);
    throw std::runtime_error("Błąd podczas pobierania czujników.");
}

json ApiClient::fetchMeasurements(int sensorId) {
    httplib::Client cli("http://api.gios.gov.pl");
    auto res = cli.Get(("/pjp-api/rest/data/getData/" + std::to_string(sensorId)).c_str());
    if (res && res->status == 200)
        return json::parse(res->body);
    throw std::runtime_error("Błąd podczas pobierania pomiarów.");
}

void ApiClient::saveToFile(const std::string& filename, const json& data) {
    std::ofstream file("data/" + filename);
    file << data.dump(4);
}

json ApiClient::loadFromFile(const std::string& filename) {
    std::ifstream file("data/" + filename);
    if (!file) throw std::runtime_error("Nie można otworzyć pliku.");
    json j;
    file >> j;
    return j;
}
