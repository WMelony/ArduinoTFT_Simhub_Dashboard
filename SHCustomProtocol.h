/*
---------PROTOCOL FORMULA:-------------------------------------------
'' + round([PersistantTrackerPlugin.SessionBestLiveDeltaSeconds], 2) + 
';TC:' + [TCLevel] + ';BB:' + round([BrakeBias], 0) + ';L' + 
isnull([DataCorePlugin.GameData.NewData.CurrentLap], '0') + ';P' + 
isnull([DataCorePlugin.GameData.NewData.PlayerLeaderboardPosition], '1') + 
';' + isnull(format([CurrentLapTime], 'm\\:ss\\:fff'), '0:00:00') + ';' + 
format([DataCorePlugin.GameData.NewData.SpeedKmh],'0') + ';' + 
[CarSettings_RPMRedLineReached] + ';'   + round([ERSPercent], 0) + ';' + 
round([TyreTemperatureFrontLeft], 0) + 'C;' + round([TyreTemperatureFrontRight], 0) + 
'C;' + round([TyreTemperatureRearLeft], 0) + 'C;' + round([TyreTemperatureRearRight], 0) + 
'C;' + [DataCorePlugin.GameData.NewData.Gear] + '\n'
----------------------------------------------------------------------

TO ADD NEW FIELDS:
1. Add new enum value in DataFields (before FIELD_COUNT)
2. Update FIELD_COUNT
3. Add helper functions if needed (like getDelta(), getBattery())
4. Add change detection function (like deltaChanged())
5. Create update function for the new field
6. Call the update function in read()
7. Update SimHub protocol formula to include the new data
*/

#ifndef __SHCUSTOMPROTOCOL_H__
#define __SHCUSTOMPROTOCOL_H__

#include <Arduino.h>
#include <Adafruit_GFX.h>  // Core graphics library
#include <MCUFRIEND_kbv.h> // Hardware-specific library
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

MCUFRIEND_kbv tft;

#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define WHITE 0xFFFF
#define GREY 0x630c
#define PINK 0xCA36

class SHCustomProtocol {
private:
  // Centering Text/Display Handling
  void drawCenteredText(String text, int x, int y, int textSize)
  {
    tft.setTextSize(textSize);
    int16_t x1, y1;
    uint16_t w, h;
    char *textPtr = const_cast<char *>(text.c_str());
    tft.getTextBounds(textPtr, 0, 0, &y1, &y1, &w, &h);

    // Center Horizontally and Vertically Before Displaying
    int centeredX = x - (w / 2) - x1; // horiz offset
    int centeredY = y - (h / 2) - y1; // vert offset

    // Displaying
    tft.setCursor(centeredX, centeredY);
    tft.print(text);
  }

  // Clear old text before uppdating
  void clearCenteredText(String text, int x, int y, int textSize)
  {
    tft.setTextColor(BLACK);
    drawCenteredText(text, x, y, textSize); // black out old text
  }
  // Drawing new text w/ color
  void drawCenteredTextColor(String text, int x, int y, int textSize, uint16_t color)
  {
    tft.setTextColor(color);
    drawCenteredText(text, x, y, textSize);
  }

  // Telemetry Data
  enum dataFields
  {
    FIELD_DELTA = 0,   // lap delta
    FIELD_TC = 1,      // traction control
    FIELD_BB = 2,      // brake bias
    FIELD_LAP = 3,     // lap number
    FIELD_POS = 4,     // position
    FIELD_TIME = 5,    // curr lap time
    FIELD_SPEED = 6,   // curr speed
    FIELD_REDL = 7,    // redline bool
    FIELD_BATTERY = 8, // ers/battery percent

    // tyre tempuratures
    FIELD_TEMP_FL = 9,
    FIELD_TEMP_FR = 10,
    FIELD_TEMP_RL = 11,
    FIELD_TEMP_RR = 12,

    FIELD_GEAR = 13, // gear num

    // NEW FIELDS HERE OKAY?
    // FIELD_FUEL = 14,
    // FIELD_DRS = 15, blablabla

    FIELD_COUNT = 14 // update when adding more
  };

  String dataFields[FIELD_COUNT];
  String oldDataFields[FIELD_COUNT];
  bool fieldChanged[FIELD_COUNT];

