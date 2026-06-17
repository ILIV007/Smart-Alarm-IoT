/*
  پروژه: دزدگیر هوشمند - بخش ATmega328P (نسخه‌ی نهایی صنعتی)
  ✅ رفع substring(10) به substring(8)
  ✅ حذف delay مسدودکننده با تایمر غیرمسدود
  ✅ ذخیره‌ی آخرین وضعیت LED برای بازیابی
  ✅ اعمال TRIGGERED حتی هنگام لمس
  ✅ نادیده گرفتن لمس‌های نویزی زیر 50ms
  ✅ پر کردن بافر فیلتر قبل از کالیبراسیون
*/

const int PIN_RED = 9;
const int PIN_GREEN = 10;
const int PIN_BLUE = 11;
const int PIN_YELLOW = 12;

const int PIN_HALL = A0;
const int PIN_LDR = A1;
const int PIN_TOUCH = 4;
const int PIN_BUZZER = 5;

const unsigned long TAP_SHORT_MIN_MS = 50;
const unsigned long TAP_SHORT_MAX_MS = 500;
const unsigned long TAP_LONG_MS = 2000;

int hallBase = 512, ldrBase = 500;
bool calibrated = false;

int lastTouchState = LOW;
int currentTouchState = LOW;
unsigned long touchStartTime = 0;
bool isTouching = false;

// ✅ متغیرهای جدید برای رفع باگ‌ها
String lastStateLED = "DISARMED";  // ذخیره آخرین وضعیت
unsigned long alarmLEDEnd = 0;     // تایمر غیرمسدود برای نمایش قرمز
bool alarmShowing = false;

int hallSamples[5] = {0}, hallIdx = 0;
int getHallFiltered() {
  hallSamples[hallIdx] = analogRead(PIN_HALL);
  hallIdx = (hallIdx + 1) % 5;
  long sum = 0;
  for(int i=0; i<5; i++) sum += hallSamples[i];
  return sum / 5;
}

int ldrSamples[5] = {0}, ldrIdx = 0;
int getLdrFiltered() {
  ldrSamples[ldrIdx] = analogRead(PIN_LDR);
  ldrIdx = (ldrIdx + 1) % 5;
  long sum = 0;
  for(int i=0; i<5; i++) sum += ldrSamples[i];
  return sum / 5;
}

bool blinking = false;
int bR, bG, bB, bY;
unsigned long bInt, bLast;
bool bState = false;

// ✅ حذف delayMicroseconds برای جلوگیری از فلش خاموش
void setLEDs(int r, int g, int b, int y) {
  analogWrite(PIN_RED, r);
  analogWrite(PIN_GREEN, g);
  analogWrite(PIN_BLUE, b);
  digitalWrite(PIN_YELLOW, (y > 127) ? HIGH : LOW);
}

void setup() {
  pinMode(PIN_TOUCH, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RED, OUTPUT); 
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT); 
  pinMode(PIN_YELLOW, OUTPUT);
  
  Serial.begin(9600);
  lastTouchState = digitalRead(PIN_TOUCH);
  delay(100);
  
  // تست رنگ‌ها
  setLEDs(255, 0, 0, 0); delay(600);
  setLEDs(0, 255, 0, 0); delay(600);
  setLEDs(0, 0, 255, 0); delay(600);
  setLEDs(0, 0, 0, 255); delay(600);
  
  setLEDs(0, 0, 255, 0); 
  digitalWrite(PIN_BUZZER, LOW);
  
  delay(1000);
  calibrateSensors();
  Serial.println("INIT|ATMEGA_READY");
}

void loop() {
  int hall = getHallFiltered();
  int ldr = getLdrFiltered();
  currentTouchState = digitalRead(PIN_TOUCH);
  
  processTouch();
  if(blinking) updateBlink();
  
  // ✅ تایمر غیرمسدود برای نمایش قرمز پس از لمس طولانی
  if (alarmShowing && millis() > alarmLEDEnd) {
    alarmShowing = false;
    // بازیابی آخرین وضعیت
    updateLED(lastStateLED);
  }
  
  Serial.print("DATA|H:"); Serial.print(hall);
  Serial.print(",L:"); Serial.print(ldr);
  Serial.print(",T:"); Serial.print(currentTouchState);
  Serial.print(",C:"); Serial.println(calibrated ? 1 : 0);
  
  processCommands();
  delay(50);
}

