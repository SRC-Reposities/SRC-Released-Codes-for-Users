/*************************************************
 * CHƯƠNG TRÌNH TEST MODULE NRTK
 * Tác giả     : Đặng Xuân Bá <badx@hcmute.edu.vn>
 * Phần cứng   : Arduino UNO / MEGA / DUE
 * Mô tả       : Đọc và hiển thị dữ liệu GPS RTK từ mô-đun NRTK
 * Phiên bản   : 2025/08/06
 *************************************************/

#include "SRC_GPS_DataReading.h"  // Thư viện đọc và xử lý dữ liệu GNGGA từ RTK
#include <HardwareSerial.h>      // Thư viện hỗ trợ UART (Serial1, Serial2, v.v.)

// -------------------------- CẤU HÌNH CỔNG UART --------------------------
// Ở Arduino MEGA/DUE: có sẵn nhiều cổng UART như Serial1, Serial2
// Ở Arduino UNO: bạn cần dùng module UART ngoài (VD: SoftwareSerial hoặc chuyển qua STM32)
#define RTKSerialPort Serial1  // Cổng UART kết nối với mô-đun GPS RTK

// -------------------------- KHAI BÁO ĐỐI TƯỢNG --------------------------
// Tạo một đối tượng kiểu SRC_GPS_DataReading để sử dụng các hàm xử lý RTK
SRC_GPS_DataReading gpsRTK;

// -------------------------- HÀM SETUP --------------------------
void setup() {
  // Khởi tạo cổng Serial chính để gửi dữ liệu lên máy tính qua cổng USB
  // (Dùng để xem dữ liệu trên Serial Monitor trong Arduino IDE)
  Serial.begin(115200);

  // Thiết lập mô-đun RTK:
  // - Tham số 100: số chu kỳ kiểm tra độ ổn định base (100 lần ~ 10s)
  // - RTKSerialPort: cổng UART đang kết nối mô-đun RTK (Serial1)
  gpsRTK.setup(100, RTKSerialPort);
}

// -------------------------- VÒNG LẶP CHÍNH --------------------------
void loop() {
  // Gọi hàm đọc dữ liệu từ mô-đun RTK
  // Nếu đọc thành công (đã parse được GNGGA hợp lệ) thì in ra
  if (gpsRTK.readData(RTKSerialPort)) {

    // ---------- IN THÔNG TIN TOẠ ĐỘ GPS (gốc) ----------
    // Dữ liệu lấy từ chuỗi GNGGA (chưa tính độ lệch so với base)
    // Đơn vị:
    //   - Lat/Lon: độ (nhân 10^7, cần chia lại nếu muốn in dạng thập phân)
    //   - Alt: cm (centimet)

    Serial.print("Lat: ");    
    Serial.print(gpsRTK.GPSData.current_gps_long[0]);  // Vĩ độ
    Serial.print("\tLong: "); 
    Serial.print(gpsRTK.GPSData.current_gps_long[1]);  // Kinh độ
    Serial.print("\tAlt: ");  
    Serial.print(gpsRTK.GPSData.current_gps_long[2]);  // Độ cao

    // ---------- IN TỌA ĐỘ ĐÃ CHUYỂN SANG HỆ TOẠ ĐỘ X/Y/Z ----------
    // Đây là độ lệch so với điểm base (tọa độ mốc ban đầu)
    // Đơn vị: cm (centimet)
    // Hữu ích khi cần biết di chuyển bao nhiêu từ điểm gốc (base)

    Serial.print("\tx: "); Serial.print(gpsRTK.GPSData.pos_cm[0]);
    Serial.print("\ty: "); Serial.print(gpsRTK.GPSData.pos_cm[1]);
    Serial.print("\tz: "); Serial.print(gpsRTK.GPSData.pos_cm[2]);

    // ---------- IN TRẠNG THÁI FIX CỦA RTK ----------
    // Trạng thái xác định độ tin cậy của dữ liệu:
    //   - 0 = No solution (chưa có tín hiệu GPS)
    //   - 1 = Float solution (dữ liệu chưa chính xác cao)
    //   - 2 = Fixed solution (dữ liệu chính xác cao)
    Serial.print("\tstatus: "); 
    Serial.print(gpsRTK.GPSData.RTK_Standard);

    // ---------- IN THỜI GIAN GIỮA 2 LẦN NHẬN DỮ LIỆU ----------
    // dt: khoảng thời gian (ms) giữa lần nhận hiện tại và lần trước
    // Giúp đánh giá độ ổn định, trễ tín hiệu...
    Serial.print("\tdt: "); 
    Serial.println(gpsRTK.GPSData.dt);
  }
}