  // check the custom protocol message to make sure all fields are complete
  bool parseMessage(String message)
  {
    // Store old values
    for (int i = 0; i < FIELD_COUNT; i++)
    {
      oldDataFields[i] = dataFields[i];
      fieldChanged[i] = false;
    }

    int fieldIndex = 0;
    int startPos = 0;

    // protocol messages seperate data by semicolons
    for (int i = 0; i <= message.length() && fieldIndex < FIELD_COUNT; i++)
    {
      if (i == message.length() || message.charAt(i) == ';')
      {
        if (fieldIndex < FIELD_COUNT)
        {
          dataFields[fieldIndex] = message.substring(startPos, i);
          dataFields[fieldIndex].trim(); // clean up

          // Check for changes
          if (dataFields[fieldIndex] != oldDataFields[fieldIndex])
          {
            fieldChanged[fieldIndex] = true;
          }

          fieldIndex++;
          startPos = i + 1;
        }
      }
    }

    return (fieldIndex >= FIELD_COUNT - 1); // Allow missing last field
  }

  // making sure the delta clears properly because it was very inconsistent annoying
  String getDelta() {
    float deltaValue = dataFields[FIELD_DELTA].toFloat();
    if (deltaValue > 0.001)
    {
      return "+" + String(deltaValue, 2);
    }
    else if (deltaValue < -0.001)
    {
      return String(deltaValue, 2);
    }
    else
    {
      return "0.0";
    }
  }
  String getOldDelta()
  {
    float deltaValue = oldDataFields[FIELD_DELTA].toFloat();
    if (deltaValue > 0.001)
    {
      return "+" + String(deltaValue, 2);
    }
    else if (deltaValue < -0.001)
    {
      return String(deltaValue, 2);
    }
    else
    {
      return "0.0";
    }
  }

  // idk if its more convinent to do this
  float getBattery()
  {
    return dataFields[FIELD_BATTERY].toFloat();
  }

  String getGear()
  {
    String gear = dataFields[FIELD_GEAR];
    // validate gear because for some reason it keeps displaying it with the tyre tempuratures
    if (gear.length() > 2)
    {
      gear = gear.substring(0, 1);
    }
    if (gear.length() == 0)
    {
      gear = "N";
    }
    return gear;
  }

  // idk if this is cleaner or faster
  bool deltaChanged()
  {
    return fieldChanged[FIELD_DELTA];
  }
  bool tcChanged()
  {
    return fieldChanged[FIELD_TC];
  }
  bool bbChanged()
  {
    return fieldChanged[FIELD_BB];
  }
  bool lapChanged()
  {
    return fieldChanged[FIELD_LAP];
  }
  bool posChanged()
  {
    return fieldChanged[FIELD_POS];
  }
  bool timeChanged()
  {
    return fieldChanged[FIELD_TIME];
  }
  bool speedChanged()
  {
    return fieldChanged[FIELD_SPEED];
  }
  bool redlChanged()
  {
    return fieldChanged[FIELD_REDL];
  }
  bool batteryChanged()
  {
    return fieldChanged[FIELD_BATTERY];
  }
  bool tempFLChanged()
  {
    return fieldChanged[FIELD_TEMP_FL];
  }
  bool tempFRChanged()
  {
    return fieldChanged[FIELD_TEMP_FR];
  }
  bool tempRLChanged()
  {
    return fieldChanged[FIELD_TEMP_RL];
  }
  bool tempRRChanged()
  {
    return fieldChanged[FIELD_TEMP_RR];
  }
  bool gearChanged()
  {
    return fieldChanged[FIELD_GEAR];
  }

  // ------------------------------ UPDATE LOGIC -------------------------------------------------------

  void updateGearAndSpeed() {
    String gear = getGear();
    String redl = dataFields[FIELD_REDL];
    static String lastGear = "";
    static String lastRedl = "";

    bool gearOrRedlChanged = (gear != lastGear) || (redl != lastRedl);

    if(gearOrRedlChanged) {
      // redraw background and if redline backg is green
      if (redl == "1" && gear != "N") {
        tft.fillRect(((tft.width() * 0.25 ) + 2), ((tft.height() * 0.25) + 2),
                    (tft.width() * 0.5) -4, (tft.height() * 0.5) -4, GREEN);
      } else {
        tft.fillRect(((tft.width() * 0.25 ) + 2), ((tft.height() * 0.25) + 2),
                    (tft.width() * 0.5) -4, (tft.height() * 0.5) -4, BLACK);
      }

      // Clear and redraw gear
      int gearCenterX = tft.width() * 0.5;
      int gearCenterY = tft.height() * 0.47;
      clearCenteredText(lastGear, gearCenterX, gearCenterY, 5);
      drawCenteredTextColor(gear, gearCenterX, gearCenterY, 5, WHITE);

      lastGear = gear;
      lastRedl = redl;
    }

    // Speed Logic
    if (speedChanged()) {
      String speed = dataFields[FIELD_SPEED];
      int speedCenterX = tft.width() * 0.5;
      int speedCenterY = tft.height() * 0.65;

      clearCenteredText(oldDataFields[FIELD_SPEED], speedCenterX, speedCenterY, 2);
      drawCenteredTextColor(speed, speedCenterX, speedCenterY, 2, GREY);
    }
  }

