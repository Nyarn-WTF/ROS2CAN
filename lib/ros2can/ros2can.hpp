#ifndef _ROS2_CAN_
#define _ROS2_CAN_

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ros2arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>


#define SSID ""
#define SSID_PW ""
#define AGENT_IP 0
#define AGENT_PORT 0

#define ROSWIFI 810
#define ROSSERIAL 114514

#define CAN_RX_QUEUE_SIZE 2

//CAN_device_t CAN_cfg;
class ROS2CAN : public ros2::Node{
private:
    geometry_msgs::Twist pubmsg, submsg;
    WiFiUDP udp;
    SemaphoreHandle_t pubsemapho, subsemapho;
    TaskHandle_t publish, canrx;
    ros2::Publisher<geometry_msgs::Twist> *publisher;
     
    CAN_frame_t rx_frame;
    ESP32CAN *can;

    static void cbPublish(void*);
    static void cbSubsclibe(geometry_msgs::Twist*, void*);
    static void watchRXCAN(void *);
    
public:
    static ROS2CAN* thisPtr;
    ROS2CAN(int);
    void setPublishmsg();
    void setPublishmsg(geometry_msgs::Twist);
    void getSubscribemsg(geometry_msgs::Twist*);
    void sendTXCAN(geometry_msgs::Twist);
    void begin();
    void stop();
};

#endif