// ✅ پر کردن بافر فیلتر قبل از کالیبراسیون
void calibrateSensors() {
  // پر کردن بافر با خوانش‌های واقعی
  for(int i=0; i<5; i++) {
    getHallFiltered();
    getLdrFiltered();
    delay(10);
  }
  
  long hSum = 0, lSum = 0;
  for(int i=0; i<30; i++) {
    hSum += getHallFiltered();
    lSum += getLdrFiltered();
    delay(50);
  }
  hallBase = hSum / 30;
  ldrBase = lSum / 30;
  calibrated = true;
  Serial.print("CALIB:"); 
  Serial.print(hallBase); 
  Serial.print(",");
  Serial.println(ldrBase);
  
  // ✅ بازیابی آخرین وضعیت پس از کالیبراسیون
  updateLED(lastStateLED);
}

void processTouch() {
  if (currentTouchState == HIGH && lastTouchState == LOW) {
    touchStartTime = millis();
    isTouching = true;
    startBlink(0, 0, 0, 255, 150);
  }
  
  if (isTouching && currentTouchState == HIGH) {
    if (millis() - touchStartTime >= TAP_LONG_MS) {
      Serial.println("EVENT:TOUCH_ALARM_3S");
      stopBlink();
      setLEDs(255, 0, 0, 0);
      
      // ✅ تایمر غیرمسدود به جای delay(500)
      alarmShowing = true;
      alarmLEDEnd = millis() + 500;
      
      isTouching = false;
      // حذف delay(500) مسدودکننده
    }
  }
  
  if (currentTouchState == LOW && lastTouchState == HIGH) {
    if (isTouching) {
      unsigned long duration = millis() - touchStartTime;
      
      // ✅ نادیده گرفتن لمس‌های نویزی زیر 50ms
      if (duration < TAP_SHORT_MIN_MS) {
        stopBlink();
        // بازیابی آخرین وضعیت
        updateLED(lastStateLED);
      }
      else if (duration >= TAP_SHORT_MIN_MS && duration < TAP_LONG_MS) {
        Serial.println("EVENT:TOUCH_ARM_SHORT");
        stopBlink();
        setLEDs(0, 255, 0, 0);
      }
      else {
        stopBlink();
        updateLED(lastStateLED);
      }
      isTouching = false;
    }
  }
  lastTouchState = currentTouchState;
}

void processCommands() {
  while(Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); 
    if(cmd.length() == 0) continue;
    
    if(cmd == "CMD:BUZZER_ON") digitalWrite(PIN_BUZZER, HIGH);
    else if(cmd == "CMD:BUZZER_OFF") digitalWrite(PIN_BUZZER, LOW);
    // ✅ اصلاح substring(10) به substring(8)
    else if(cmd.startsWith("CMD:RGB:")) parseLED(cmd.substring(8));
    else if(cmd == "CMD:CALIBRATE" || cmd == "CMD:CAL_NOW") {
      calibrateSensors(); 
    }
    else if(cmd.startsWith("STATE:")) {
      String st = cmd.substring(6);
      st.trim();
      
      // ✅ اعمال TRIGGERED حتی هنگام لمس
      if (st == "TRIGGERED") {
        stopBlink();
        alarmShowing = false;
        updateLED(st);
        lastStateLED = st;
      }
      // ✅ نادیده گرفتن فرمان‌های عادی هنگام لمس
      else if(!blinking && !isTouching && !alarmShowing) {
        updateLED(st);
        lastStateLED = st;
      }
    }
  }
}

void parseLED(String data) {
  int r=0, g=0, b=0, y=0;
  int idx1 = data.indexOf(',');
  int idx2 = data.indexOf(',', idx1+1);
  int idx3 = data.indexOf(',', idx2+1);
  
  if(idx1 > 0 && idx2 > idx1) {
    r = data.substring(0, idx1).toInt();
    g = data.substring(idx1+1, idx2).toInt();
    if(idx3 > idx2) { 
      b = data.substring(idx2+1, idx3).toInt(); 
      y = data.substring(idx3+1).toInt(); 
    }
    else { 
      b = data.substring(idx2+1).toInt(); 
    }
  }
  setLEDs(r, g, b, y);
}

void updateLED(String state) {
  state.trim();
  if(state == "DISARMED")        setLEDs(0, 0, 255, 0);
  else if(state == "ARMED")      setLEDs(0, 255, 0, 0);
  else if(state == "SUSPICIOUS") setLEDs(0, 0, 0, 255);
  else if(state == "TRIGGERED")  setLEDs(255, 0, 0, 0);
}

void startBlink(int r, int g, int b, int y, unsigned long intv) {
  blinking = true; bR = r; bG = g; bB = b; bY = y; 
  bInt = intv; bLast = millis(); bState = true;
  setLEDs(r, g, b, y);
}

void stopBlink() { blinking = false; setLEDs(0, 0, 0, 0); }

void updateBlink() {
  if(!blinking) return;
  if(millis() - bLast >= bInt) { 
    bLast = millis(); bState = !bState; 
    setLEDs(bState ? bR : 0, bState ? bG : 0, bState ? bB : 0, bState ? bY : 0); 
  }
}