  void updateDelta() {
    // delta should have frequent updates for resposniveness
    String delta = getDelta();
    String oldDelta = getOldDelta();

    int deltaCenterX = tft.width() * 0.5;
    int deltaCenterY = 60;

    // Clear old text first using formatted old value so it actually clears
    clearCenteredText(oldDelta, deltaCenterX, deltaCenterY, 1.5);

    // Draw new delta with color based on float
    // ik theres some repetitive logic but im too lazy rn
    uint16_t deltaColor = GREY;
    float deltaValue = dataFields[FIELD_DELTA].toFloat();
    if (deltaValue > 0.001) {
      deltaColor = RED;
    } else if (deltaValue < -0.001) {
      deltaColor = GREEN;
    }

    drawCenteredTextColor(delta, deltaCenterX, deltaCenterY, 1.5, deltaColor);
  }

  void updateTemperature() {
    // Display tempuratures in square to represent tyre positions
    
    //Front Left
    if (tempFLChanged()) {
      int tempFLX = tft.width() * 0.82;
      int tempFLY = tft.height() * 0.45;

      clearCenteredText(oldDataFields[FIELD_TEMP_FL], tempFLX, tempFLY, 1.5);
      drawCenteredTextColor(dataFields[FIELD_TEMP_FL], tempFLX, tempFLY, 1.5, RED);

    }

    //Front Right
    if (tempFRChanged()) {
      int tempFRX = tft.width() * 0.93;
      int tempFRY = tft.height() * 0.45;

      clearCenteredText(oldDataFields[FIELD_TEMP_FR], tempFRX, tempFRY, 1.5);
      drawCenteredTextColor(dataFields[FIELD_TEMP_FR], tempFRX, tempFRY, 1.5, RED);      
    }

    //Rear Left
    if (tempRLChanged()) {
      int tempRLX = tft.width() * 0.82;
      int tempRLY = tft.height() * 0.65;

      clearCenteredText(oldDataFields[FIELD_TEMP_RL], tempRLX, tempRLY, 1.5);
      drawCenteredTextColor(dataFields[FIELD_TEMP_RL], tempRLX, tempRLY, 1.5, RED);

    }

    //Rear Right
    if (tempRRChanged()) {
      int tempRRX = tft.width() * 0.93;
      int tempRRY = tft.height() * 0.65;

      clearCenteredText(oldDataFields[FIELD_TEMP_RR], tempRRX, tempRRY, 1.5);
      drawCenteredTextColor(dataFields[FIELD_TEMP_RR], tempRRX, tempRRY, 1.5, RED);      
    }
  }

  void updateBattery() {
    if (batteryChanged()) {
      float battery = getBattery();

      tft.fillRect(0, ((tft.height() * 0.75) + 2), tft.width(), 80, BLACK);
      tft.fillRect(0, ((tft.height() * 0.75) + 2), tft.width() * (battery / 100), 80, GREEN);

      int batteryCenterX = tft.width() * 0.5;
      int batteryCenterY = tft.height() * 0.85;
      String bat = String(round(battery));
      drawCenteredTextColor(bat, batteryCenterX, batteryCenterY, 2, WHITE);
    }
  }

  void updateBrakeBalance() {
    if (bbChanged()) {
      // Center in left-middle area, aligned with TC
      int bbCenterX = tft.width() * 0.125;
      int bbCenterY = tft.height() * 0.45;

      clearCenteredText(oldDataFields[FIELD_BB], bbCenterX, bbCenterY, 2);
      drawCenteredTextColor(dataFields[FIELD_BB], bbCenterX, bbCenterY, 2, GREEN);
    }
  }

  void updateTractionControl() {
    if (tcChanged()) {
      // Center in left-middle area, aligned with TC
      int tcCenterX = tft.width() * 0.125;
      int tcCenterY = tft.height() * 0.55;

      clearCenteredText(oldDataFields[FIELD_TC], tcCenterX, tcCenterY, 2);
      drawCenteredTextColor(dataFields[FIELD_TC], tcCenterX, tcCenterY, 2, RED);
    }
  }

