#include "SRC_GPS_DataReading.h"

//====================== Constructor ======================
// Hàm khởi tạo đối tượng đọc GPS RTK
SRC_GPS_DataReading::SRC_GPS_DataReading() { }

//====================== Destructor =======================
// Hàm hủy (hiện không sử dụng tài nguyên động nên để trống)
SRC_GPS_DataReading::~SRC_GPS_DataReading() { }

//====================== setup() ==========================
// Khởi tạo giao tiếp với GPS và thiết lập số lần cần base ổn định
bool SRC_GPS_DataReading::setup(int BaseStableMax, HardwareSerial& GPSSerial) {
	GPSSerial.begin(115200); // Bắt đầu giao tiếp với tốc độ 115200 bps
	_BaseStableMax = (BaseStableMax <= 0) ? 1 : BaseStableMax; // Nếu <=0 thì đặt tối thiểu là 1
	return true;
}

//====================== readData() =======================
// Đọc một dòng dữ liệu từ GPS RTK qua Serial và xử lý
bool SRC_GPS_DataReading::readData(HardwareSerial& GPSSerial) {
	bool result = false;
	GPSData.isNew = false;  // Reset cờ dữ liệu mới

	if (GPSSerial.available()) { // Nếu có dữ liệu
		String gga = GPSSerial.readStringUntil('\n'); // Đọc đến khi gặp '\n'

		// Nếu parse thành công chuỗi GNGGA thì xử lý base và trả kết quả true
		if (ParseString(gga, "$GNGGA", &GPSData)) {
			BaseProcess(&GPSData);
			result = true;
		}
	}
	return result;
}

//====================== BaseProcess() ====================
// Kiểm tra vị trí hiện tại có đủ ổn định để đặt làm base không
void SRC_GPS_DataReading::BaseProcess(srcrtk_data *gnggaData) {
	if (!gnggaData->isGotBase && gnggaData->isNew) { // Chỉ xử lý nếu chưa có base và có dữ liệu mới
		if (gnggaData->stableCnt == 0) {
			// Lưu lại toạ độ hiện tại làm base
			for (int i = 0; i < 3; i++)
				gnggaData->base_gps_long[i] = gnggaData->current_gps_long[i];
		}

		// Kiểm tra độ dao động tọa độ hiện tại so với base có nhỏ hơn ngưỡng ổn định không
		if (labs(gnggaData->pos_cm[0]) <= SRC_BASE_STABLE_DEVIATION &&
			labs(gnggaData->pos_cm[1]) <= SRC_BASE_STABLE_DEVIATION &&
			labs(gnggaData->pos_cm[2]) <= SRC_BASE_STABLE_DEVIATION) {

			gnggaData->stableCnt++; // Tăng số lần ổn định liên tiếp
			if (gnggaData->stableCnt > _BaseStableMax) {
				gnggaData->stableCnt = _BaseStableMax;
				gnggaData->isGotBase = true; // Đánh dấu đã có base
			}
		} else {
			gnggaData->stableCnt = 0; // Reset nếu dao động quá lớn
		}
	}
}

//====================== ParseString() ====================
// Tách chuỗi GNGGA thành các phần dữ liệu như tọa độ, chất lượng, độ cao...
bool SRC_GPS_DataReading::ParseString(const String &input, const String &prefix, srcrtk_data *data) {
	if (!input.startsWith(prefix)) return false; // Nếu không phải chuỗi GNGGA thì bỏ qua

	String content = input.substring(prefix.length() + 1, input.length() - 1); // Cắt bỏ "$GNGGA," và ký tự cuối
	String dataString[20];
	int i = 0, start = 0, end = content.indexOf(',');

	// Tách chuỗi thành các phần nhỏ lưu vào mảng
	while (end != -1 && i < 10) {
		dataString[i++] = content.substring(start, end);
		start = end + 1;
		end = content.indexOf(',', start);
	}
	if (i < 10) dataString[i++] = content.substring(start);

	// Kiểm tra định dạng cơ bản của chuỗi GNGGA
	if (((dataString[2][0] == 'N') || (dataString[2][0] == 'S')) &&
		((dataString[4][0] == 'W') || (dataString[4][0] == 'E')) &&
		dataString[9][0] == 'M') {

		long fixType = 0;

		// Bước 1: Đọc độ cao (altitude)
		if (convertString2Long(dataString[8], -1, &data->current_gps_long[2]) &&
			// Bước 2: Đọc vĩ độ
			convertLatitudetoDecimal(dataString[1], dataString[2][0], &data->current_gps_long[0]) &&
			// Bước 3: Đọc kinh độ
			convertLongitudetoDecimal(dataString[3], dataString[4][0], &data->current_gps_long[1]) &&
			// Bước 4: Chuyển định dạng chất lượng (float/fix)
			convertString2Long(dataString[5], -1, &fixType)) {

			// Mapping chất lượng GPS
			if (fixType == SRC_GPS_GNGGA_FIX_MODE)
				data->RTK_Standard = SRC_GPS_DATA_FIX;
			else if (fixType == SRC_GPS_GNGGA_FLOAT_MODE)
				data->RTK_Standard = SRC_GPS_DATA_FLOAT;
			else
				data->RTK_Standard = SRC_GPS_DATA_NOSOLUTION;

			// Tính độ lệch so với base
			data->pos_cm[0] = (data->current_gps_long[0] - data->base_gps_long[0]) * SRC_LAT_TO_CM;
			data->pos_cm[1] = -(data->current_gps_long[1] - data->base_gps_long[1]) * SRC_LON_TO_CM;
			data->pos_cm[2] = (data->current_gps_long[2] - data->base_gps_long[2]) * SRC_ALT_TO_CM;

			// Đánh dấu có dữ liệu mới và cập nhật thời gian
			data->isNew = true;
			long timeTemp = millis();
			data->dt = timeTemp - data->preTime;
			data->preTime = timeTemp;

			return true;
		}
	}
	return false;
}

