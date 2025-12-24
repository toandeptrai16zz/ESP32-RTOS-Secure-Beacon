/**
 * MÁY PHÁT (TAG) - CHỈ PHÁT SÓNG, KHÔNG ĐÈN
 */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <MD5Builder.h>
#include "SecureConfig.h" 

BLEAdvertising *pAdvertising;

// Hàm tạo chữ ký bảo mật
String generateSignature(String salt) {
    MD5Builder md5;
    md5.begin();
    md5.add(salt + SECRET_KEY);
    md5.calculate();
    return md5.toString().substring(0, 8);
}

// Task duy nhất: Cứ 2 giây đổi mã và phát sóng
void Task_Advertise(void *parameter) {
    BLEDevice::init(DEVICE_NAME.c_str());
    pAdvertising = BLEDevice::getAdvertising();
    
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    oAdvertisementData.setFlags(0x04);
    oAdvertisementData.setCompleteServices(BLEUUID(SERVICE_UUID));
    pAdvertising->setAdvertisementData(oAdvertisementData);

    for (;;) {
        // 1. Tạo mã mới
        String salt = String(random(100000, 999999));
        String signature = generateSignature(salt);
        std::string payload = std::string(salt.c_str()) + "#" + std::string(signature.c_str());

        // 2. Cập nhật gói tin
        BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
        oScanResponseData.setManufacturerData(payload);
        pAdvertising->setScanResponseData(oScanResponseData);
        
        // 3. Phát sóng
        pAdvertising->start();
        Serial.printf("Dang Phat: Salt=%s (LED dang tat)\n", salt.c_str());

        // 4. Nghỉ 2 giây
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        pAdvertising->stop(); 
    }
}

void setup() {
    Serial.begin(115200);
    // Tắt đèn Flash (GPIO 4) để đảm bảo không sáng
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW); 

    xTaskCreate(Task_Advertise, "AdvTask", 4096, NULL, 1, NULL);
}

void loop() {
    vTaskDelete(NULL);
}