  void updateLapInfo() {
    if (lapChanged()) {
      int lapCenterX = tft.width() * 0.125;
      int lapCenterY = 40;

      clearCenteredText(oldDataFields[FIELD_LAP], lapCenterX, lapCenterY, 2);
      drawCenteredTextColor(dataFields[FIELD_LAP], lapCenterX, lapCenterY, 2, PINK);
    }
  }

  void updatePosition() {
    if (posChanged()) {
      int posCenterX = tft.width() * 0.875;
      int posCenterY = 40;

      clearCenteredText(oldDataFields[FIELD_POS], posCenterX, posCenterY, 2);
      drawCenteredTextColor(dataFields[FIELD_POS], posCenterX, posCenterY, 2, BLUE);
    }
  }

  void updateTime() {
    if (timeChanged()) {
      int timeCenterX = tft.width() * 0.5;
      int timeCenterY = 35;

      clearCenteredText(oldDataFields[FIELD_TIME], timeCenterX, timeCenterY, 2);
      drawCenteredTextColor(dataFields[FIELD_TIME], timeCenterX, timeCenterY, 2, GREEN);
    }
  }

public:
  /* default simhub context (idk why theres so much):
  CUSTOM PROTOCOL CLASS
  SEE https://github.com/zegreatclan/SimHub/wiki/Custom-Arduino-hardware-support

  GENERAL RULES :
    - ALWAYS BACKUP THIS FILE, reinstalling/updating SimHub would overwrite it with the default version.
    - Read data AS FAST AS POSSIBLE in the read function
    - NEVER block the arduino (using delay for instance)
    - Make sure the data read in "read()" function READS ALL THE DATA from the serial port matching the custom protocol definition
    - Idle function is called hundreds of times per second, never use it for slow code, arduino performances would fall
    - If you use library suspending interrupts make sure to use it only in the "read" function when ALL data has been read from the serial port.
      It is the only interrupt safe place

  COMMON FUNCTIONS :
    - FlowSerialReadStringUntil('\n')
      Read the incoming data up to the end (\n) won't be included
    - FlowSerialReadStringUntil(';')
      Read the incoming data up to the separator (;) separator won't be included
    - FlowSerialDebugPrintLn(string)
      Send a debug message to simhub which will display in the log panel and log file (only use it when debugging, it would slow down arduino in run conditions)

  */

  // Called when starting the arduino (setup method in main sketch)
  void setup()
  {
    uint16_t ID = tft.readID();
    if (ID == 0xD3D3)
      ID = 0x9481; // force ID if write-only display
    tft.begin(ID);
    tft.setRotation(1);
    tft.fillScreen(BLACK);
    tft.setFont(&FreeSansBold9pt7b);
  }

  // Called when new data is coming from computer
  void read()
  {
    /*
    // EXAMPLE 1 - read the whole message and sent it back to simhub as debug message
    // Protocol formula can be set in simhub to anything, it will just echo it
    // -------------------------------------------------------
    String message = FlowSerialReadStringUntil('\n');
    FlowSerialDebugPrintLn("Message received : " + message);

    */
    // -------------------------------------------------------
    // EXAMPLE 2 - reads speed and gear from the message
    // Protocol formula must be set in simhub to
    // format([DataCorePlugin.GameData.NewData.SpeedKmh],'0') + ';' + isnull([DataCorePlugin.GameData.NewData.Gear],'N')
    // -------------------------------------------------------

    String message = FlowSerialReadStringUntil('\n');

    if (!parseMessage(message))
    {
      FlowSerialDebugPrintLn("Parse error - expected " + String(FIELD_COUNT) + " fields");
      return; // skip failed protocol message
    }

    // Draw grid
    tft.drawFastHLine(0, (tft.height() * 0.25), tft.width(), WHITE);
    tft.drawFastHLine(0, (tft.height() * 0.75), tft.width(), WHITE);
    tft.drawFastVLine((tft.width() * 0.25), 0, tft.width(), WHITE);
    tft.drawFastVLine((tft.width() * 0.75), 0, tft.width(), WHITE);
    // Update for changed fields
    updateGearAndSpeed();
    updateDelta();
    updateTemperature();
    updateBattery();
    updateBrakeBalance();
    updateTractionControl();
    updateLapInfo();
    updatePosition();
    updateTime();
  }

  void loop() {
    // Called once per arduino loop
  }

  void idle() {
    // Called between each byte read - keep minimal
  }
};

#endif