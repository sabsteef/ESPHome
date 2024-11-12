#include "esphome.h"

namespace esphome {
namespace racon_gateway {

static const size_t EXPECTED_DATA_LENGTH = 64;  // Pas aan naar de verwachte lengte van de data

struct ParsedData {
  float aanvoer_temp;
  float retour_temp;
  float zonneboiler_temp;
  float buiten_temp;
  float boiler_temp;
  // Voeg hier andere velden toe zoals in `datamap`
};

class RaconGateway : public Component, public UARTDevice {
 public:
  RaconGateway(UARTComponent *parent) : UARTDevice(parent) {}
  void setup() override;
  void send_and_read_data();

 private:
  ParsedData parsed_data;
};

}  // namespace racon_gateway
}  // namespace esphome
