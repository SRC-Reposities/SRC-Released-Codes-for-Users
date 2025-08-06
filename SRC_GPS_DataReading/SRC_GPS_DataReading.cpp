/*************************************************************************************  
 * @file 	SRC_GPS_DataReading.h
 * @brief 	Read GNGGA NMEA 0138 data from RTK Devices and parse them to detailed parameters such as lat, lon, alt, xw, yw, zw, status, dt			
			Hardware: Arduino, STM32F1, STM32F4
 * @version 1.1 
 * @date 	2025-08-06
 * @author 	Dang Xuan Ba (badx@hcmute.edu.vn)
 * @copyright Copyright (c) 2025  
**************************************************************************************/
#include "SRC_GPS_DataReading.h"

/*----------------------------------------------------------------------------------------------------------
* SRC_GNGGA
* Description: This class is used to parse the GNGGA data from the GPS module.
* Example:

    SRC_GNGGA orc_gngga;
    String test = "$GNGGA,023151.00,1048.6109280,N,10649.3255087,E,4,12,0.56,10.363,M,-2.076,M,1.0,2446*4C";
    GNGGA A;
    src_gngga.ParseString(test, "$GNGGA", &A);
    A.printData();

    Output: *1048.610928,10649.325508,10.363,3,2446#

----------------------------------------------------------------------------------------------------------*/
SRC_GPS_DataReading::SRC_GPS_DataReading() {
	
}

/********************************************
* ~SRC_GNGGA
* Description: Destructor for the ORC_GNGGA class.
********************************************/
SRC_GPS_DataReading::~SRC_GPS_DataReading() {
}
/*****************************************************
 * @brief Bắt đầu cho hệ thống hoạt động
 *****************************************************/
bool SRC_GPS_DataReading::setup(int BaseStableMax, HardwareSerial& GPSSerial) {
	GPSSerial.begin(115200); //Khoi tao toc do ket noi voi module GPS
	_BaseStableMax = (BaseStableMax <= 0)? 1: BaseStableMax;  
}

/*****************************************************
 * @brief Bắt đầu cho hệ thống hoạt động
 *****************************************************/
bool SRC_GPS_DataReading::readData(HardwareSerial& GPSSerial) {
	// Cac bien tamj
	bool result = false;
	//-- Reset bien trang thai du lieu moi
	GPSData.isNew = false;
	
	//Doc du lieu tu cong UART
	if(GPSSerial.available()){ //Co du lieu moi
		String gga = GPSSerial.readStringUntil('\n');  //Doc du lieu nhan ve
		if(ParseString(gga,"$GNGGA", &GPSData)){ //Khi tach du lieu thanh cong thi moi xu ly tiep, và tính toán sai lech so voi vi tri diem Base
		  //Xu ly Base
		  BaseProcess(&GPSData);
		  result = true;
		}		
	}	
	return result;

  
}

/***********************************************************************************************************
 * @brief Hàm kiểm tra độ ổn định của Base, nếu đã kiểm tra rồi thì bỏ qua
 * @param gnggaData dữ liệu gps hệ thống
 **********************************************************************************************************/
void SRC_GPS_DataReading::BaseProcess(srcrtk_data *gnggaData){
  if(!(gnggaData->isGotBase)){
    if(gnggaData->isNew){ //Chi xu ly tin hieu moi
      if(gnggaData->stableCnt == 0){ //Luu lai BASE
        gnggaData->base_gps_long[0] = gnggaData->current_gps_long[0];
        gnggaData->base_gps_long[1] = gnggaData->current_gps_long[1];
        gnggaData->base_gps_long[2] = gnggaData->current_gps_long[2];
      }      
		//-----START OF DEBUGGING------------------------------
		//Goi du lieu checkbase
		// Serial.printf("Checkbase: %ld,%ld,%ld,%d,%d#\n",
                    // gnggaData->pos_cm[0],
                    // gnggaData->pos_cm[1],
                    // gnggaData->pos_cm[2],
                    // gnggaData->RTK_Standard,
                    // gnggaData->stableCnt);
		//-----END OF DEBUGGING------------------------------
      if((labs(gnggaData->pos_cm[0]) <= SRC_BASE_STABLE_DEVIATION)&&(labs(gnggaData->pos_cm[1]) <= SRC_BASE_STABLE_DEVIATION)&&(labs(gnggaData->pos_cm[2]) <= SRC_BASE_STABLE_DEVIATION)){ //Kiem tra do dao dong
        gnggaData->stableCnt ++;
        if(gnggaData->stableCnt > _BaseStableMax){
          gnggaData->stableCnt = _BaseStableMax;
          gnggaData->isGotBase = true;
        }
      }else gnggaData->stableCnt = 0;
    }
  }
}

