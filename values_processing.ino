String getValue(String range) {
  if (wifiConnected) {
    bool success;
    FirebaseJson response;
    FirebaseJsonData result;
    while (!success) {


      success = GSheet.values.get(&response, spreadsheetId, range);
      response.get(result, "values/[0]/[0]");
    }
    return result.to<String>().c_str();
  } else {
    return "FALSE";
  }
}

void updateSettings() {
  if (wifiConnected) {
    if (millis() - updateTimer >= 10000) {
      updateTimer = millis();
      if (getValue("Settings!G2") == "TRUE") {

        for (int i = 1; i <= 6; i++) {
          lineData.retrieveData(i);
          char row[15];
          sprintf(row, "Settings!B%i:D%i", i + 1, i + 1);
          LineData receivedData = checkColumn(row);
          if (lineData != receivedData) {
            Serial.println("not matching, updating");
            Serial.println(receivedData.remainingLiquid);
            Serial.println(receivedData.setDose);
            Serial.println(receivedData.pourMillis);
            lineData.updateData(receivedData.remainingLiquid, receivedData.setDose, receivedData.pourMillis, i);
          }
        }
      }
    }
  }
}
void updateSettingsInGoogle() {
  if (wifiConnected) {
    for (int i = 1; i <= 6; i++) {
      lineData.retrieveData(i);
      char row[15];
      sprintf(row, "Settings!B%i:D%i", i + 1, i + 1);
      LineData receivedData = checkColumn(row);
      if (lineData != receivedData) {
        Serial.println("not matching, updating");
        Serial.println(receivedData.remainingLiquid);
        Serial.println(receivedData.setDose);
        Serial.println(receivedData.pourMillis);
        postRemainder(i);
      }
    }
  }
}

void postRemainder(int l) {
  if (wifiConnected) {
    char range[10];
    sprintf(range, "Settings!B%i", l + 1);
    postValue(String(lineData.remainingLiquid), range);
  }
}

void postValue(String dataToSend, String dataRange) {
  if (wifiConnected) {
    bool success;
    while (!success) {
      GSheet.ready();
      FirebaseJson response;
      FirebaseJson valueRange;

      valueRange.add("range", dataRange);
      valueRange.set("values/[0]/[0]", dataToSend);  // column 1/row 1
      success = GSheet.values.update(&response, spreadsheetId, dataRange, &valueRange);
    }
  }
}

LineData checkColumn(String range) {
  bool success = false;
  FirebaseJson response;
  FirebaseJsonData result;
  LineData data;

  while (!success) {
    success = GSheet.values.get(&response, spreadsheetId, range);

    if (success) {
      // Parse the first cell in the specified column
      response.get(result, "values/[0]/[0]");
      data.remainingLiquid = result.to<int>();

      // Parse the second cell in the specified column
      response.get(result, "values/[0]/[1]");
      data.setDose = result.to<int>();

      // Parse the third cell in the specified column
      response.get(result, "values/[0]/[2]");
      data.pourMillis = result.to<int>();
    }
  }

  return data;
}

void postLog(String t, String l, String o, String d, String r, bool ls) {
  if (wifiConnected) {
    GSheet.ready();
    FirebaseJson response;
    FirebaseJson valueRange;
    bool logSuccess = 0;
    while (!logSuccess) {
      if (millis() - pouringTimer >= lineData.pourMillis) {
        digitalWrite(relay_pins[l.toInt()-1], 1);
      }
    
    if (!ls) {
      getLastRow(0);
      valueRange.add("range", commonLoggerSheet + "!A" + String(rowToLog) + ":E" + String(rowToLog));
      valueRange.add("majorDimension", "ROWS");
      valueRange.set("values/[0]/[0]", t);  // column 1/row 1
      valueRange.set("values/[0]/[1]", l);  // column 2/row 1
      valueRange.set("values/[0]/[2]", o);  // column 3/row 1
      valueRange.set("values/[0]/[3]", d);  // column 4/row 1
      valueRange.set("values/[0]/[4]", r);  // column 5/row 1
      logSuccess = GSheet.values.update(&response, spreadsheetId, commonLoggerSheet + "!A" + String(rowToLog) + ":E" + String(rowToLog), &valueRange);
      response.toString(Serial, true);
      Serial.println();
    } else {
      getLastRow(l.toInt());
      valueRange.add("range", loggerSheet[l.toInt()] + "!A" + String(rowToLog) + ":D" + String(rowToLog));
      valueRange.add("majorDimension", "ROWS");
      valueRange.set("values/[0]/[0]", t);  // column 1/row 1
      valueRange.set("values/[0]/[1]", o);  // column 2/row 1
      valueRange.set("values/[0]/[2]", d);  // column 3/row 1
      valueRange.set("values/[0]/[3]", r);  // column 4/row 1
      logSuccess = GSheet.values.update(&response, spreadsheetId, loggerSheet[l.toInt()] + "!A" + String(rowToLog) + ":D" + String(rowToLog), &valueRange);
      response.toString(Serial, true);
      Serial.println();
    }
  }
}
}


void getTime() {
  if (wifiConnected) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      strftime(current_time, sizeof(current_time), "%H:%M:%S", &timeinfo);
      strftime(currentTimestamp, sizeof(currentTimestamp), "%d/%m/%y %H:%M:%S", &timeinfo);
      Serial.println(currentTimestamp);
    } else {
      Serial.println("Failed to obtain time");
    }
  }
}

void getLastRow(byte l) {
  if (wifiConnected) {
    bool success;

    FirebaseJson response;
    FirebaseJsonData result;
    while (!success) {
            if (millis() - pouringTimer >= lineData.pourMillis) {
        digitalWrite(relay_pins[l-1], 1);
      }

      Serial.println("Trying to get last row");
      // GSheet.ready();

      success = GSheet.values.get(&response, spreadsheetId, loggerSheet[l] + "!G1");
      Serial.println(success);
      response.get(result, "values/[0]/[0]");
    }
    rowToLog = result.to<int>() + 2;
  }
}