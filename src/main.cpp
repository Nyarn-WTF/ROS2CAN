#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ros2arduino.h>
#include <ESP32SJA1000.h>
#include "main.hpp"

class ROS2CAN : public ros2::Node{
private:
  ros2::Publisher<geometry_msgs::Twist> *_publisher;
  QueueHandle_t command_q, status_q;
  
  static void subsclibed(geometry_msgs::Twist* msg, void *arg){
    (void)(arg);
    static ROS2CAN *_this = ROS2CAN::thisPtr;
    xQueueSend(_this->command_q, msg, 0);
  }

  static void publishing(void *arg){
    static ROS2CAN *_this = ROS2CAN::thisPtr;
    geometry_msgs::Twist msg;
    while(1){
      if(xQueueReceive(_this->status_q, &msg, 0) == pdTRUE){
        _this->_publisher->publish(&msg);
      }
      delay(1);
    }
  }

  static void CANRead(int psize){
    static ROS2CAN *_this = ROS2CAN::thisPtr;
    geometry_msgs::Twist msg;
    //canframeをTwistに変換してstatus_qにセット
    xQueueSend(_this->status_q, &msg, 0);
  }

  static void CANSend(void *arg){
    static ROS2CAN *_this = ROS2CAN::thisPtr;
    static geometry_msgs::Twist msg;
    while(1){
      if(xQueueReceive(_this->command_q, &msg, 0) == pdTRUE){
        //msgをcanframeに入れて送信
      }
      delay(1);
    }
  }

public:
  static ROS2CAN* thisPtr;

  ROS2CAN():Node("RobotNode"){
    thisPtr = this;
    command_q = xQueueCreate(QUEUE_SIZE, sizeof(geometry_msgs::Twist));
    status_q = xQueueCreate(QUEUE_SIZE, sizeof(geometry_msgs::Twist));
    Serial.end();
    Serial.begin(115200);
    while(!Serial);
    _publisher = this->createPublisher<geometry_msgs::Twist>("RobotState");
    this->createSubscriber<geometry_msgs::Twist>("Command2Robot", (ros2::CallbackFunc)this->subsclibed, nullptr);
    xTaskCreatePinnedToCore(publishing, "pub", 1024*4, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(CANSend, "cansend", 1024*4, NULL, 3, NULL, 1);
    CAN.begin(1000E3);
    CAN.onReceive(CANRead);
    ros2::init(&Serial);
  }

  ROS2CAN(WiFiUDP *udp):Node("RobotNode"){
    thisPtr = this;
    command_q = xQueueCreate(QUEUE_SIZE, sizeof(geometry_msgs::Twist));
    status_q = xQueueCreate(QUEUE_SIZE, sizeof(geometry_msgs::Twist));
    WiFi.begin(SSID, SSID_PW);
    while(WiFi.status() != WL_CONNECTED);
     _publisher = this->createPublisher<geometry_msgs::Twist>("RobotState");
    this->createSubscriber<geometry_msgs::Twist>("Command2Robot", (ros2::CallbackFunc)this->subsclibed, nullptr);
    xTaskCreatePinnedToCore(publishing, "pub", 1024*4, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(CANSend, "cansend", 1024*4, NULL, 3, NULL, 1);
    CAN.begin(1000E3);
    CAN.onReceive(CANRead);
    ros2::init(udp, AGENT_IP, AGENT_PORT);
  }
};

ROS2CAN* node;
ROS2CAN* ROS2CAN::thisPtr;

void setup() {
  enableCore1WDT();
  node = new ROS2CAN();
}

void loop() {
  enableCore1WDT();
  
  ros2::spin(node);
}