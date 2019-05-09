#include "Blob.h"
#include "InfluxPublisher.h"
#include "LoraReaderPoll.h"
#include "Reading.h"
#include "TimeTask.h"
#include "BlobSensor.h"

#define SDA 21
#define SCL 22
#define OLEDPIN 16

OLEDClass OLED(OLEDPIN, SDA, SCL);
Blob blob;
InfluxPublisher publisher(blob.readingsQueue,"monitor.phisaver.com", 8086, "test", "blobs", "bbeeson", "*****");
LoraReaderPoll reader(blob.readingsQueue);
BlobSensor ts(blob.readingsQueue);

void setup() {
  Serial.begin(115200); while (!Serial) {};
  //Log.begin(LOG_LEVEL_VERBOSE, &Serial); // SILENT|FATAL|ERROR|WARNING|NOTICE|TRACE|VERBOSE
  Log.begin(LOG_LEVEL_TRACE, &Serial); // SILENT|FATAL|ERROR|WARNING|NOTICE|TRACE|VERBOSE

  try {
    OLED.begin();
    OLED.setFramesToDisplay(OLEDClass::MessagesMask);
    OLED.message("Bridge Blob!");
    
    blob.addPublisher(&publisher);
    blob.addReader(&reader);  
    blob.addSensor(&ts);
    blob.setupWiFi();
    blob.setupLora();
    blob.begin();
    
    publisher.taskify(10000 /* ms */, 1 /* task priority */);
    reader.taskify(1 /* ms */,2 /* priority */);
    ts.showOnOLED = false;
    ts.taskify(5000, 1);
    TimeTask.begin(3600 * 10, 0);
    
  } catch (const std::exception& e) {
    Log.fatal("%s\n", e.what());
    OLED.message(e.what());
    delay(10000);
    ESP.restart();
  }
  Log.notice("Setup complete\n");
}

void loop(void) {
  static int wifiStatus;
  if (WiFi.status() != wifiStatus) {
    Log.verbose("WiFi Status: %s\n",wl_status_to_string(WiFi.status()));  
    OLED.message("WiFi:" + String(wl_status_to_string(WiFi.status())));
    wifiStatus = WiFi.status();
  }
  
  //Serial.println(TimeTask.getLocalTimeStr());
  
  delay(10000);
}

const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
}
