#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#include <ESP8266WiFi.h>
#include <ESP_Google_Sheet_Client.h>
#include <WiFiManager.h>

#include <GS_SDHelper.h>
bool wifiConnected;

const char* ntpServer = "pool.ntp.org";

#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"

#include <time.h>
char currentTimestamp[20];
char current_time[9];


String loggerSheet [7] = {"Log", "Line1", "Line2", "Line3", "Line4", "Line5", "Line6"};

String commonLoggerSheet = "Log";

#define PROJECT_ID "alcohol-dosator"

String spreadsheetId = "1crW59HxztzZEH567qcHX6kxFHuHkufd-ai3O8IucMTc";

#define CLIENT_EMAIL "esp-logger@alcohol-dosator.iam.gserviceaccount.com"

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQD4QiaGGp8RXHcj\nopJeE84Oz5Ft1DDefbbWO8lvUeeR09JmOPbt0vWgYvfs7Mjr3nnt/78HA4Y3xipC\nuN5mklJYJ6HSJrYjbha47QDoIDZkqKkKQ9koU9U2fV7ecQ1O/8GqBPL4EX+xS+QU\nFI1dqiM0vNzOyoGkaKGRCg+aPnJGmqKkQTsPSs4lsm6/l0eVcX7p82ZaNd0HY2nr\nB9ChsOFnQ+5mBJgtTXQEBNxvR9hY4DAGasoRM9tTVO6nPAPax0vccq8nydd1pJsN\nZ+68N9Ej5f2LoSrKtvv2o93vuFgLGQ+8URJNQLMUCZ6iOxjAaMaaOb2QRuIIyzs3\ngRijDpsTAgMBAAECggEABnVqHSkKJGB8dP/gwPPQ84+w3+OaVd99Zzts9SlzVdbl\n7WTj/7HyU/bfLlnGrGRAVdHGfX84+bB9HvlCR/bcj5CQZDA/otJVYwA/esqDv8Cn\njS3Qs3zi2iUO18inKPv8ZNj2HUAPS2SHDYKvXtVzTwbu065vCwDx2x0g5MllHbMR\nFAJFtUkhGwiFgrLeb4WzY86Rm7q2qKCjRRBrHgopGnJpGAS2/veyIWn3FK8DndYc\n56nh4KBieeLZjVNHqk562HLhmnMSuWsGOTu4MUZfGD5Rm+HqJrWoDp+Qxd0iyrHi\n/Y7ARbwdKDdugx1gvOP3ZLpC6nZ6FlF+8IPAEixYCQKBgQD+Cyux60RarsfrNBZs\n6SA05rbzlELq96bw9v1atfvLHi1U30ml788G52fuhtfwtHvotfm5I5y6i6FAPfXM\nwR/AhME7rMZT5EKcoP8Lr+k9NflCsjgLLSDHaCcloyrCPCJbLwplkPT19rKtfBp/\n5mYqFf7q97YwfLBuU7pYfADW/QKBgQD6K5MZzF0tUlYWWvLyCYnB51Cn4WCk/Cwy\nbdtJjwZx7xLDzaDRjlGq9Kjd5R6hZV7pgZJRQ7wy6DwZShGqUyOK5x/NEYJcmy4Y\nz4PZcIUs/JO3ELxzJ0zRkWWJsGoNWb9ZMi0Gc0Q/ZGuz9Qu/swQ4aif1K8zR3AuY\nyYwbALQ/TwKBgQC/NUszA+AT3P+OEUtbt+/MwWYsWYgwcCwPq+cvUk/tJo56FgHJ\nyoBpMtzwFEAN5NpqYiVOavZ5FcVNpBUQgr9SWNlUAAvacrCMUUUF1+ZWcvxkbd1u\n1A2a6NKmKnRxc29scDCEF6G4S3Fa0FDluVVmEd4nnkBbqE3nDE/yb4r2RQKBgQCo\nTFI8w9BXYhHKHktcavBDA1OubR+wPnmZP3CLzu1eDqDIGvi6oYztfoRwKuhtWZ39\nZw7BHtloQfXUaZDyzdWWZ6BsPABAPqFHTat8x4bAWfch699rdJ/oSHFPrN/btWnW\nHrxNn5PlNbCDMzJBN7R6I2hYNqwm7mra0ZQwwkuAXwKBgCh8m0Eh9EQy9yMoe7ko\nNKrny4Snm+zTGHLK35yfx+XTmD+CIZqHG6f2wOEikKpS6DW/mB+nM+TTu3TG0ff/\nWMhppMFKlnKpNUzp+NVDrJpiBm3EebZlfwdnUF7jIuMdto/aXUvI86dJRpGYE+AG\nUWXR5zidQfcZ16t/JPg4Q9GZ\n-----END PRIVATE KEY-----\n";

