#include <Arduino.h>
#include <SoftwareSerial.h>

#define PIN_RX D5
#define PIN_TX D6

#ifndef SW_SERIAL_UNUSED_PIN
#define SW_SERIAL_UNUSED_PIN -1
#endif

SoftwareSerial serialRX(PIN_RX, SW_SERIAL_UNUSED_PIN);
SoftwareSerial serialTX(PIN_TX, SW_SERIAL_UNUSED_PIN);


void setup()
{
	pinMode(PIN_RX, INPUT);
	pinMode(PIN_TX, INPUT);

	Serial.begin(115200);
	serialRX.begin(9600);
	serialTX.begin(9600);

}

bool inRX = false;
bool inTX = false;
bool inSerial = false;
String sBuffer;

uint32_t lastMS = 0;
void loop()
{
	if (Serial.available())
	{
		if (!inSerial)
			sBuffer = String();
		inSerial = true;
		while (Serial.available())
		{
			sBuffer += String((char)Serial.read());
			sBuffer.trim();
		}
		
		if (sBuffer.length() == 35)
		{
			Serial.print("Got a proper length string");
			bool toESP = false;
			if (sBuffer.startsWith("TX:"))
			{
				toESP = true;
			}

			sBuffer = sBuffer.substring(3);

			SoftwareSerial serialRX_SEND(SW_SERIAL_UNUSED_PIN, PIN_RX);
			//SoftwareSerial serialTX_SEND(SW_SERIAL_UNUSED_PIN, PIN_TX);
			serialRX_SEND.begin(9600);

			if (toESP)
			{
				serialTX.enableRx(false);
				pinMode(PIN_TX, OUTPUT);

			}
			else
			{
				serialRX.enableRx(false);
				pinMode(PIN_RX, OUTPUT);
			}
			
			while (sBuffer.length() > 0)
			{
				String v = sBuffer.substring(0,2);
				sBuffer = sBuffer.substring(2);
				uint8_t byte = (uint8_t)strtol(v.c_str(), nullptr, 16);
				if (toESP)
					;//serialTX_SEND.write(byte);
				else
					serialRX_SEND.write(byte);

				Serial.printf("!! %02x !!",byte);
			}
			Serial.printf("\n");
			if (toESP)
			{
				pinMode(PIN_TX, INPUT);
				serialTX.enableRx(true);

			}
			else
			{
				pinMode(PIN_RX, INPUT);
				serialRX.enableRx(true);
			}

			inSerial = false;

		}
		else
		{
			//Serial.printf("Size was %d\n", sBuffer.length());
		}
		lastMS = millis();
	}

	if (serialRX.available())
	{
		if (inTX)
			Serial.print("\n");
		if (!inRX)
			Serial.print("RX: ");
		inRX = true;
		inTX = false;
		
		
		while (serialRX.available())
		{
			Serial.printf("%02x",serialRX.read());
			Serial.print(" ");
			lastMS = millis();
		}
	}
	
	if (serialTX.available())
	{
		if (inRX)
			Serial.print("\n");
		if (!inTX)
			Serial.print("TX: ");
		inRX = false;
		inTX = true;
		
		while (serialTX.available())
		{
			Serial.printf("%02x",serialTX.read());
			Serial.print(" ");
		}
		lastMS = millis();
	}

	if ((inRX || inTX || inSerial) && millis() - lastMS > 10)
	{
		inRX = false;
		inTX = false;
		if (inSerial)
			Serial.printf("Size was %d: %s\n", sBuffer.length(),sBuffer.c_str());
		inSerial = false;

		Serial.print("\n");
	}

}