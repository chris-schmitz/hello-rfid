// * Pull in dependencies
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currenttouched = 0;

// * Define constants for the music maker wing
#define VS1053_RESET -1
#define VS1053_CS 6
#define VS1053_DCS 10
#define VS1053_DREQ 9
#define CARDCS 5
#define DEFAULT_VOLUME 0
#define USING_MUSIC_MAKER_WING true

// * Define constants for the RFID reader
#define SS_PIN 19  // Configurable, see typical pin layout above
#define RST_PIN 11 // Configurable, see typical pin layout above
// ! Note that I'm using the Feather M0 Express for this project and to get the
// ! card reader to work I had to use something other than the SDA pin. I have no idea why,
// ! but I did. While troubleshooting I found this answer on stack exchange that didn't directly
// ! apply to my issue, but it did suggest that you could use any pin for the select and not just
// ! the SDA pin:
// ? https://arduino.stackexchange.com/a/58030/35534

// * Define constants for neopixel bar
#define NEOPIXEL 12
#define ONBOARD_PIXEL 8
#define TOTAL_PIXELS 8

// * Instantiate our classes
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Adafruit_NeoPixel bar = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel onboardPixel = Adafruit_NeoPixel(1, ONBOARD_PIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

// * setup variables for the RFID tags we're looking for
String content = "";
String targetTag1 = "04 72 d2 4a e6 4c 81";
String targetTag2 = "04 52 c5 4a e6 4c 80";

// * setup pokemon RFID tags
String bulbasaur = "04 60 d0 4a e6 4c 81";
String charmander = "04 78 d2 4a e6 4c 81";
String squirtle = "04 59 d0 4a e6 4c 81";
String pikachu = "04 60 d1 4a e6 4c 81";

void setup()
{
	Serial.begin(9600); // Initialize serial communications with the PC
	// while (!Serial)
	// ; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	setupRFIDCardReader();
	setupNeopixelBar();
	setupMusicMakerWing();
	signalReady();
	onboardPixel.begin();

	if (!cap.begin(0x5A))
	{
		Serial.println("Couldn't find the cap touch sensor");
		while (1)
			;
	}
	Serial.println("Found cap touch sensor");

	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void setupMusicMakerWing()
{
	if (!USING_MUSIC_MAKER_WING)
	{
		return;
	}

	if (!player.begin())
	{
		Serial.println(F("Could not find the music maker wing. Either add the wing or turn the wing usage off in the code."));
		while (1)
			;
	}

	Serial.println(F("Music Maker wing found"));

	if (!SD.begin(CARDCS))
	{
		Serial.println(F("SD card either failed or not found."));
		while (1)
			;
	}
	Serial.println("SD Card Ok");
	// printDirectory(SD.open("/"), 0);

	player.setVolume(DEFAULT_VOLUME, DEFAULT_VOLUME);
	player.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

	// player.sineTest(0x44, 500);
}

void setupRFIDCardReader()
{
	SPI.begin();					   // Init SPI bus
	mfrc522.PCD_Init();				   // Init MFRC522
	mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
									   // Serial.println(mfrc522.PCD_GetAntennaGain());
									   // mfrc522.PCD_SetAntennaGain(mfrc522.PCD_RxGain.RxGain_48dB);
									   // Serial.println(mfrc522.PCD_GetAntennaGain());
}

void setupNeopixelBar()
{
	Serial.println("starting neopixel bar");
	bar.begin();
}

void signalReady()
{
	for (int i = 0; i <= TOTAL_PIXELS; i++)
	{
		bar.setPixelColor(i, 89, 209, 14);
		bar.show();
		delay(90);
	}
	for (int i = TOTAL_PIXELS; i >= 0; i--)
	{
		bar.setPixelColor(i, 0, 0, 0);
		bar.show();
		delay(90);
	}
}

void handleCapTouch(uint8_t index)
{
	Serial.print("Touched pin ");
	Serial.println(index);

	if (index == 0)
	{
		player.stopPlaying();
	}
	if (index == 1)
	{
		player.stopPlaying();
		player.startPlayingFile("songs/track001.mp3");
	}
	if (index == 2)
	{
		player.stopPlaying();
		player.startPlayingFile("songs/track002.mp3");
	}
	if (index == 3)
	{
		player.stopPlaying();
		player.startPlayingFile("songs/track003.mp3");
	}
	if (index == 4)
	{
		player.stopPlaying();
		player.startPlayingFile("songs/track004.mp3");
	}
}

bool readACardOnThePreviousLoop = false;

void loop()
{

	// 	* Added in for cap touch testing
	// currenttouched = cap.touched();
	// for (uint8_t i = 0; i < 12; i++)
	// {
	// 	if ((currenttouched & _BV(i)) && !(lasttouched & _BV(i)))
	// 	{
	// 		handleCapTouch(i);
	// 	}
	// }

	// lasttouched = currenttouched;
	// return;

	// // ! temp disabiling the rest of the sketch so I can test out cap touch player

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
	else if (content.substring(1) == charmander)
	{

		fillBar(255, 0, 0);
		readACardOnThePreviousLoop = true;
		player.playFullFile("POKEMON/CHAR.mp3");
	}
	else if (content.substring(1) == bulbasaur)
	{

		fillBar(0, 255, 0);
		readACardOnThePreviousLoop = true;
		player.playFullFile("POKEMON/BULB.mp3");
	}
	else if (content.substring(1) == squirtle)
	{
		fillBar(0, 0, 255);
		readACardOnThePreviousLoop = true;
		player.playFullFile("POKEMON/SQUIRT.mp3");
	}
	else if (content.substring(1) == pikachu)
	{
		fillBar(255, 255, 0);
		readACardOnThePreviousLoop = true;
		player.playFullFile("POKEMON/PIKA.mp3");
	}
	else
	{
		Serial.println("no tag match found");
		readACardOnThePreviousLoop = false;
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
void printDirectory(File dir, int numTabs)
{
	while (true)
	{

		File entry = dir.openNextFile();
		if (!entry)
		{
			// no more files
			//Serial.println("**nomorefiles**");
			break;
		}
		for (uint8_t i = 0; i < numTabs; i++)
		{
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if (entry.isDirectory())
		{
			Serial.println("/");
			printDirectory(entry, numTabs + 1);
		}
		else
		{
			// files have sizes, directories do not
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
	}
}