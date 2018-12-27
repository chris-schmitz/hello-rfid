/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 *
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 *
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 *
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 *
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>

#define RST_PIN 16 // Configurable, see typical pin layout above
#define SS_PIN 25  // Configurable, see typical pin layout above

#define NEOPIXEL 15
#define TOTAL_PIXELS 8

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Adafruit_NeoPixel bar = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL, NEO_GRB + NEO_KHZ800);

String content = "";
String targetTag1 = "04 72 d2 4a e6 4c 81";
String targetTag2 = "04 52 c5 4a e6 4c 80";

void setup()
{
	Serial.begin(9600); // Initialize serial communications with the PC
	while (!Serial)
		;							   // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();					   // Init SPI bus
	mfrc522.PCD_Init();				   // Init MFRC522
	mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

	bar.begin();

	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

bool readACardOnThePreviousLoop = false;

void loop()
{

	// * Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent())
	{
		if (readACardOnThePreviousLoop != true)
		{
			emptyBar();
		}
		readACardOnThePreviousLoop = false;
		return;
	}

	// * Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial())
	{
		return;
	}

	// * Dump debug info about the card; PICC_HaltA() is automatically called
	// mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

	captureUID();
	Serial.println("Got tag id:");
	Serial.println(content.substring(1));

	// ! Note that we need to start reading the sub string at index 1 instead of 0
	// ! otherwise we get a space at the beginning
	if (content.substring(1) == targetTag1)
	{
		Serial.println("found tag 1");
		readACardOnThePreviousLoop = true;
		fillBar(0, 255, 0);
	}
	else if (content.substring(1) == targetTag2)
	{
		Serial.println("found tag 2");
		readACardOnThePreviousLoop = true;
		fillBar(0, 255, 255);
	}
	else
	{
		readACardOnThePreviousLoop = false;
		Serial.println("no tag match found");
	}
}

void fillBar(uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t i;
	for (i = 0; i < TOTAL_PIXELS; i++)
	{
		bar.setPixelColor(i, r, g, b);
	}
	bar.show();
}
void emptyBar()
{
	uint8_t i;
	for (i = 0; i < TOTAL_PIXELS; i++)
	{
		bar.setPixelColor(i, 0, 0, 0);
	}
	bar.show();
}

void captureUID()
{
	content = "";
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		// Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
		// Serial.print(mfrc522.uid.uidByte[i], HEX);

		content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
		content.concat(String(mfrc522.uid.uidByte[i], HEX));
	}
	// Serial.println();
}
