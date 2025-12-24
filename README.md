# ESP32 BLE Security & Proximity System

Hệ thống cảnh báo khoảng cách và xác thực bảo mật không dây (Rolling Code MD5) trên nền tảng FreeRTOS.

## Tính năng chính
- Bảo mật: Xác thực chữ ký số MD5 chống sao chép tín hiệu.
- Đa nhiệm: Sử dụng FreeRTOS tách biệt tác vụ quét sóng và cảnh báo.
- Cảnh báo: Đèn LED nháy theo khoảng cách (Gần/Xa) dựa trên tín hiệu RSSI.
- Tiết kiệm pin: Trạm phát hoạt động ở chế độ ẩn (Stealth Mode).

## Phần cứng yêu cầu
- 2 bo mạch ESP32 (ESP32-CAM hoặc DevKit V1).
- Nguồn cấp 5V (Sạc dự phòng).

## Cấu trúc thư mục
- TRAM_PHAT_TAG: Code cho mạch phát (Tag).
- TRAM_THU_STATION: Code cho mạch thu (Station).

## Cấu hình (File SecureConfig.h)
Lưu ý: File này phải giống hệt nhau ở cả 2 mạch.

const String SECRET_KEY = "ESP32_VIP_KEY_2025"; 
const String DEVICE_NAME = "ESP32_TAG"; 
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

const int RSSI_THRESHOLD_NEAR = -60; // Ngưỡng Gần
const int RSSI_THRESHOLD_FAR  = -75; // Ngưỡng Xa

## Hướng dẫn sử dụng
1. Cài đặt Arduino IDE và thêm thư viện ESP32.  
2. Mở project TRAM_PHAT_TAG, cấu hình SecureConfig.h và tải lên mạch phát.
3. Mở project TRAM_THU_STATION, cấu hình SecureConfig.h và tải lên mạch thu.
4. Kết nối nguồn và kiểm tra hoạt động của hệ thống.
## Lưu ý
- Đảm bảo cả hai mạch sử dụng cùng một cấu hình trong SecureConfig.h.
- Điều chỉnh ngưỡng RSSI theo môi trường sử dụng để đạt hiệu quả tốt nhất.
- Hệ thống có thể được mở rộng với nhiều mạch thu hoặc phát tùy theo nhu cầu.
## Giấy phép
Dự án này được cấp phép theo giấy phép MIT