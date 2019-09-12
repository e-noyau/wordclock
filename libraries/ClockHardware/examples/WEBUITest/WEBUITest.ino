#include <WiFi.h>

#define DEBUG 1
#include <logging.h>

#include <WebUI.h>

WebUI ui("Ping", "pong");

void setup() {
  setupLogging();
  ui.setup();
};


void loop() {
  ui.loop();

  DLOGLN(WiFi.status() == WL_CONNECTED);
  DLOGLN(WiFi.localIP());
  delay(1000);

};

