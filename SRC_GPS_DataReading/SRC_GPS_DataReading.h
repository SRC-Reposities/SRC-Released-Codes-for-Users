/*************************************************************************************  
 * @file       SRC_GPS_DataReading.h
 * @brief      Thư viện đọc dữ liệu GNGGA (NMEA 0183) từ thiết bị GPS RTK và phân tích
 *             thành các tham số chi tiết như: lat, lon, alt, x, y, z, trạng thái, thời gian...
 * @hardware   Hỗ trợ: Arduino, STM32F1, STM32F4
 * @version    1.1 
 * @date       2025-08-06
 * @author     Dang Xuan Ba (badx@hcmute.edu.vn)
 * @copyright  Copyright (c) 2025  
**************************************************************************************/

#ifndef SRC_GPS_DataReading_H
#define SRC_GPS_DataReading_H

#include <Arduino.h>

// ======================= Hằng số chuyển đổi đơn vị =======================
#define SRC_LAT_TO_CM        1.09469075f       // Tỉ lệ chuyển vĩ độ sang cm
#define SRC_LON_TO_CM        1.127819549f      // Tỉ lệ chuyển kinh độ sang cm
#define SRC_ALT_TO_CM        0.1f              // Tỉ lệ chuyển độ cao sang cm

// ======================= Tham số ổn định cơ sở RTK =======================
#define SRC_BASE_STABLE_DEFAULT_MAX   100      // Số lần cần để xác định vị trí base ổn định
#define SRC_BASE_STABLE_DEVIATION     4        // Độ lệch cho phép để xem là ổn định [cm]

// ======================= Trạng thái dữ liệu GPS ==========================
#define SRC_GPS_DATA_NOSOLUTION       0        // Không có tín hiệu GPS
#define SRC_GPS_DATA_FLOAT            1        // Tín hiệu chưa chính xác hoàn toàn (float)
#define SRC_GPS_DATA_FIX              2        // Tín hiệu đã chính xác (fixed)

// ======================= Chế độ định vị GNGGA ============================
#define SRC_GPS_GNGGA_FIX_MODE        4        // GNGGA fix mode
#define SRC_GPS_GNGGA_FLOAT_MODE      5        // GNGGA float mode

// ======================= Cấu trúc dữ liệu GPS RTK ========================
typedef struct {
  bool isNew;                    // Có dữ liệu mới hay không
  bool isGotBase;                // Đã xác định vị trí base hay chưa
  bool isError;                  // Có lỗi khi đọc chuỗi dữ liệu

  int16_t readErrCnt;           // Số lần đọc lỗi chuỗi
  int16_t readStrErrCnt;        // Số lần lỗi khi phân tích chuỗi
  uint8_t nosolutionCnt;        // Đếm số lần không có solution

  long stableCnt;               // Số lần liên tiếp có base ổn định
  long preTime;                 // Thời gian đọc trước đó (millis)
  long dt;                      // Khoảng thời gian giữa 2 lần đọc (ms)

  long RTK_Standard;            // Trạng thái RTK (0: no sol, 1: float, 2: fixed)
  
  long current_gps_long[4];     // [lat, lon, alt] - Đơn vị: lat/lon *10^7, alt [cm]
  long base_gps_long[4];        // Tọa độ base [lat, lon, alt]
  long pos_cm[4];               // Tọa độ x, y, z thực tế (cm)
  long pre_pos_cm[4];           // Tọa độ x, y, z trước đó (cm)
} srcrtk_data;


// ======================= Lớp xử lý dữ liệu GPS ===========================
class SRC_GPS_DataReading {
public:
  SRC_GPS_DataReading();                    // Constructor
  ~SRC_GPS_DataReading();                   // Destructor

  // Khởi tạo mô-đun đọc dữ liệu GPS
  bool setup(int BaseStableMax, HardwareSerial& GPSSerial);

  // Đọc dữ liệu GPS từ cổng Serial, cập nhật GPSData
  bool readData(HardwareSerial& GPSSerial); 

  // Phân tích chuỗi NMEA để lấy thông tin GNGGA
  bool ParseString(const String &input, const String &prefix, srcrtk_data *data);

  // Xử lý quá trình xác định base (tọa độ cố định)
  void BaseProcess(srcrtk_data *gnggaData);

  // Hàm chuyển đổi chuỗi sang số nguyên
  bool convertString2Long(String in, int len_in, long *out);

  // Kiểm tra một ký tự có phải số không, và chuyển thành giá trị số
  bool isStringNumber(char in, long *out);

  // Chuyển chuỗi vĩ độ (LA) dạng NMEA sang giá trị số thập phân
  bool convertLatitudetoDecimal(String LA, char latpos, long *outdata);

  // Chuyển chuỗi kinh độ (LO) dạng NMEA sang giá trị số thập phân
  bool convertLongitudetoDecimal(String LO, char lonpos, long *outdata);

  // Chuyển dữ liệu NMEA (gồm cả LA/LO) sang giá trị số thập phân
  bool convertNMEADatatoDecimal(String in, long *outdata);

  // Chuyển chuỗi phút sang kiểu long
  bool convertMinString2Long(String in, long *out); 

  srcrtk_data GPSData;     // Dữ liệu GPS được lưu sau mỗi lần đọc

private:
  long labs(long in);      // Hàm lấy giá trị tuyệt đối (long)
  int _BaseStableMax = SRC_BASE_STABLE_DEFAULT_MAX; // Ngưỡng ổn định base mặc định
};

#endif  // SRC_GPS_DataReading_H
