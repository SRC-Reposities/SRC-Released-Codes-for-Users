/*************************************************************************************  
 * @file 	SRC_GPS_DataReading.h
 * @brief 	Read GNGGA NMEA 0138 data from RTK Devices and parse them to detailed parameters such as lat, lon, alt, xw, yw, zw, status, dt			
			Hardware: Arduino, STM32F1, STM32F4
 * @version 1.1 
 * @date 	2025-08-06
 * @author 	Dang Xuan Ba (badx@hcmute.edu.vn)
 * @copyright Copyright (c) 2025  
**************************************************************************************/
#ifndef SRC_GPS_DataReading_H
#define SRC_GPS_DataReading_H
#include <Arduino.h>

//=======================GPS Parameters==================
#define SRC_LAT_TO_CM 1.09469075f
#define SRC_LON_TO_CM 1.127819549f
#define SRC_ALT_TO_CM 0.1f
#define SRC_BASE_STABLE_DEFAULT_MAX 100 //10 lan
#define SRC_BASE_STABLE_DEVIATION 4 //[cm]
#define SRC_GPS_DATA_NOSOLUTION 0 //No solution
#define SRC_GPS_DATA_FLOAT 		1 //No solution
#define SRC_GPS_DATA_FIX 		2 //No solution
#define SRC_GPS_GNGGA_FIX_MODE 4
#define SRC_GPS_GNGGA_FLOAT_MODE 5

	
typedef struct{
	bool isNew, isGotBase, isError;
	int16_t readErrCnt, readStrErrCnt;  
	uint8_t nosolutionCnt;
	long stableCnt;
	long preTime;
	long dt;
	long RTK_Standard;                      // trang thai gps: nosolution-0, float-1, fixed-2  
	long current_gps_long[4]; 				//lat*10^7, long*10^7, z in [cm]
	long base_gps_long[4]; 					//lat*10^7, long*10^7, z in [cm]
	long pos_cm[4];                        	// vi tri x,y in [cm], z in [cm]
	long pre_pos_cm[4];                     // vi tri x,y in [cm], z in [cm]
} srcrtk_data;

class SRC_GPS_DataReading
{
public:
  SRC_GPS_DataReading();
  ~SRC_GPS_DataReading();
	bool setup(int BaseStableMax, HardwareSerial& GPSSerial);
	bool readData(HardwareSerial& GPSSerial); 
	bool ParseString(const String &input, const String &prefix, srcrtk_data *data);
	void BaseProcess(srcrtk_data *gnggaData);
	bool convertString2Long(String in, int len_in, long *out);

	bool isStringNumber(char in, long *out);
	bool convertLatitudetoDecimal(String LA, char latpos, long *outdata);
	bool convertLongitudetoDecimal(String LO, char lonpos, long *outdata);
	bool convertNMEADatatoDecimal(String in, long *outdata);
	bool convertMinString2Long(String in, long *out); 
  
	srcrtk_data GPSData;
  private:
	long labs(long in);
	int _BaseStableMax = SRC_BASE_STABLE_DEFAULT_MAX;
};
#endif  // CONFIG_H
