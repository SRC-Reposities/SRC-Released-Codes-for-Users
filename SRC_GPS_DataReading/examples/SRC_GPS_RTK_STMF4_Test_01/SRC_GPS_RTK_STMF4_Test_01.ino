/*************************************************
 * CHƯƠNG TRÌNH TEST MODULE NRTK
 * Tác giả     : Đặng Xuân Bá <badx@hcmute.edu.vn>
 * Phần cứng   : STM32F407VETX
 * Mô tả       : Đọc dữ liệu GPS RTK từ module NRTK
 * Ngày tạo    : 2025/08/06
 *************************************************/

#include "SRC_GPS_DataReading.h"   // Thư viện đọc dữ liệu RTK
#include <HardwareSerial.h>

// Khai báo cổng UART dùng cho mô-đun RTK (F4V4 hỗ trợ HardwareSerial với chân PB11, PB10)
HardwareSerial RTKSerialPort(PB11, PB10);
SRC_GPS_DataReading gpsRTK; // Đối tượng quản lý dữ liệu GPS RTK

// ---------------------- HÀM KHỞI TẠO ----------------------
void setup() {
  Serial.begin(115200); // UART mặc định dùng để in dữ liệu ra màn hình
  gpsRTK.setup(100, RTKSerialPort); // Thiết lập GPS RTK với khoảng ổn định 100ms (~10s)
}

// ---------------------- VÒNG LẶP CHÍNH ----------------------
void loop() {
  if (gpsRTK.readData(RTKSerialPort)) {
    // In thông tin vị trí GPS: kinh độ, vĩ độ, độ cao
    Serial.print("Lat: ");    Serial.print(gpsRTK.GPSData.current_gps_long[0]);
    Serial.print("\tLong: "); Serial.print(gpsRTK.GPSData.current_gps_long[1]);
    Serial.print("\tAlt: ");  Serial.print(gpsRTK.GPSData.current_gps_long[2]);

    // In tọa độ X, Y, Z (cm)
    Serial.print("\tx: "); Serial.print(gpsRTK.GPSData.pos_cm[0]);
    Serial.print("\ty: "); Serial.print(gpsRTK.GPSData.pos_cm[1]);
    Serial.print("\tz: "); Serial.print(gpsRTK.GPSData.pos_cm[2]);

    // Trạng thái RTK (VD: 0 - chưa fix, 4 - fix)
    Serial.print("\tstatus: "); Serial.print(gpsRTK.GPSData.RTK_Standard);

    // Thời gian lấy mẫu
    Serial.print("\tdt: "); Serial.println(gpsRTK.GPSData.dt);
  }
}
