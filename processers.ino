void pourLiquid(byte l) {
  digitalWrite(relay_pins[l - 1], 0);
  if (pour_flag) {
    pour_flag = 0;
    postRemainder(l);

    getTime();
    postLog(currentTimestamp, String(line), "POUR", String(lineData.setDose), String(lineData.remainingLiquid), 0);
    postLog(currentTimestamp, String(line), "POUR", String(lineData.setDose), String(lineData.remainingLiquid), 1);
  }

  if (millis() - pouringTimer >= lineData.pourMillis) {
    digitalWrite(relay_pins[l - 1], 1);
    lcd.clear();
    screen = MAIN_SCREEN;
  }
}

int button() {
  int a = analogRead(0);

  if (700 > a and a > 660) {
    return 8;
  } else if (640 > a and a > 600) {
    return 7;
  } else if (600 > a and a > 510) {
    return 6;
  } else if (510 > a and a > 410) {
    return 5;
  } else if (410 > a and a > 310) {
    return 4;
  } else if (310 > a and a > 210) {
    return 3;
  } else if (210 > a and a > 100) {
    return 2;
  } else if (100 > a) {
    return 1;
  } else {
    return 0;
  }
}
