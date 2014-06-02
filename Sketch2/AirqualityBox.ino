

#include <string.h>
#include <Wire.h>
#include <WProgram.h>
#include <DS1307.h>
int RTCValues[7];
char TimeStr[9];
char DateStr[9];

#include <DSM501.h>
#define DSM501_PM10	3
#define DSM501_PM25	2
DSM501 dsm501(DSM501_PM10, DSM501_PM25);

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

#include <dht11.h>
dht11 DHT11;
#define DHT11PIN 4
float LastTemp,LastHum,LastDew;
long PrevDHTMillis, PrevTimeMillis, PrevPMMillis;
double PM25Wgt,PM10Wgt;
boolean Disp25;



void setup()
{
	Serial.begin(115200);
	lcd.begin(16,2);
	DS1307.begin();
	dsm501.begin();
	lcd.backlight();
	dsm501.reset();
	DHT_ReadData();
	Display_DHT();
	PrevTimeMillis = millis();
	PrevPMMillis = millis();
	PrevDHTMillis = millis();
	
	  
}

char BCD2DEC(char var){
	if (var>9){
		var=(var>>4)*10+(var&0x0f);
	}
	return var;
}

void Time_Read ()
{
	DS1307.getDate(RTCValues);	
}

void DisplayTime()
{
	sprintf(TimeStr, "%02d:%02d:%02d", RTCValues[4], RTCValues[5], RTCValues[6]);
	sprintf(DateStr, "%02d-%02d-%02d", RTCValues[0], RTCValues[1], RTCValues[2]);
	lcd.setCursor(0,1);
	dsm501.update();
	lcd.print(TimeStr);
	dsm501.update();
	//lcd.print(RTCValues[0]);lcd.print("/");
	//lcd.print(RTCValues[1]);lcd.print("/");
	//lcd.print(RTCValues[2]);
	lcd.setCursor(0,0);
	dsm501.update();
	lcd.print(DateStr);
	//lcd.print(RTCValues[4]);lcd.print(":");
	//lcd.print(RTCValues[5]);lcd.print(":");
	//lcd.print(RTCValues[6]);
}

double dewPointFast(double celsius, double humidity)
{
	double a = 17.271;
	double b = 237.7;
	double temp = (a * celsius) / (b + celsius) + log(humidity/100);
	double Td = (b * temp) / (a - temp);
	return Td;
}

void DHT_ReadData ()
{
	int chk = DHT11.read(DHT11PIN);
	dsm501.update();
	
	switch (chk)
	{
		case 0: 
		LastTemp = (float) DHT11.temperature;
		LastHum = (float) DHT11.humidity;
		LastDew = (float) dewPointFast(LastTemp,LastHum);
		
		break;
		case -1: Serial.println("Checksum error"); break;
		case -2: Serial.println("Time out error"); break;
		default: Serial.println("Unknown error"); break;
	}
}

void Display_DHT ()
{
	lcd.setCursor(9,0);
	dsm501.update(); 
	lcd.print(DHT11.temperature);
	dsm501.update();
	lcd.print("C ");
	dsm501.update(); 
	lcd.print(DHT11.humidity); 
	dsm501.update();
	lcd.print("%");
}

void Display_PM25 ()
{
	lcd.setCursor(9,1);
	dsm501.update();
	lcd.print("2:");
	dsm501.update();
	lcd.print(PM25Wgt);
}

void Display_PM10 ()
{
	lcd.setCursor(9,1);
	dsm501.update();
	lcd.print("1:");
	dsm501.update();
	lcd.print(PM10Wgt);
}

void Serial_Sendstuff ()
{
	Serial.print(DateStr); Serial.print(" "); Serial.print(TimeStr); Serial.print(" ");
	Serial.print(LastTemp); Serial.print("C "); Serial.print(LastHum); Serial.print("% ");
	Serial.print(LastDew); Serial.print("C ");
	Serial.print("PM25(1): "); Serial.print(PM25Wgt); Serial.print("ug/m3 ");
	Serial.print("PM10(1): "); Serial.print(PM10Wgt); Serial.print("ug/m3 ");
	Serial.print("PM25(2): "); Serial.print(dsm501.getParticalWeight2(0)); Serial.print("ug/m3 ");
	Serial.print("PM10(2): "); Serial.print(dsm501.getParticalWeight2(1)); Serial.print("ug/m3 ");
	Serial.print("AQI: "); Serial.println(dsm501.getAQI());
}

void loop()
{
	dsm501.update();
	if ((millis() - PrevTimeMillis) > 1000)
	{
		Time_Read();
		dsm501.update();
		DisplayTime();
		dsm501.update();
		PrevTimeMillis = millis();
	}
	if ((millis() - PrevDHTMillis) > 5000)
	{
		DHT_ReadData();
		dsm501.update();
		Display_DHT();
		dsm501.update();
		PrevDHTMillis = millis();
		if (Disp25)
		{
			Display_PM25();
			Disp25 = false;
		}else
		{
			Display_PM10();
			Disp25 = true;
		}
	}
	if((millis() - PrevPMMillis) > 30000)
	{
			PM25Wgt = dsm501.getParticalWeight2(0);
			PM10Wgt = dsm501.getParticalWeight2(1);
			Serial_Sendstuff();
			PrevPMMillis = millis();	
	}

}
