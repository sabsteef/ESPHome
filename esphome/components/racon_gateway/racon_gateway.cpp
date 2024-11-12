#include "racon_gateway.h"
#include "esphome/core/log.h"
#include "esphome/components/mqtt/mqtt_client.h"
#include <vector>
#include <cstring>  // Voor memcpy

namespace esphome {
namespace racon_gateway {

static const char *TAG = "racon_gateway";
static const char send_string[] = "\x02\xFE\x01\x05\x08\x02\x01\x69\xAB\x03";
static const char *mqtt_topic_parsed_data = "esp32/parsed_data";

// Defineer een struct om de data in op te slaan, volgens `fmt`
struct ParsedData {
    int8_t start;
    int16_t aanvoer_temp;
    int8_t retour_temp;
    int8_t zonneboiler_temp;
    int16_t buiten_temp;
    int16_t array_13h[13];
    int8_t extra_byte;
    int16_t short_value;
    int8_t array_7b[7];
    uint8_t array_4B[4];
    int8_t array_11b[11];
    int16_t end_short1;
    int16_t end_short2;
    int8_t array_9b[9];
    int16_t final_short;
    int8_t stop_byte;
};

// Byte-naar-bit conversie functie
std::vector<int> byte_to_bit(uint8_t x) {
  std::vector<int> bits(8);
  for (int i = 0; i < 8; ++i) {
    bits[7 - i] = (x >> i) & 1;
  }
  return bits;
}

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");
}

void RaconGateway::send_and_read_data() {
  // Stuur de data
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);

  // Lees de binnenkomende data in een buffer
  std::vector<uint8_t> raw_data;
  while (this->available()) {
    raw_data.push_back(this->read());
  }

  // Controleer of de ontvangen data de juiste lengte heeft
  if (raw_data.size() < sizeof(ParsedData)) {
    ESP_LOGW(TAG, "Onvoldoende data ontvangen om te parsen.");
    return;
  }

  // Verwerk de data volgens `fmt`
  ParsedData parsed_data;
  const uint8_t *ptr = raw_data.data();

  std::memcpy(&parsed_data.start, ptr, sizeof(parsed_data.start));
  ptr += sizeof(parsed_data.start);

  std::memcpy(&parsed_data.aanvoer_temp, ptr, sizeof(parsed_data.aanvoer_temp));
  ptr += sizeof(parsed_data.aanvoer_temp);

  std::memcpy(&parsed_data.retour_temp, ptr, sizeof(parsed_data.retour_temp));
  ptr += sizeof(parsed_data.retour_temp);

  std::memcpy(&parsed_data.zonneboiler_temp, ptr, sizeof(parsed_data.zonneboiler_temp));
  ptr += sizeof(parsed_data.zonneboiler_temp);

  std::memcpy(&parsed_data.buiten_temp, ptr, sizeof(parsed_data.buiten_temp));
  ptr += sizeof(parsed_data.buiten_temp);

  std::memcpy(parsed_data.array_13h, ptr, sizeof(parsed_data.array_13h));
  ptr += sizeof(parsed_data.array_13h);

  std::memcpy(&parsed_data.extra_byte, ptr, sizeof(parsed_data.extra_byte));
  ptr += sizeof(parsed_data.extra_byte);

  std::memcpy(&parsed_data.short_value, ptr, sizeof(parsed_data.short_value));
  ptr += sizeof(parsed_data.short_value);

  std::memcpy(parsed_data.array_7b, ptr, sizeof(parsed_data.array_7b));
  ptr += sizeof(parsed_data.array_7b);

  std::memcpy(parsed_data.array_4B, ptr, sizeof(parsed_data.array_4B));
  ptr += sizeof(parsed_data.array_4B);

  std::memcpy(parsed_data.array_11b, ptr, sizeof(parsed_data.array_11b));
  ptr += sizeof(parsed_data.array_11b);

  std::memcpy(&parsed_data.end_short1, ptr, sizeof(parsed_data.end_short1));
  ptr += sizeof(parsed_data.end_short1);

  std::memcpy(&parsed_data.end_short2, ptr, sizeof(parsed_data.end_short2));
  ptr += sizeof(parsed_data.end_short2);

  std::memcpy(parsed_data.array_9b, ptr, sizeof(parsed_data.array_9b));
  ptr += sizeof(parsed_data.array_9b);

  std::memcpy(&parsed_data.final_short, ptr, sizeof(parsed_data.final_short));
  ptr += sizeof(parsed_data.final_short);

  std::memcpy(&parsed_data.stop_byte, ptr, sizeof(parsed_data.stop_byte));

  // Log de geparste data
  ESP_LOGI(TAG, "Parsed Data:");
  ESP_LOGI(TAG, "  Aanvoer Temp: %f", parsed_data.aanvoer_temp * 0.01f);
  ESP_LOGI(TAG, "  Retour Temp: %f", parsed_data.retour_temp * 0.01f);
  ESP_LOGI(TAG, "  Zonneboiler Temp: %f", parsed_data.zonneboiler_temp * 0.01f);
  ESP_LOGI(TAG, "  Buiten Temp: %f", parsed_data.buiten_temp * 0.01f);
  // Voeg andere velden toe zoals gedefinieerd in je Python `datamap`

  // Bouw een JSON string voor MQTT-publicatie
  char payload[512];
  snprintf(payload, sizeof(payload),
           "{\"aanvoer_temp\": %.2f, \"retour_temp\": %.2f, \"zonneboiler_temp\": %.2f, \"buiten_temp\": %.2f}",
           parsed_data.aanvoer_temp * 0.01f, parsed_data.retour_temp * 0.01f,
           parsed_data.zonneboiler_temp * 0.01f, parsed_data.buiten_temp * 0.01f);

  // Publiceer de JSON data naar MQTT
  if (mqtt::global_mqtt_client != nullptr) {
    mqtt::global_mqtt_client->publish(mqtt_topic_parsed_data, payload);
    ESP_LOGI(TAG, "Geparste data naar MQTT gestuurd: %s", payload);
  } else {
    ESP_LOGW(TAG, "MQTT client niet beschikbaar.");
  }
}

}  // namespace racon_gateway
}  // namespace esphome
