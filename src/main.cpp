#include <Arduino.h>
#include <ros2can.hpp>

ROS2CAN *ros2can;

void setup() {
  ros2can = new ROS2CAN(ROSWIFI);
  ros2can->begin();
}

void loop() {
  ros2::spin(ros2can);
}