//====================== convertLatitudetoDecimal() ==============
// Chuyển vĩ độ (1048.610928) sang số thập phân (dạng long *10^7)
bool SRC_GPS_DataReading::convertLatitudetoDecimal(String LA, char latpos, long *outdata) {
	long tmp = 0;
	if (convertNMEADatatoDecimal(LA, &tmp)) {
		*outdata = (latpos == 'S') ? -tmp : tmp;
		return true;
	}
	return false;
}

//====================== convertLongitudetoDecimal() ==============
// Chuyển kinh độ (10649.325508) sang số thập phân (long *10^7)
bool SRC_GPS_DataReading::convertLongitudetoDecimal(String LO, char lonpos, long *outdata) {
	long tmp = 0;
	if (convertNMEADatatoDecimal(LO, &tmp)) {
		*outdata = (lonpos == 'W') ? -tmp : tmp;
		return true;
	}
	return false;
}

//====================== convertNMEADatatoDecimal() ==============
// Chuyển chuỗi định dạng NMEA (VD: 1048.610928) → 10.8101° * 1e7 = 108101821
bool SRC_GPS_DataReading::convertNMEADatatoDecimal(String in, long *outdata) {
	int dotPoint = in.indexOf('.');
	if (dotPoint < 2) return false;

	if (dotPoint == 2 && in.length() == 10) {
		// Nếu chỉ có phút → chuyển trực tiếp
		return convertMinString2Long(in, outdata);
	} else {
		// Nếu có đầy đủ độ và phút
		String hhString = in.substring(0, dotPoint - 2); // phần độ
		String mmString = in.substring(dotPoint - 2);    // phần phút

		long hhTemp = 0, mmTemp = 0;
		if (convertString2Long(hhString, -1, &hhTemp) &&
			convertMinString2Long(mmString, &mmTemp)) {
			*outdata = hhTemp * 1e7 + mmTemp;
			return true;
		}
	}
	return false;
}

//====================== isStringNumber() ==========================
// Kiểm tra ký tự có phải là chữ số từ '0' đến '9'
bool SRC_GPS_DataReading::isStringNumber(char in, long *out) {
	switch (in) {
		case '0': *out = 0; return true;
		case '1': *out = 1; return true;
		case '2': *out = 2; return true;
		case '3': *out = 3; return true;
		case '4': *out = 4; return true;
		case '5': *out = 5; return true;
		case '6': *out = 6; return true;
		case '7': *out = 7; return true;
		case '8': *out = 8; return true;
		case '9': *out = 9; return true;
		default: return false;
	}
}

//====================== convertMinString2Long() ===================
// Chuyển phút từ chuỗi `"48.610928"` sang long: phút * 1e7 / 60
bool SRC_GPS_DataReading::convertMinString2Long(String in, long *out) {
	if (in.length() == 10) {
		long tmp;
		if (convertString2Long(in, 10, &tmp)) {
			*out = tmp / 60;
			return true;
		}
	}
	return false;
}

//====================== convertString2Long() ======================
// Chuyển chuỗi chứa số và dấu '.' thành số nguyên
bool SRC_GPS_DataReading::convertString2Long(String in, int len_in, long *out) {
	int len = (len_in > 0) ? len_in : in.length();
	long result = 0, tmpChar = 0;
	for (int i = 0; i < len; i++) {
		if (isStringNumber(in[i], &tmpChar)) {
			result = result * 10 + tmpChar;
		} else if (in[i] != '.') {
			return false;
		}
	}
	*out = result;
	return true;
}

//====================== labs() ====================================
// Trị tuyệt đối của một số kiểu long
long SRC_GPS_DataReading::labs(long in) {
	return (in < 0) ? -in : in;
}
