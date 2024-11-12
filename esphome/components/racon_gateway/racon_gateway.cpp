#include "racon_gateway.h"
#include "esphome/core/log.h"
#include "esphome/components/mqtt/mqtt_client.h"  // Voeg MQTT-component toe
#include "esphome.h"
#include <vector>

namespace esphome {
namespace racon_gateway {

static const char *TAG = "racon_gateway";

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");
}

void RaconGateway::send_and_read_data() {
  // Stuur dezelfde string als in de Python-code
  const char* send_string = "\x02\xFE\x01\x05\x08\x02\x01\x69\xAB\x03";
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), strlen(send_string));

  // Wacht 1 seconde om data te ontvangen
  delay(1000);

  // Lees de binnenkomende data
  std::vector<uint8_t> data;
  while (this->available()) {
    data.push_back(this->read());
  }

  // Log de ruwe data om te controleren
  ESP_LOGI(TAG, "Ontvangen ruwe data:");
  for (size_t i = 0; i < data.size(); ++i) {
    ESP_LOGI(TAG, "Byte %d: 0x%02X", i, data[i]);
  }

  // Verwerk de data met dezelfde structuur als Python `fmt`
  if (data.size() == EXPECTED_DATA_LENGTH) {
    // Vul de parsed_data struct met de geconverteerde waarden
    this->parsed_data.aanvoer_temp = data[5] * 0.01f;
    this->parsed_data.retour_temp = data[7] * 0.01f;
    this->parsed_data.zonneboiler_temp = data[9] * 0.01f;
    this->parsed_data.buiten_temp = data[11] * 0.01f;
    this->parsed_data.boiler_temp = data[13] * 0.01f;
    // Voeg andere velden toe zoals in `datamap` met conversies
    // ...

    // Log de geparste data
    ESP_LOGI(TAG, "Parsed Data:");
    ESP_LOGI(TAG, "  Aanvoer Temp: %f", this->parsed_data.aanvoer_temp);
    ESP_LOGI(TAG, "  Retour Temp: %f", this->parsed_data.retour_temp);
    ESP_LOGI(TAG, "  Zonneboiler Temp: %f", this->parsed_data.zonneboiler_temp);
    ESP_LOGI(TAG, "  Buiten Temp: %f", this->parsed_data.buiten_temp);
    ESP_LOGI(TAG, "  Boiler Temp: %f", this->parsed_data.boiler_temp);

    // Maak een JSON payload voor MQTT
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"aanvoer_temp\": %.2f, \"retour_temp\": %.2f, \"zonneboiler_temp\": %.2f, "
             "\"buiten_temp\": %.2f, \"boiler_temp\": %.2f}",
             this->parsed_data.aanvoer_temp, this->parsed_data.retour_temp, this->parsed_data.zonneboiler_temp,
             this->parsed_data.buiten_temp, this->parsed_data.boiler_temp);

    // Publiceer de data naar een specifiek MQTT-topic
    mqtt::global_mqtt_client->publish("racon_gateway/sensor_data", payload);
    ESP_LOGI(TAG, "Geparste data naar MQTT gestuurd: %s", payload);
  } else {
    ESP_LOGW(TAG, "Data niet ontvangen in het verwachte formaat of lengte!");
  }
}

}  // namespace racon_gateway
}  // namespace esphome
