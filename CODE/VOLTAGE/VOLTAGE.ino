#include <LiquidCrystal.h>

const int rs = 12, en = 13, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#define BTN 2
#define BUZZER 3
#define LEDPIN 4

int vol_Welcome = 100;
int vol_Switch  = 80;
int vol_Tester  = 100; 

int menu = 0;   // 0=Voltmeter | 1=Diode Test
unsigned long lastPress = 0, lastUpdate = 0;
bool lastBtnState = HIGH;

byte ohmChar[8]   = {0b01110,0b10001,0b10001,0b10001,0b01010,0b01010,0b11111,0b00000};
byte diodeChar[8] = {0b00000,0b11011,0b10111,0b11111,0b10111,0b11011,0b00000,0b00000};

void playTone(int mode, int duration = 0) {
  int level = 0;
  if (mode == 1) level = vol_Welcome;
  if (mode == 2) level = vol_Switch;
  if (mode == 3) level = vol_Tester;
  if (level == 0) { digitalWrite(BUZZER, LOW); return; }
  int delayMicros = map(level, 0, 100, 1000, 80); 
  if (duration > 0) {
    long elapsed = 0;
    while (elapsed < duration * 1000L) {
      digitalWrite(BUZZER, HIGH); delayMicroseconds(delayMicros);
      digitalWrite(BUZZER, LOW);  delayMicroseconds(delayMicros);
      elapsed += delayMicros * 2;
    }
  } else {
    digitalWrite(BUZZER, HIGH); delayMicroseconds(delayMicros);
    digitalWrite(BUZZER, LOW);  delayMicroseconds(delayMicros);
  }
}

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  lcd.begin(8, 2);
  lcd.createChar(0, ohmChar);
  lcd.createChar(2, diodeChar);
  nokiaBoot();
}

void loop() {
  checkButton(); // Permanent button check

  // Fix: Manage menus in a single structure to avoid time conflicts
  if (menu == 1) {
    diodeTest();
  } 
  else {
    noTone(BUZZER); // We make sure that the diode test sound does not sound in the voltmeter menu.
    if (millis() - lastUpdate > 400) {
      voltMeter();
      lastUpdate = millis();
    }
  }
}

void nokiaBoot() {
  lcd.clear();
  int nokiaP[] = {100, 90, 60, 75};
  for (int i = 0; i <= 2; i++) {
    lcd.clear();
    lcd.setCursor(i, 0); lcd.print("MKIT");
    vol_Welcome = nokiaP[i];
    playTone(1, 120);
    delay(200);
  }
  vol_Welcome = 90;
  delay(1000); 
  lcd.setCursor(3, 1); lcd.print("M2");
  playTone(1, 150);
  delay(500);
  for (int j = 0; j < 3; j++) {
    lcd.noDisplay(); delay(300);
    lcd.display();
    playTone(1, 150);
    delay(300);
  }
  lcd.clear();
}

void voltMeter() {
  lcd.setCursor(1, 0); lcd.print("VOLTS:  ");
  digitalWrite(LEDPIN, LOW);

  // 1. Read the calibration value from A3
  // This value determines how much to subtract from the final number (between 0 and 1 V).
  float offset = map(analogRead(A3), 0, 1023, 0, 100) / 100.0; 

  long sum = 0;
  for(int i=0; i<100; i++) sum += analogRead(A1);
  float avgAdc = sum / 100.0;

  // 2. Calculation of main voltage
  float Vin = (avgAdc * 5.0 / 1023.0) * 49.78; 
  
  // 3. Applying balance: Subtracting the error amount (Offset)
  Vin = Vin - offset;

  // 4. Prevent negative numbers from being displayed
  if (Vin < 0.1) Vin = 0.0;

  lcd.setCursor(0, 1);
  lcd.print("V:"); lcd.print(Vin, 1); lcd.print("V   ");
}

void diodeTest() {
  lcd.setCursor(0, 0); lcd.print(" TESTER:  ");
  int adc = analogRead(A2);
  lcd.setCursor(0, 1);
  lcd.write(byte(2));
  if(adc < 450) { 
    lcd.print(" Thorny ");
    digitalWrite(LEDPIN, HIGH);
    tone(BUZZER, 2700);
  } else {
    lcd.print(" OPEN   ");
    digitalWrite(LEDPIN, LOW);
    noTone(BUZZER); // Mute when open
  }
}

void checkButton() {
  bool btnState = digitalRead(BTN);
  if(btnState == LOW && lastBtnState == HIGH) {
    if(millis() - lastPress > 250) {
      noTone(BUZZER); // Fix: Mute any sound when changing menu
      playTone(2, 50);
      menu++;
      if(menu > 1) menu = 0;
      lcd.clear();
      lastPress = millis();
    }
  }
  lastBtnState = btnState;
}