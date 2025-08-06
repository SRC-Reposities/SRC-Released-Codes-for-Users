/*************************************************
* CHUONG TRINH TEST MODULE NRTK
* Written by Dang Xuan Ba <badx@hcmute.edu.vn>
* Hardware: ARDUINO UNO/MEGA/DUE
* Date of version: 2025/08/06
*************************************************/
#include "SRC_GPS_DataReading.h"  // Thư viện doc du lieu RTK
#include <HardwareSerial.h>
#define RTKSerialPort Serial1

SRC_GPS_DataReading GPSRTKVar;
// ------------------- HÀM KHỞI TẠO -------------------
void setup() {	
	Serial.begin(115200);  //Cai dat cong UART0
	GPSRTKVar.setup(100, RTKSerialPort); //Cai dat module doc du lieu GPS RTK (100 ~ on dinh trong 10s)
  
}

// ------------------- VÒNG LẶP CHÍNH -------------------
void loop() {
  if(GPSRTKVar.readData(RTKSerialPort)){
    Serial.print("Lat: ");    Serial.print(GPSRTKVar.GPSData.current_gps_long[0]);
    Serial.print("\tLong: "); Serial.print(GPSRTKVar.GPSData.current_gps_long[1]);
    Serial.print("\tAlt: ");  Serial.print(GPSRTKVar.GPSData.current_gps_long[2]);

    Serial.print("\tx: "); Serial.print(GPSRTKVar.GPSData.pos_cm[0]);
    Serial.print("\ty: "); Serial.print(GPSRTKVar.GPSData.pos_cm[1]);
    Serial.print("\tz: "); Serial.print(GPSRTKVar.GPSData.pos_cm[2]);

    Serial.print("\tstatus: "); Serial.print(GPSRTKVar.GPSData.RTK_Standard);

    Serial.print("\tdt: "); Serial.println(GPSRTKVar.GPSData.dt);
  }

}
