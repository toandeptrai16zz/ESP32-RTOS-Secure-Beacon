/**
 * TRẠM THU (STATION) - KIẾN TRÚC FREERTOS ĐA NHIỆM
 */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <MD5Builder.h>
#include "SecureConfig.h"

// --- CẤU HÌNH RTOS ---
#define LED_PIN 4   // Đèn LED trên mạch
#define SCAN_TIME 1 // Quét nhanh 1 giây/lần để đèn phản hồi lẹ hơn

QueueHandle_t dataQueue;
volatile DeviceState currentState = STATE_IDLE;
unsigned long lastSeenTime = 0; // Biến lưu thời gian lần cuối nhìn thấy

struct BleDataPacket {
    char salt[10];
    char signature[10];
    int rssi;
};

// --- HÀM HỖ TRỢ BĂM MD5 ---
String calculateHash(String salt) {
    MD5Builder md5;
    md5.begin();
    md5.add(salt + SECRET_KEY);
    md5.calculate();
    return md5.toString().substring(0, 8);
}

// --- CALLBACK QUÉT BLE (Chạy trong ngữ cảnh Task Scan) ---
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Chỉ xử lý nếu đúng UUID của hệ thống mình
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
            
            if (advertisedDevice.haveManufacturerData()) {
                std::string data = advertisedDevice.getManufacturerData();
                String rawStr = String(data.c_str());
                int splitIndex = rawStr.indexOf('#');

                if (splitIndex > 0) {
                    // Đóng gói dữ liệu
                    BleDataPacket packet;
                    String sSalt = rawStr.substring(0, splitIndex);
                    String sSig = rawStr.substring(splitIndex + 1);
                    
                    // Copy dữ liệu vào struct (dùng strncpy cho an toàn)
                    strncpy(packet.salt, sSalt.c_str(), sizeof(packet.salt));
                    strncpy(packet.signature, sSig.c_str(), sizeof(packet.signature));
                    packet.rssi = advertisedDevice.getRSSI();

                    // GỬI VÀO QUEUE (Gửi nhanh để Scan tiếp)
                    // portMAX_DELAY nghĩa là nếu Queue đầy thì chờ đến khi có chỗ trống
                    xQueueSend(dataQueue, &packet, 0); 
                }
            }
        }
    }
};

// ==========================================================
// TASK 1: CHUYÊN QUÉT (Producer)
// ==========================================================
void Task_ScanBLE(void *parameter) {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    for (;;) {
        // Quét trong SCAN_TIME giây. false = không clear kết quả cũ tự động
        pBLEScan->start(SCAN_TIME, false);
        
        // Sau mỗi đợt quét, xóa bộ đệm để làm mới
        pBLEScan->clearResults();
        
        // Nghỉ 100ms để nhường CPU
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// ==========================================================
// TASK 2: CHUYÊN TÍNH TOÁN & XÁC THỰC
// ==========================================================
void Task_ProcessData(void *parameter) {
    BleDataPacket recvPacket;

    for (;;) {
        // Chờ nhận dữ liệu từ Queue
        if (xQueueReceive(dataQueue, &recvPacket, 100 / portTICK_PERIOD_MS) == pdPASS) {
            
            lastSeenTime = millis(); // Cập nhật thời gian vừa thấy nhau
            
            // Tính toán bảo mật
            MD5Builder md5;
            md5.begin();
            md5.add(String(recvPacket.salt) + SECRET_KEY);
            md5.calculate();
            String myCalcSig = md5.toString().substring(0, 8);

            if (myCalcSig.equals(String(recvPacket.signature))) {
                // --> XÁC THỰC THÀNH CÔNG, GIỜ XÉT KHOẢNG CÁCH
                Serial.printf("RSSI: %d dBm\n", recvPacket.rssi);

                if (recvPacket.rssi > RSSI_THRESHOLD_NEAR) { // Ví dụ > -60
                    currentState = STATE_NEAR;
                } else if (recvPacket.rssi < RSSI_THRESHOLD_FAR) { // Ví dụ < -75
                    currentState = STATE_FAR;
                } else {
                    // Ở giữa (Vùng đệm) -> Tạm thời tắt đèn để đỡ loạn
                    currentState = STATE_IDLE;
                }
            }
        } 
        
        // CƠ CHẾ TIMEOUT: Nếu 3 giây không thấy gì -> Tự về IDLE (Tắt đèn)
        if (millis() - lastSeenTime > 3000) {
            currentState = STATE_IDLE;
        }
    }
}

// --- TASK ĐÈN LED (ƯU Tiên Cao) ---
void Task_Indication(void *parameter) {
    pinMode(LED_PIN, OUTPUT);
    
    for (;;) {
        switch (currentState) {
            case STATE_NEAR:
                // GẦN: CHỚP NHANH
                // Chu kỳ 100ms (50 bật - 50 tắt)
                digitalWrite(LED_PIN, HIGH); vTaskDelay(50 / portTICK_PERIOD_MS);
                digitalWrite(LED_PIN, LOW);  vTaskDelay(50 / portTICK_PERIOD_MS);
                break;

            case STATE_FAR:
                // Sáng 1 giây - Tắt 1 giây
                digitalWrite(LED_PIN, HIGH); vTaskDelay(1000 / portTICK_PERIOD_MS);
                digitalWrite(LED_PIN, LOW);  vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;

            default: // STATE_IDLE (Mất tín hiệu hoặc ở vùng giữa)
                // TẮT HẲN
                digitalWrite(LED_PIN, LOW);
                vTaskDelay(100 / portTICK_PERIOD_MS); // Nhường CPU
                break;
        }
    }
}
// ==========================================================
// SETUP CHÍNH
// ==========================================================
void setup() {
    Serial.begin(115200);
    Serial.println("Starting RTOS System...");

    // 1. Tạo Hàng đợi (chứa tối đa 10 gói tin)
    dataQueue = xQueueCreate(10, sizeof(BleDataPacket));

    // 2. Khởi tạo các Task
    // Cú pháp: Function, Name, StackSize, Param, Priority, Handle
    
    // Task Quét: Cần Stack khá lớn cho thư viện BLE
    xTaskCreate(Task_ScanBLE, "ScanTask", 4096, NULL, 1, NULL);
    
    // Task Xử lý: Cần Stack cho MD5 và String
    xTaskCreate(Task_ProcessData, "ProcessTask", 4096, NULL, 2, NULL);
    
    // Task Đèn: Nhẹ nhàng, ưu tiên thấp cũng được
    xTaskCreate(Task_Indication, "LedTask", 1024, NULL, 1, NULL);
}

void loop() {
    // Trong FreeRTOS, loop() bỏ trống hoặc xóa đi cũng được
    vTaskDelete(NULL); 
}