/*************************************************
 * CHƯƠNG TRÌNH TEST MODULE NRTK
 * Tác giả     : Đặng Xuân Bá <badx@hcmute.edu.vn>
 * Phần cứng   : Arduino UNO / MEGA / DUE
 * Mô tả       : Đọc và hiển thị dữ liệu GPS RTK từ mô-đun NRTK qua Serial Monitor
 * Phiên bản   : 2025/08/06
 *************************************************/

#include "SRC_GPS_DataReading.h"  // Thư viện xử lý dữ liệu GNGGA từ mô-đun RTK
#include <HardwareSerial.h>      // Hỗ trợ dùng nhiều cổng Serial như Serial1, Serial2...

// ---------------------------------------------------------
// CẤU HÌNH CỔNG UART KẾT NỐI VỚI MÔ-ĐUN RTK
// ---------------------------------------------------------
// Trên Arduino UNO: chỉ có 1 cổng Serial, cần module UART ngoài (VD: SoftwareSerial)
// Trên Arduino MEGA/DUE: có sẵn Serial1, Serial2, Serial3 → dùng trực tiếp
#define RTKSerialPort Serial1  // Sử dụng Serial1 để giao tiếp với mô-đun GPS RTK

// ---------------------------------------------------------
// TẠO ĐỐI TƯỢNG XỬ LÝ DỮ LIỆU GPS
// ---------------------------------------------------------
// gpsRTK sẽ chứa toàn bộ các thông số GPS thu được, như: lat/lon/alt, x/y/z, trạng thái fix, v.v.
SRC_GPS_DataReading gpsRTK;

// ---------------------------------------------------------
// HÀM setup(): chạy duy nhất một lần sau khi reset/khởi động
// ---------------------------------------------------------
void setup() {
  // Khởi tạo cổng Serial mặc định (UART0 – kết nối USB) để hiển thị dữ liệu
  // Mở Serial Monitor trong Arduino IDE để xem dữ liệu tại baudrate 115200
  Serial.begin(115200);

  // Thiết lập mô-đun RTK:
  // - Tham số đầu tiên (100): Số lần liên tiếp tín hiệu ổn định để xác định base (~10 giây)
  // - Tham số thứ hai: Cổng UART kết nối với mô-đun RTK (ở đây là Serial1)
  gpsRTK.setup(100, RTKSerialPort);
}

// ---------------------------------------------------------
// HÀM loop(): chạy liên tục sau khi setup() hoàn tất
// ---------------------------------------------------------
void loop() {
  // Đọc dữ liệu từ mô-đun RTK. Nếu có chuỗi GNGGA hợp lệ thì xử lý
  if (gpsRTK.readData(RTKSerialPort)) {

    // ================================
    // 1. HIỂN THỊ TOẠ ĐỘ GPS NGUYÊN GỐC
    // ================================
    // Đơn vị:
    // - Lat/Lon: độ, nhưng nhân 10^7 (VD: 1066800000 ~ 10.66800000 độ)
    // - Alt: độ cao theo mực nước biển, đơn vị cm
    // ⇒ Nếu cần xem tọa độ ở dạng dễ hiểu: chia Lat/Lon cho 10^7

    Serial.print("Lat: ");    
    Serial.print(gpsRTK.GPSData.current_gps_long[0]);  // Vĩ độ (latitude)

    Serial.print("\tLong: "); 
    Serial.print(gpsRTK.GPSData.current_gps_long[1]);  // Kinh độ (longitude)

    Serial.print("\tAlt: ");  
    Serial.print(gpsRTK.GPSData.current_gps_long[2]);  // Độ cao (altitude)

    // ================================
    // 2. HIỂN THỊ TOẠ ĐỘ X / Y / Z SO VỚI BASE
    // ================================
    // Đây là tọa độ tương đối so với điểm mốc base (tọa độ khi tín hiệu ổn định)
    // Đơn vị: cm
    // Ứng dụng:
    // - Đo khoảng cách dịch chuyển
    // - Điều khiển robot di chuyển
    // - Vẽ bản đồ theo thời gian thực

    Serial.print("\tx: ");
    Serial.print(gpsRTK.GPSData.pos_cm[0]);  // Độ lệch theo trục X

    Serial.print("\ty: ");
    Serial.print(gpsRTK.GPSData.pos_cm[1]);  // Độ lệch theo trục Y

    Serial.print("\tz: ");
    Serial.print(gpsRTK.GPSData.pos_cm[2]);  // Độ lệch theo trục Z (cao/thấp hơn base)

    // ================================
    // 3. TRẠNG THÁI ĐỊNH VỊ RTK (FIX STATUS)
    // ================================
    // RTK_Standard có thể là:
    // - 0 = No Solution: không có định vị
    // - 1 = Float: định vị tạm, chưa chính xác cao
    // - 2 = Fixed: định vị chuẩn RTK, độ chính xác centimet
    // → Ứng dụng: Kiểm tra xem tín hiệu đã sẵn sàng cho các hệ thống cần độ chính xác cao chưa

    Serial.print("\tstatus: ");
    Serial.print(gpsRTK.GPSData.RTK_Standard);

    // ================================
    // 4. KHOẢNG THỜI GIAN GIỮA 2 BẢN TIN (Δt)
    // ================================
    // Đơn vị: millisecond
    // → Cho biết tốc độ gửi dữ liệu của mô-đun RTK (thường từ 100–1000ms)
    // → Nếu dt quá lớn có thể do mất tín hiệu hoặc thiết bị bị treo

    Serial.print("\tdt: ");
    Serial.println(gpsRTK.GPSData.dt);
  }
}