void tokenStatusCallback(TokenInfo info);

#define MAIN_SCREEN 0
#define POUR_SCREEN 1
#define CALIB_SETUP 2
#define CALIB_PERFORM 3
#define CALIB_CONFIRM 4
#define RESET_BAG 5
#define PURGE_AIR 6
#define FORCE_RESET 7

#define BAG_AMOUNT_ML 10000
#define DEFAULT_DOSE 150
#define DEFAULT_TIME 2000

bool pour_flag = 0;

uint32_t pouringTimer, calibTime, updateTimer, rowToLog;

int relay_pins[6] = { 14, 12, 16, 2, 3, 13 };
byte screen = 0;
byte line, dosesToggle;

byte doses[3] = { 50, 100, 150 };



LiquidCrystal_I2C lcd(0x26, 16, 2);

class LineData {
public:
  unsigned int remainingLiquid;
  unsigned int setDose;
  unsigned int pourMillis;

  LineData()
    : remainingLiquid(0), setDose(0), pourMillis(0) {}

  void retrieveData(int l) {
    EEPROM.get(l * 12 + 0, remainingLiquid);
    EEPROM.get(l * 12 + 4, setDose);
    EEPROM.get(l * 12 + 8, pourMillis);
  }
  void updateData(int a, int b, int c, int l) {
    EEPROM.put(l * 12 + 0, a);
    EEPROM.put(l * 12 + 4, b);
    EEPROM.put(l * 12 + 8, c);
    EEPROM.commit();
  }
  void saveCalib(int l) {

    EEPROM.put(l * 12 + 4, setDose);
    EEPROM.put(l * 12 + 8, pourMillis);
    EEPROM.commit();
  }
  void resetBag(int l) {
    unsigned int amount = BAG_AMOUNT_ML;
    remainingLiquid = amount;
    EEPROM.put(l * 12, amount);
    EEPROM.commit();
  }
  void setDefault(int l) {
    unsigned int a = BAG_AMOUNT_ML;
    unsigned int b = DEFAULT_DOSE;
    unsigned int c = DEFAULT_TIME;

    EEPROM.put(l * 12, a);
    EEPROM.put(l * 12 + 4, b);
    EEPROM.put(l * 12 + 8, c);
    EEPROM.commit();
  }
  void pour(int l) {
    remainingLiquid = remainingLiquid - setDose;

    EEPROM.put(l * 12, remainingLiquid);
    EEPROM.commit();
  }
  bool operator==(const LineData& other) const {
    return remainingLiquid == other.remainingLiquid && setDose == other.setDose && pourMillis == other.pourMillis;
  }

  // Overload the inequality operator (optional)
  bool operator!=(const LineData& other) const {
    return !(*this == other);
  }
};

LineData lineData;

