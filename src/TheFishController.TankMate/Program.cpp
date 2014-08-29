#include <Arduino.h>
#include <Wire\Wire.h>
#include <LiquidCrystal\LCD.h>
#include <LiquidCrystal\LiquidCrystal_I2C.h>



#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define PIN_LCD_BACKLIGHT    3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7



#define PIN_LED_STRIP_RED 10
#define PIN_LED_STRIP_GREEN 11
#define PIN_LED_STRIP_BLUE 9
#define PIN_TEMP_SENSOR 0
#define PIN_PHOTO_RESISTOR 1



LiquidCrystal_I2C	lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);



struct colourValues
{
	int redValue;
	int greenValue;
	int blueValue;
} moonlight, daytime;



const int lightThreshold = 750;


//TODO: This is around reading the temperature 3 times for a reading
//TODO: but we only want to read the temperature every minute or so
//TODO: so need to add more of this...
long interval = 500;
long lastReadingMillis = 0;
int readingNumber = 0;


long temperatureReadingInterval = 6000;
long lastTemperatureReadingMillis = 0;
float cumulativeReadingTemperature;


float lastTempReding;
colourValues currentColour;



void defineColours(){

	moonlight.redValue = 0;
	moonlight.greenValue = 0;
	moonlight.blueValue = 128;

	daytime.redValue = 255;
	daytime.greenValue = 255;
	daytime.blueValue = 255;

}



void setup()
{
	
	Serial.begin(9600);

	//Some pin initialisation 
	pinMode(PIN_LED_STRIP_RED, OUTPUT);
	pinMode(PIN_LED_STRIP_GREEN, OUTPUT);
	pinMode(PIN_LED_STRIP_BLUE, OUTPUT);

	//Set last reading to -99 (unlikely to occur in an aquarium...)
	//so we know its not been read yet
	lastTempReding = -99;

	//Instantiate the known colours
	defineColours();

	//Initialise the LCD
	lcd.begin(16, 2);
	lcd.setBacklightPin(PIN_LCD_BACKLIGHT, POSITIVE);
	
	//Write out the header line
	lcd.home();
	lcd.clear();
	lcd.print("Fish Controller!");


	

	
}


float ReadTemperature(){


	float multiplier = ((5.0 * 1000) / 1024) / 10;


	int reading;
	

	unsigned long currentMillis = millis();

	Serial.print("Current millis: ");
	Serial.println(currentMillis);

	Serial.print("Last reading millis: ");
	Serial.println(lastReadingMillis);

	Serial.print("The sum: ");
	Serial.println(currentMillis - lastReadingMillis);


	if (currentMillis - lastReadingMillis > interval){

		lastReadingMillis = currentMillis;

		//Do a double read of the sensor
		reading = analogRead(PIN_TEMP_SENSOR);
		delayMicroseconds(100);
		reading = analogRead(PIN_TEMP_SENSOR);

		Serial.print("Temperature reading number: ");
		Serial.println(readingNumber);

		//add to the cumulative temp
		cumulativeReadingTemperature = cumulativeReadingTemperature + (reading * multiplier);

		readingNumber = readingNumber + 1;

		
	}


	//If we have taken 3 readings (0 to 2) then its time to return the actual reading value
	if (readingNumber == 3){

		Serial.println("3rd reading taken, returning temperature value");

		//reset the reading count
		readingNumber = 0;

		Serial.print("Temperature: ");
		Serial.println(cumulativeReadingTemperature / 3);

		float returnVal = cumulativeReadingTemperature / 3;
		cumulativeReadingTemperature = 0;

		return returnVal;

	}
	else{

		Serial.println("More readings to be taken before returning avg. value");

		//else return a predefined value which won't occur naturally (in an aquarium)
		return -99;

	}
	

}



void refreshDisplay(float temp)
{

	//Write everything out again encase there are some garbled characters due to voltage spikes or something
	lcd.home();
	lcd.print("Fish Controller!");
	lcd.setCursor(0, 1);
	lcd.print("Temp: ");
	lcd.setCursor(6, 1);
	lcd.print(temp, DEC);
	lcd.setCursor(11, 1);
	lcd.print("C    ");


	//If the temperature hasn't been read yet set it to the current value to prevent an erroneous alert
	if (lastTempReding = -99){

		lastTempReding = temp;

	}


	//If the temperature has changed by more than half a degree switch the back light on
	if ((temp > (lastTempReding + 1.0)) || (temp < (lastTempReding - 1.0)))
	{
		//TODO: perhaps an alert on a rapid temperature change - maybe take action like turn off the lights
	}
	
	
}


void setLEDColour(){



	int lightReading = analogRead(PIN_PHOTO_RESISTOR);

	Serial.println("Light Reading;");
	Serial.println(lightReading);


	if (lightReading < lightThreshold){

		//Daytime

		analogWrite(PIN_LED_STRIP_RED, 255);
		analogWrite(PIN_LED_STRIP_GREEN, 255);
		analogWrite(PIN_LED_STRIP_BLUE, 255);

	}
	else
	{

		//Night time

		analogWrite(PIN_LED_STRIP_RED, 0);
		analogWrite(PIN_LED_STRIP_GREEN, 0);
		analogWrite(PIN_LED_STRIP_BLUE, 128);
	}

}


bool shouldReadTemperature(){

	
	//Need a kind of between operator to allow the 3 readings
	if (millis() - lastTemperatureReadingMillis > temperatureReadingInterval){

		Serial.println("Should read temperature: true");

		return true;
			
	}
	else{

		return false;

	}


}


void pauseTemperatureReadings(){

	lastTemperatureReadingMillis = millis();

}


void propogateTempratureReading(float currentTemp){


	Serial.println(currentTemp);

	refreshDisplay(currentTemp);

	lastTempReding = currentTemp;


}


void loop()
{

	//setLEDColour();


	if (shouldReadTemperature()){

		//Read the temperature
		float temp = ReadTemperature();

		//The temperature returns on the 3rd reading for noise reduction
		//On every other reading it will return -99
		if (temp != -99){

			//We have a result so stop reading for a while
			pauseTemperatureReadings();

			//Do what we need to do with the reading (lcd, serial etc...)
			propogateTempratureReading(temp);

		}		

	}
	else {

		Serial.println("No temperature reading");

	}
	
	//TODO: Remove this delay
	delay(250);

}