/**********************************************************************************************************
* ParseString
* Description: This function is used to parse the input string and store the data in the GNGGA struct.
* Parameters:
*   - const String &input: The input string to be parsed.
*   - const String &prefix: The prefix of the input string.
*   - GNGGA *data: The struct to store the parsed data.
* Return: True if the input string is parsed successfully, otherwise false.
-------
**********************************************************************************************************/
bool SRC_GPS_DataReading::ParseString(const String &input, const String &prefix, srcrtk_data *data) {
   if (!input.startsWith(prefix)) //Du lieu khong hop le --> thoat
    return false;

  int length = 10;
  String content = input.substring(prefix.length() + 1, input.length() - 1); //Lay chuoi du lieu chinh
  int i = 0;
  int start = 0;
  int end = content.indexOf(',');
  bool result = false;
  long timeTemp = 0;
  long fixType = 0;
  String dataString[20];
  //Tach cac cum du lieu de vao mang
  while (end != -1 && i < length) {
    dataString[i] = "";
    dataString[i++] = content.substring(start, end);   
    start = end + 1;
    end = content.indexOf(',', start);
  }

  if (i < length) {
    dataString[i++] = content.substring(start);
  }

  //Kiem tra du lieu nhan ve co hop le khong
  if(((dataString[2][0]=='N')||(dataString[2][0]=='S'))&&((dataString[4][0]=='W')||(dataString[4][0]=='E'))&&(dataString[9][0]=='M')){
    //Buoc 1: Chuyen du lieu Altitude
    if(convertString2Long(dataString[8], -1, &(data->current_gps_long[2]))){      
      //Buoc 2: chuyen du lieu lat
      if(convertLatitudetoDecimal((dataString[1]), dataString[2][0], &(data->current_gps_long[0]))){    
        //Buoc 3: chuyen doi du lieu long
        if(convertLongitudetoDecimal(dataString[3], dataString[4][0], &(data->current_gps_long[1]))){         
          //Buoc 5: chuyen doi du lieu chat luong
          if(convertString2Long(dataString[5], -1, &fixType)){
			//Mapping Quality
			if(fixType == SRC_GPS_GNGGA_FIX_MODE)
			  data->RTK_Standard = SRC_GPS_DATA_FIX;
			else if (fixType == SRC_GPS_GNGGA_FLOAT_MODE)
				data->RTK_Standard = SRC_GPS_DATA_FLOAT;
			else data->RTK_Standard = SRC_GPS_DATA_NOSOLUTION;
			
			//tinh do dich chuyen
			data->pos_cm[0] = (data->current_gps_long[0] - data->base_gps_long[0])*SRC_LAT_TO_CM;
			data->pos_cm[1] = -(data->current_gps_long[1] - data->base_gps_long[1])*SRC_LON_TO_CM;
			data->pos_cm[2] = (data->current_gps_long[2] - data->base_gps_long[2])*SRC_ALT_TO_CM;	 
			  
			data->isNew = true;              
            timeTemp = millis();
            data->dt = timeTemp - data->preTime;
            data->preTime = timeTemp;
			result = true;
         //   Serial.println("Done!");
		 
          }
        }

      }

    }
  }
  return result;  
}

/*---------------------------------------------------------------------------------------------------------
* convertLatitudetoDecimal
* Description: This function is used to convert the latitude from the GPS module to decimal format.
* Parameters:
*   - String LATITUDE: The latitude from the GPS module.
*   - char latpos: The position of the latitude.
* Return: The latitude in decimal format.
-------
---------------------------------------------------------------------------------------------------------*/
bool SRC_GPS_DataReading::convertLatitudetoDecimal(String LA, char latpos, long *outdata) {
  long tempOUt = 0;
  if(convertNMEADatatoDecimal(LA, &tempOUt)){
    *outdata = (latpos == 'S')?(-tempOUt):tempOUt;
    return true;
  }return false;
}

/*---------------------------------------------------------------------------------------------------------
* convertLongitudetoDecimal
* Description: This function is used to convert the longitude from the GPS module to decimal format.
* Parameters:
*   - String LONGITUDE: The longitude from the GPS module.
*   - char lonpos: The position of the longitude.
* Return: The longitude in decimal format.
-------
---------------------------------------------------------------------------------------------------------*/
bool SRC_GPS_DataReading::convertLongitudetoDecimal(String LO, char lonpos, long *outdata) {
  long tempOUt = 0;
  if(convertNMEADatatoDecimal(LO, &tempOUt)){
    *outdata = (lonpos == 'W')?(-tempOUt):tempOUt;
    return true;
  }return false;
}