void setup() {

  lcd.begin();
  Serial.begin(115200);
  lcd.setCursor(0, 0);
  lcd.print("loading...");
  initWifi();

  EEPROM.begin(512);
  // for (int i = 1; i < 7; i++) {
  //   lineData.setDefault(i);
  // }

  for (int i = 0; i < 6; i++) {
    Serial.println(relay_pins[i]);
    pinMode(relay_pins[i], OUTPUT);

    digitalWrite(relay_pins[i], 1);
  }


  getTime();
  postValue("FALSE", "Settings!F2");
  postLog(currentTimestamp, "SYSTEM", "ON", "", "", 0);
  updateSettingsInGoogle();
  lcd.clear();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) wifiConnected = 0;
  Serial.println(screen);
  switch (screen) {
    case MAIN_SCREEN:
      lcd.setCursor(0, 0);
      lcd.print(" PRESS A BUTTON ");
      lcd.setCursor(0, 1);
      lcd.print("    TO POUR");
      if (!wifiConnected) lcd.print("    X");
      if (button() != 0) {


        if (button() == 7 and getValue("Settings!F2") == "TRUE") {
          lcd.clear();
          screen = CALIB_SETUP;
        } else if (button() == 8) {
          lcd.clear();
          screen = RESET_BAG;
        } else if (button() <= 6 and button() >= 1) {
          lcd.clear();
          line = button();
          pouringTimer = millis();
          lineData.retrieveData(line);
          lineData.pour(line);
          if (lineData.remainingLiquid > 10000) {
            lineData.remainingLiquid = 0;
            screen = FORCE_RESET;
          } else {
            screen = POUR_SCREEN;
          }
          pour_flag = 1;
        }
        delay(200);
      }
      break;

    case FORCE_RESET:
      lcd.setCursor(0, 0);
      lcd.print("Please Reset the");
        lcd.setCursor(0, 1);
      lcd.print("bag-in-box ");
      lcd.print(line);
      delay(2000);
      lcd.clear();
      screen = MAIN_SCREEN;
      break;
    case POUR_SCREEN:

      lcd.setCursor(0, 0);
      lcd.print("Line ");
      lcd.print(line);
      lcd.print(" - ");
      lcd.print(lineData.setDose);
      lcd.print(" ml");
      lcd.setCursor(0, 1);
      lcd.print(lineData.remainingLiquid);
      lcd.print(" ml left");
      pourLiquid(line);
      break;
    case CALIB_SETUP:
      updateSettings();
      lcd.setCursor(0, 0);
      lcd.print("CALIBRATION MODE");
      lcd.setCursor(0, 1);
      lcd.print("Amount: ");
      lcd.print(doses[dosesToggle]);
      lcd.print("ml");
      if (button() == 7) {
        dosesToggle = dosesToggle == 2 ? 0 : dosesToggle + 1;
        lcd.setCursor(7, 1);
        lcd.print("       ");
        delay(200);
      }
      if (button() >= 1 and button() <= 6) {
        lcd.clear();

        screen = CALIB_PERFORM;
        calibTime = millis();
        line = button();
        digitalWrite(relay_pins[line - 1], 0);
      }
      break;
    case CALIB_PERFORM:
      lcd.setCursor(0, 0);
      lcd.print("CALIBRATING...");
      lcd.setCursor(0, 1);
      lcd.print("LINE ");
      lcd.print(line);
      lcd.print(" ");
      lcd.print(doses[dosesToggle]);
      lcd.print("ml");
      if (button() == 0) {
        digitalWrite(relay_pins[line - 1], 1);
        lineData.pourMillis = millis() - calibTime;
        lineData.setDose = doses[dosesToggle];
        lcd.clear();
        screen = CALIB_CONFIRM;
      }
      break;
    case CALIB_CONFIRM:
      lcd.setCursor(0, 0);
      lcd.print("LINE ");
      lcd.print(line);
      lcd.print(" ");
      lcd.print(lineData.setDose);
      lcd.print("ml");
      lcd.setCursor(0, 1);
      lcd.print(lineData.pourMillis * 0.001);
      lcd.print("s.");
      lcd.print(" Confirm?");
      if (button() == line) {
        digitalWrite(relay_pins[line - 1], 0);
        lcd.clear();
        screen = CALIB_PERFORM;
        calibTime = millis();
      }
      if (button() == 7) {
        lineData.saveCalib(line);
        char dosageRange[12];
        sprintf(dosageRange, "Settings!C%i", (line + 1));

        char pourRange[12];
        sprintf(dosageRange, "Settings!C%i", (line + 1));
        sprintf(pourRange, "Settings!D%i", (line + 1));
        postValue(String(lineData.setDose), dosageRange);
        postValue(String(lineData.pourMillis), pourRange);

        getTime();
                postLog(currentTimestamp, String(line), "Calibrate", String(lineData.setDose), String(lineData.pourMillis), 0);
        postLog(currentTimestamp, String(line), "Calibrate", String(lineData.setDose), String(lineData.pourMillis), 1);
        lcd.clear();

        screen = MAIN_SCREEN;
      }
      break;
    case RESET_BAG:
      lcd.setCursor(0, 0);
      lcd.print("Select a line");
      if (button() == 8) {
        delay(200);
        lcd.clear();
        screen = MAIN_SCREEN;
      }
      if (button() >= 1 and button() <= 6) {
        delay(100);
        lcd.clear();
        screen = PURGE_AIR;
        line = button();
      }
      break;
    case PURGE_AIR:
      lcd.setCursor(0, 0);
      lcd.print("HOLD TO PURGE");
      lcd.setCursor(0, 1);
      lcd.print("AIR");
      if (button() == line) {
        digitalWrite(relay_pins[line - 1], 0);
      } else {
        digitalWrite(relay_pins[line - 1], 1);
      }
      if (button() == 8) {

        delay(200);

        lineData.resetBag(line);

        postRemainder(line);
                postLog(currentTimestamp, String(line), "Reset Bag", "", "10000", 0);
        postLog(currentTimestamp, String(line), "Reset Bag", "", "10000", 1);
        lcd.clear();
        screen = MAIN_SCREEN;
      }
      break;
  }
}
