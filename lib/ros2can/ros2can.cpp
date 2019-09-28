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

#include <ros2can.hpp>

ROS2CAN *ROS2CAN::thisPtr = NULL;
CAN_device_t CAN_cfg;

ROS2CAN::ROS2CAN(int mode):Node("Robot"){
    ROS2CAN::thisPtr = this;
    //CAN Config
    CAN_cfg.speed = CAN_SPEED_125KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_5;
    CAN_cfg.rx_pin_id = GPIO_NUM_4;
    CAN_cfg.rx_queue = xQueueCreate(CAN_RX_QUEUE_SIZE, sizeof(CAN_frame_t));
    can = new ESP32CAN();
    can->CANInit();
    //ROS Config
    pubsemapho = xSemaphoreCreateBinary();
    subsemapho = xSemaphoreCreateBinary();
    //Way to XCRE-DDS Agent Config
    if(mode == ROSWIFI){
        WiFi.begin(SSID, SSID_PW);
        while(WiFi.status() != WL_CONNECTED);
        ros2::init(&this->udp, AGENT_IP, AGENT_PORT);
    }else if(mode == ROSSERIAL){
        Serial.begin(115200);
        while(!Serial);
        ros2::init(&Serial);
    }else{
        return;
    }
}

void ROS2CAN::begin(){
    this->createSubscriber<geometry_msgs::Twist>("command", (ros2::CallbackFunc)this->cbSubsclibe, nullptr);
    publisher = this->createPublisher<geometry_msgs::Twist>("state");
    xTaskCreatePinnedToCore(this->cbPublish, "publish", 1024*4, NULL, 1, &this->publish, CONFIG_ARDUINO_RUN_CORE1);
    xTaskCreatePinnedToCore(this->watchRXCAN, "canrx", 1024*4, NULL, 1, &this->canrx, CONFIG_ARDUINO_RUN_CORE1);
}

void ROS2CAN::stop(){
    this->deleteSubscriber("command");
    this->deletePublisher("state");
    vTaskDelete(this->publish);
    vTaskDelete(this->canrx);
}

void ROS2CAN::watchRXCAN(void *unuse){
    static ROS2CAN *_this = ROS2CAN::thisPtr;
    while(1){
        if (xQueueReceive(CAN_cfg.rx_queue, &_this->rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
            //キューの中身をpubmsgにしてsetPublishmsgよぶ
        }
        delay(1);
    }
}

void ROS2CAN::sendTXCAN(geometry_msgs::Twist sendmsg){
    static SemaphoreHandle_t sendsemapho = xSemaphoreCreateBinary();
    
    CAN_frame_t tx_frame;
    //sendmsgをCAN_frameに変換してCANwrite
    xSemaphoreTake(sendsemapho, portMAX_DELAY);
    can->CANWriteFrame(&tx_frame);
    xSemaphoreGive(sendsemapho);
}

void ROS2CAN::setPublishmsg(){
    xSemaphoreGive(pubsemapho);
}

void ROS2CAN::setPublishmsg(geometry_msgs::Twist pmsg){
    pubmsg = pmsg;
    xSemaphoreGive(pubsemapho);
}

void ROS2CAN::getSubscribemsg(geometry_msgs::Twist *rtmsg){
    xSemaphoreTake(subsemapho, portMAX_DELAY);
    rtmsg->linear = submsg.linear;
    rtmsg->angular = submsg.angular;
    xSemaphoreGive(subsemapho);
}

void ROS2CAN::cbPublish(void* unused){
    static geometry_msgs::Twist *msg = (geometry_msgs::Twist*)malloc(sizeof(geometry_msgs::Twist));
    static ROS2CAN *_this = ROS2CAN::thisPtr;
    static SemaphoreHandle_t publisehrsemapho = xSemaphoreCreateBinary();
    while(1){
        xSemaphoreTake(_this->pubsemapho, portMAX_DELAY);
        msg->linear = _this->pubmsg.linear;
        msg->angular = _this->pubmsg.angular;
        xSemaphoreTake(publisehrsemapho, portMAX_DELAY);
        _this->publisher->publish(msg);
        xSemaphoreGive(publisehrsemapho);
        delay(1);
    }
    free(msg);
}

void ROS2CAN::cbSubsclibe(geometry_msgs::Twist *msg, void* arg){
    (void)(arg);
    ROS2CAN *_this = ROS2CAN::thisPtr;
    xSemaphoreTake(_this->subsemapho, portMAX_DELAY);
    _this->submsg.linear = msg->linear;
    _this->submsg.angular = msg->angular;
    _this->sendTXCAN(_this->submsg);
    xSemaphoreGive(_this->subsemapho);
}