/*---------------------------------------------------------------------------------------------------------
* convertNMEADatatoDecimal
* Description: This function is used to convert the longitude from the GPS module to decimal format.
* Parameters:
*   - String LONGITUDE: The longitude from the GPS module.
*   - char lonpos: The position of the longitude.
* Return: The longitude in decimal format.
-------
---------------------------------------------------------------------------------------------------------*/
bool SRC_GPS_DataReading::convertNMEADatatoDecimal(String in, long *outdata) {
  String inString = in, hhString = "", mmString = "";
  int dotPoint = inString.indexOf('.');
  long hhTemp = 0, mmTemp = 0;
  if (dotPoint < 2){ //Chuoi bi loi: dau cham nam sai vi tri
    return false;
  }else if (dotPoint == 2) //Khong co du lieu gio: mm.mmmmmmm (tong cong 10 ky tu)
  {
    if(inString.length()==10) //dung gia tri  => chuyen ve gia tr gio
    {      
      return (convertMinString2Long(inString, outdata));
    }else return false; //sai dinh dang du lieu
  }else{
    //Tach du lieu gio va phut
    hhString = inString.substring(0, dotPoint - 2);
    mmString = inString.substring(dotPoint - 2, inString.length());
    //Serial.println(hhString); Serial.println(mmString);
    //Chuyen sang kieu long
    if((convertString2Long(hhString, -1, &hhTemp)) && (convertMinString2Long(mmString, &mmTemp))){
      *outdata = hhTemp*1e7 + mmTemp;
      return true;
    }else return false;
  }
}

/*---------------------------------------------------------------------------------------------------------
* isStringNumber
* Description: This function is used to check if the input character is a number.
* Parameters:
*   - int *out: The output value.
*   - char in: The input character.
* Return: True if the input character is a number, otherwise false.
-------
---------------------------------------------------------------------------------------------------------*/
bool SRC_GPS_DataReading::isStringNumber(char in, long *out) {
  switch (in) {
    case '0':
      *out = 0;
      return true;
    case '1':
      *out = 1;
      return true;
    case '2':
      *out = 2;
      return true;
    case '3':
      *out = 3;
      return true;
    case '4':
      *out = 4;
      return true;
    case '5':
      *out = 5;
      return true;
    case '6':
      *out = 6;
      return true;
    case '7':
      *out = 7;
      return true;
    case '8':
      *out = 8;
      return true;
    case '9':
      *out = 9;
      return true;
    default:
      return false;
  }
}

/*---------------------------------------------------------------------------------------------------------
* convertMinString2Long
* Description: This function is used to convert the input string to a long value.
* Parameters:
*   - String in: The input string.
* Return: The long value of the input string.
-------
---------------------------------------------------------------------------------------------------------*/
bool SRC_GPS_DataReading::convertMinString2Long(String in, long *out) {
  String string_in = in;
  int len = string_in.length(); 
  long temp = 0; 
  if(len == 10){
    if (convertString2Long(string_in, 10, &temp)){
      *out = (temp/60);
      return true;
    }else return false;
  }else return false;
}

/*---------------------------------------------------------------------------------------------------------
* convertString2Long
* Description: This function is used to convert the input string to a long value.
* Parameters:
*   - String in: The input string.
* Return: The long value of the input string.
-------
---------------------------------------------------------------------------------------------------------*/
bool SRC_GPS_DataReading::convertString2Long(String in, int len_in, long *out) {
  String string_in = in, data = "";
  int len = string_in.length();
  long temp_out = 0, temp_char = 0;
  bool checking;
  //Lay du lieu chieu dai du lieu muon convert: neu len_in <=0 thi lay chieu dai chuoi trc tiep
  if(len_in > 0) 
    len = len_in;
  for (int i = 0; i<len;i++)
  {   
    
    if(isStringNumber(in[i], &temp_char)){  //Kiem tra ky tu co phai dang so ko
      temp_out = temp_out * 10 + temp_char;
    } else if(in[i] != '.') //Neu khong phai dang so va cung ko phai dau cham thi ngung tim va bao loi
    {
      return false;
    }
  }
  *out = temp_out;
  return true;
}


/***********************************************************
 * @brief Hàm tính trị tuyệt đối của kiểu Long
 * @param in đầu vào kiểu Long
 **********************************************************/
long SRC_GPS_DataReading::labs(long in){
  return ((in < 0)? (-in):in);
}