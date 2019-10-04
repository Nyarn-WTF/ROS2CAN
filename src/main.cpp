#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ros2arduino.h>
#include <CAN.h>

#define SSID       "SSID"
#define SSID_PW    "PW"
#define AGENT_IP   "IPADDRESS"
#define AGENT_PORT 2019

class ROS2CAN : public ros2::Node{
private:
public:
  ROS2CAN():Node("RobotNode"){
    Serial.end();
    Serial.begin(115200);
    while(!Serial);
    ros2::init(&Serial);
  }

  ROS2CAN(WiFiUDP *udp):Node("RobotNode"){
    WiFi.begin(SSID, SSID_PW);
    while(WiFi.status() != WL_CONNECTED);
    ros2::init(udp, AGENT_IP, AGENT_PORT);
  }

  void begin(){

  }

  void stop(){

  }
};

ROS2CAN* node;

void setup() {
  enableCore1WDT();
  node = new ROS2CAN();
}

void loop() {
  disableCore1WDT();
  enableCore1WDT();
  ros2::spin(node);
}