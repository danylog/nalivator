void initWifi(){
   // WiFiManager to handle Wi-Fi connection
  WiFiManager wifiManager;
  if(button()== 0){
    
  wifiManager.setConfigPortalTimeout(1);  // Set timeout for the config portal (optional)
  }else{
    lcd.clear();
      lcd.setCursor(0, 0);
  lcd.print("WiFi setup");
    wifiManager.resetSettings();
    wifiManager.setConfigPortalTimeout(60);
  }
  // Try to auto-connect to the last known Wi-Fi
  if (wifiManager.autoConnect("ESP8266_AP")) {
    wifiConnected = true;
    Serial.println("Connected to Wi-Fi.");
  } else {
    wifiConnected = false;
    Serial.println("Failed to connect to Wi-Fi. Running in offline mode.");
  }
=
  // In case SD/SD_MMC storage file access, mount the SD/SD_MMC card.
  // SD_Card_Mounting(); // See src/GS_SDHelper.h
if(wifiConnected){
  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

  // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

  // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
  // delay(10000);
  
   configTime(0, 0, ntpServer);
  setenv("TZ", MY_TZ, 1);  // Set timezone environment variable
  tzset();  // Apply timezone setting
  }


}

void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}