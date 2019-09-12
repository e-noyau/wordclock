#include "WebUI.h"

#define HTTP_PORT 80


WebUI::WebUI(const char *accessPointName, const char *initialPassword)
    : web_server_(HTTP_PORT),
      iot_web_conf_(accessPointName, &dns_server_, &web_server_,
                    initialPassword) {}

void WebUI::setup() {
  iot_web_conf_.init();
  web_server_.on("/", [this] { handleRoot(); });
  web_server_.on("/config", [this] { iot_web_conf_.handleConfig(); });
  web_server_.onNotFound([this] { iot_web_conf_.handleNotFound(); });
}

void WebUI::loop() { iot_web_conf_.doLoop(); }

void WebUI::handleRoot() {
  // -- Let IotWebConf test and handle captive portal requests.
  if (iot_web_conf_.handleCaptivePortal()) {
    // -- Captive portal request were already served.
    return;
  }
  String s =
      "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" "
      "content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>Hello world!";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  web_server_.send(200, "text/html", s);
}
