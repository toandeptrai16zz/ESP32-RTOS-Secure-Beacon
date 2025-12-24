#ifndef SECURE_CONFIG_H
#define SECURE_CONFIG_H
#include <Arduino.h>

// Khóa bí mật và Tên
const String SECRET_KEY = "ESP32_VIP_KEY_2025"; 
const String DEVICE_NAME = "ESP32_TAG"; 
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

// Cấu hình khoảng cách
const int RSSI_THRESHOLD_NEAR = -60; // Lớn hơn là GẦN
const int RSSI_THRESHOLD_FAR  = -75; // Nhỏ hơn là XA

// Trạng thái hệ thống (Dùng để giao tiếp giữa các Task)
enum DeviceState {
    STATE_IDLE,  // Không thấy gì
    STATE_NEAR,  // Gần
    STATE_FAR,   // Xa
    STATE_FAKE   // Phát hiện giả mạo
};

#endif