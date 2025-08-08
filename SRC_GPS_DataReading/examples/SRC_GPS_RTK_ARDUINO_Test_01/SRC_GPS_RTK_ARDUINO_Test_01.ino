/*************************************************
 * CHƯƠNG TRÌNH TEST MODULE NRTK
 * Tác giả     : Đặng Xuân Bá <badx@hcmute.edu.vn>
 * Phần cứng   : Arduino UNO / MEGA / DUE
 * Mô tả       : Đọc và hiển thị dữ liệu GPS RTK từ mô-đun NRTK
 * Phiên bản   : 2025/08/06
 *************************************************/

#include "SRC_GPS_DataReading.h"  // Thư viện đọc dữ liệu GPS RTK
#include <HardwareSerial.h>

#define RTKSerialPort Serial1  // Cổng UART kết nối với mô-đun RTK

SRC_GPS_DataReading gpsRTK; // Đối tượng xử lý dữ liệu GPS RTK

// ---------------------- HÀM KHỞI TẠO ----------------------
void setup() {
  Serial.begin(115200); // Cổng UART mặc định để hiển thị dữ liệu ra Serial Monitor
  gpsRTK.setup(100, RTKSerialPort); // Thiết lập module RTK với chu kỳ 100ms (~ổn định trong 10s)
}

// ---------------------- VÒNG LẶP CHÍNH ----------------------
void loop() {
  if (gpsRTK.readData(RTKSerialPort)) {
    // Hiển thị toạ độ GPS: vĩ độ, kinh độ, độ cao
    Serial.print("Lat: ");    Serial.print(gpsRTK.GPSData.current_gps_long[0]);
    Serial.print("\tLong: "); Serial.print(gpsRTK.GPSData.current_gps_long[1]);
    Serial.print("\tAlt: ");  Serial.print(gpsRTK.GPSData.current_gps_long[2]);

    // Hiển thị vị trí x, y, z theo đơn vị cm
    Serial.print("\tx: "); Serial.print(gpsRTK.GPSData.pos_cm[0]);
    Serial.print("\ty: "); Serial.print(gpsRTK.GPSData.pos_cm[1]);
    Serial.print("\tz: "); Serial.print(gpsRTK.GPSData.pos_cm[2]);

    // Trạng thái RTK (0: No Fix, 4: RTK Fix...)
    Serial.print("\tstatus: "); Serial.print(gpsRTK.GPSData.RTK_Standard);

    // Hiển thị thời gian đo (timestamp)
    Serial.print("\tdt: "); Serial.println(gpsRTK.GPSData.dt);
  }
}
