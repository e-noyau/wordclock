#pragma once

#include <IotWebConf.h>

class WebUI {
 public:
  WebUI(const char *accessPointName, const char *initialPassword);

  void setup();
  void loop();

 private:
  void handleRoot();

  DNSServer dns_server_;
  WebServer web_server_;
  IotWebConf iot_web_conf_;
};
