/*
MIT License

Copyright (c) 2019 hogepiyo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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