#include <Arduino.h>
#include <pt.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h> // Для использования reset()

#define LED_PIN 13 // Замените на номер пина, к которому подключен ваш светодиод
#define RX_PIN 10  // Пин для RX
#define TX_PIN 9   // Пин для TX

SoftwareSerial softSerial(RX_PIN, TX_PIN); // RX, TX

struct User {
  String username;
  String password;
  bool isAdmin;
};

User users[] = {
  {"User", "1234", false},
  {"Root", "admin", true}
};

User *currentUser = NULL;

bool showMemory = true;

void checkLogin() {
  String inputUsername;
  String inputPassword;
  
  while (true) {
    Serial.println(F("Enter username:"));
    while (Serial.available() == 0) {} // Ждем ввода имени пользователя
    inputUsername = Serial.readStringUntil('\n'); // Читаем введенное имя пользователя
    inputUsername.trim();

    Serial.println(F("Enter password:"));
    while (Serial.available() == 0) {} // Ждем ввода пароля
    inputPassword = Serial.readStringUntil('\n'); // Читаем введенный пароль
    inputPassword.trim();

    for (int i = 0; i < sizeof(users)/sizeof(users[0]); i++) {
      if (inputUsername == users[i].username && inputPassword == users[i].password) {
        currentUser = &users[i];
        Serial.println(F("Login successful"));
        digitalWrite(LED_PIN, HIGH); // Включаем светодиод
        delay(300); // Ждем 300 мс
        digitalWrite(LED_PIN, LOW); // Выключаем светодиод
        return; // Выходим из функции, если авторизация успешна
      }
    }

    Serial.println(F("Incorrect username or password"));
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH); // Включаем светодиод
      delay(100);
      digitalWrite(LED_PIN, LOW); // Выключаем светодиод
      delay(100);
    }
  }
}

extern unsigned int __heap_start;
extern void *__brkval;

int getFreeMemory() {
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__heap_start);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  return freeValue;
}

SoftwareSerial BTSerial(9, 10); // RX, TX

struct pt pt1, pt2;
static int counter1 = 0, counter2 = 0;
static unsigned long lastRun1 = 0, lastRun2 = 0;
static char time[] = "00:00:00";
static char date[] = "01.01.2024";
static unsigned long lastTimeUpdate = 0, lastDateUpdate = 0;
bool taskRunning = false, task1Running = false, task2Running = false;

struct Task {
  const char *name;
  bool isRunning;
  unsigned long startTime;
};

Task tasks[10];
int taskCount = 0;

void startTask(const char *name) {
  if (taskCount < 10) {
    tasks[taskCount].name = name;
    tasks[taskCount].isRunning = true;
    tasks[taskCount].startTime = millis();
    taskCount++;
  }
}

void endTask(const char *name) {
  for (int i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
      tasks[i].isRunning = false;
      break;
    }
  }
}

void ps() {
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].isRunning) {
      Serial.print(tasks[i].name);
      Serial.println(F(": Running"));
    }
  }
}

static int task(struct pt *pt, bool &taskRunning, int &counter, unsigned long &lastRun, int delay) {
  PT_BEGIN(pt);
  while (taskRunning) {
    if (millis() - lastRun >= delay) {
      lastRun = millis();
      Serial.print(F("Task: "));
      Serial.println(counter++);
    }
    PT_WAIT_UNTIL(pt, millis() - lastRun >= delay);
  }
  PT_END(pt);
}

static int task1(struct pt *pt) {
  return task(pt, task1Running, counter1, lastRun1, 1000);
}

static int task2(struct pt *pt) {
  return task(pt, task2Running, counter2, lastRun2, 1500);
}

void printTime() {
  Serial.println(time);
}

void printDate() {
  Serial.println(date);
}

void setTime(String newTime) {
  newTime.toCharArray(time, sizeof(time));
  lastTimeUpdate = millis();
}

void setDate(String newDate) {
  newDate.toCharArray(date, sizeof(date));
  lastDateUpdate = millis();
}

void echo(const char *message) {
  Serial.println(message);
}

void id() {
  Serial.println(currentUser->username);
}

void top() {
  int freeMemory = getFreeMemory();
  int usedMemory = 2048 - freeMemory;
  float memoryUsagePercentage = (float)usedMemory / 2048 * 100;

  Serial.print(F("Free memory: "));
  Serial.println(freeMemory);
  Serial.print(F("Memory usage: "));
  Serial.print(usedMemory);
  Serial.print(F(" bytes ("));
  Serial.print(memoryUsagePercentage);
  Serial.println(F("%)"));

  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].isRunning) {
      unsigned long runTimeMillis = millis() - tasks[i].startTime;
      unsigned long runTimeSecs = runTimeMillis / 1000;
      unsigned long runTimeMins = runTimeSecs / 60;
      runTimeSecs = runTimeSecs % 60;
      Serial.print(tasks[i].name);
      Serial.print(F(": Running for "));
      Serial.print(runTimeMins);
      Serial.print(F(" minutes and "));
      Serial.print(runTimeSecs);
      Serial.println(F(" seconds"));
    }
  }
}

void uname() {
#if defined(ARDUINO_AVR_UNO)
  Serial.print(F("Arduino UNO"));
#elif defined(ARDUINO_AVR_NANO)
  Serial.print(F("Arduino Nano"));
#elif defined(ARDUINO_AVR_MEGA2560)
  Serial.print(F("Arduino Mega 2560"));
#else
  Serial.print(F("Unknown"));
#endif
}

void help() {
  Serial.println(F("List of available commands:"));
  Serial.println(F("1 - Start task 1."));
  Serial.println(F("2 - Start task 2."));
  Serial.println(F("time - Display current time."));
  Serial.println(F("date - Display current date."));
  Serial.println(F("settime [time] - Set time. Example: 'settime 12:34:56'"));
  Serial.println(F("setdate [date] - Set date. Example: 'setdate 31.12.2023'"));
  Serial.println(F("echo [message] - Display message."));
  Serial.println(F("id - Display username."));
  Serial.println(F("top - Display number of tasks executed."));
  Serial.println(F("ps - Display task status."));
  Serial.println(F("calc [operation] [number1] [number2] - Perform a mathematical operation. Operation can be '+', '-', '*' or '/'."));
  Serial.println(F("kill [task number] - Stop a task."));
  Serial.println(F("usermod [text] - Change username."));
  Serial.println(F("hide [mem] - Hide memory status."));
  Serial.println(F("show [mem] - Show memory status."));
  Serial.println(F("help - Display this help."));
  if (currentUser->isAdmin) {
    Serial.println(F("sudo [command] - Execute command with administrator privileges."));
    Serial.println(F("reboot - Reboot the system."));
    Serial.println(F("exit - Lock the terminal."));
  }
}

void calculate(String operation) {
  operation.trim();
  int spaceIndex1 = operation.indexOf(' ');
  int spaceIndex2 = operation.lastIndexOf(' ');
  float operand1 = operation.substring(0, spaceIndex1).toFloat();
  String op = operation.substring(spaceIndex1 + 1, spaceIndex2);
  float operand2 = operation.substring(spaceIndex2 + 1).toFloat();
  
  if (op == "+") {
    Serial.println(operand1 + operand2);
  } else if (op == "-") {
    Serial.println(operand1 - operand2);
  } else if (op == "*") {
    Serial.println(operand1 * operand2);
  } else if (op == "/") {
    if (operand2 != 0) {
      Serial.println(operand1 / operand2);
    } else {
      Serial.println(F("Error: division by zero"));
    }
  } else if (op == "^") {
    Serial.println(pow(operand1, operand2));
  } else if (op == "%") {
    Serial.println(fmod(operand1, operand2));
  } else {
    Serial.println(F("Unknown operation"));
  }
}

void kill(String taskName) {
  String taskNames[taskCount];
  for (int i = 0; i < taskCount; i++) {
    taskNames[i] = tasks[i].name;
    if (taskNames[i] == taskName) {
      tasks[i].isRunning = false;
      Serial.println(taskName + F(" stopped."));
      return;
    }
  }
  Serial.println(String(F("Unknown task: ")) + taskName);
}

void reboot() {
  if (currentUser->isAdmin) {
    wdt_enable(WDTO_15MS); // Включаем сторожевой таймер на 15 мс
    while (true) {} // Ждем перезагрузки
  } else {
    Serial.println(F("Access is denied"));
  }
}

void listenSignals() {
  if (softSerial.available()) {
    char received = softSerial.read();
    Serial.print(F("Received signal: "));
    Serial.println(received);
  }
}

void exit() {
  if (currentUser->isAdmin) {
    Serial.println(F("Blocked."));
    while (true) {} // Блокируем систему
  } else {
    Serial.println(F("Access is denied"));
  }
}

void handleBluetoothCommand(String command) {
  Serial.println(F("Bluetooth command accepted."));
}

void manageLED() {
  int freeMemory = getFreeMemory();

  if (freeMemory >= 900) {
    digitalWrite(LED_PIN, LOW);
  } else if (freeMemory < 900 && freeMemory >= 600) {
    if (millis() % 4000 < 2000) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else if (freeMemory < 600 && freeMemory >= 150) {
    if (millis() % 2000 < 1000) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else if (freeMemory < 150) {
    digitalWrite(LED_PIN, HIGH);
  }
}

void updateTimeAndDate() {
  String timeStr = String(time);
  int hour = timeStr.substring(0, 2).toInt();
  int minute = timeStr.substring(3, 5).toInt();
  int second = timeStr.substring(6, 8).toInt();

  if (millis() - lastTimeUpdate >= 1000) {
    second++;
    lastTimeUpdate = millis();
    if (second >= 60) {
      second = 0;
      minute++;
      if (minute >= 60) {
        minute = 0;
        hour++;
        if (hour >= 24) {
          hour = 0;
          incrementDate();
        }
      }
    }
    snprintf(time, sizeof(time), "%02d:%02d:%02d", hour, minute, second);
  }
}

void incrementDate() {
  String dateStr(date);
  int day = atoi(dateStr.substring(0, 2).c_str());
  int month = atoi(dateStr.substring(3, 5).c_str());
  int year = atoi(dateStr.substring(6, 10).c_str());

  day++;
  lastDateUpdate = millis();
  
  // Упрощенная проверка на количество дней в месяце
  if ((day > 30 && (month == 4 || month == 6 || month == 9 || month == 11)) ||
      (day > 31 && (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)) ||
      (day > 28 && month == 2 && !(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) || // Не високосный год
      (day > 29 && month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))) {  // Високосный год
    day = 1;
    month++;
    if (month > 12) {
      month = 1;
      year++;
    }
  }
  snprintf(date, sizeof(date), "%02d.%02d.%04d", day, month, year);
}

void displayInterface() {
  int freeMemory = getFreeMemory();
  int usedMemory = 2048 - freeMemory;
  float memoryUsagePercentage = (float)usedMemory / 2048 * 100;

  if (showMemory) {
    Serial.print(F("Ram: "));
    Serial.print(usedMemory);
    Serial.print(F("b "));
    Serial.print(memoryUsagePercentage);
    Serial.println(F("%"));
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  checkLogin();
}

void loop() {
  String command;

  if (getFreeMemory() < 30) {
    for (int i = 0; i < taskCount; i++) {
      if (tasks[i].isRunning) {
        kill(tasks[i].name);
        break;
      }
    }
  }

  if (getFreeMemory() <= 6) {
    reboot(); // Перезапускаем контроллер при критической нехватке памяти
  }

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    displayInterface();
    Serial.print(currentUser->username);
    Serial.print(F("@"));
    uname();
    Serial.print(F(":~$ "));
    Serial.println(command);


    if (command.startsWith(F("1"))) {
      bool task1Running = false;
      startTask("Task 1");
      task1(&pt1);
    } else if (command.startsWith(F("2"))) {
      bool task2Running = true;
      startTask("Task 2");
      task2(&pt2);
    } else if (command.startsWith(F("time"))) {
      printTime();
    } else if (command.startsWith(F("date"))) {
      printDate();
    } else if (command.startsWith(F("settime "))) {
      setTime(command.substring(8));
      Serial.print(F("Current time: "));
    } else if (command.startsWith(F("setdate "))) {
      setDate(command.substring(8));
      Serial.print(F("Current date: "));
    } else if (command.startsWith(F("echo "))) {
      echo(command.substring(5).c_str());
    } else if (command.startsWith(F("id"))) {
      id();
    } else if (command.startsWith(F("top"))) {
      top();
    } else if (command.startsWith(F("ps"))) {
      ps();
    } else if (command.startsWith(F("calc "))) {
      calculate(command.substring(5));
    } else if (command.startsWith(F("kill "))) {
      kill(command.substring(5));
    } else if (command.startsWith(F("usermod "))) {
      if (currentUser->isAdmin) {
        currentUser->username = command.substring(8);
        Serial.println("Name changed to " + currentUser->username);
      } else {
        Serial.println(F("Access is denied"));
      }
    } else if (command.startsWith(F("sudo "))) {
      String pinCommand = command.substring(5);
      int pinNumber = pinCommand.substring(3, pinCommand.indexOf(' ')).toInt();
      String pinValue = pinCommand.substring(pinCommand.indexOf(' ') + 1);
      
      if (pinCommand.startsWith("pin")) {
          if (pinValue == "true") {
            pinMode(pinNumber, OUTPUT);
            digitalWrite(pinNumber, HIGH);
            Serial.println("Pin " + String(pinNumber) + " set to HIGH");
          } else if (pinValue == "false") {
            pinMode(pinNumber, OUTPUT);
            digitalWrite(pinNumber, LOW);
            Serial.println("Pin " + String(pinNumber) + " set to LOW");
          } else {
            int brightness = pinValue.toInt();
            if (brightness >= 0 && brightness <= 255) {
              pinMode(pinNumber, OUTPUT);
              analogWrite(pinNumber, brightness);
              Serial.println("Pin " + String(pinNumber) + " brightness set to " + String(brightness));
            }
          }
        } else if (pinCommand.startsWith("pinTX")) {
          // Отправляем данные через TX пин
          Serial.begin(9600); // Используйте Serial вместо Serial1
          Serial.print(pinValue);
          Serial.println("Sent " + pinValue + " to TX pin");
        } else if (pinCommand.startsWith("pinRX")) {
          // Читаем данные из RX пина
          Serial.begin(9600); // Используйте Serial вместо Serial1
          while (Serial.available() > 0) {
            char received = Serial.read();
            Serial.print("Received from RX pin: ");
            Serial.println(received, HEX);
          }
        } else if (pinCommand.startsWith("pinA")) {
          // Читаем аналоговое значение с пина
          int value = analogRead(pinNumber);
          Serial.print("Analog value at pin A" + String(pinNumber) + ": ");
          Serial.println(value);
        } else if (pinCommand.startsWith("vm")) {
          int analogPin = pinCommand.substring(8).toInt(); // Пин, к которому подключен делитель напряжения
          float referenceVoltage = 12.0; // Опорное напряжение Arduino
          int analogValue = analogRead(analogPin);
          float voltage = (analogValue / 1023.0) * referenceVoltage;

          // Определение единиц измерения напряжения
          if (voltage < 1.0) {
              Serial.print(F("Battery voltage: "));
              Serial.print(voltage * 1000);
              Serial.println(F("mV"));
          } else {
              Serial.print(F("Battery voltage: "));
              Serial.print(voltage);
              Serial.println(F("V"));
          }

          // Расчет мощности
          float current = 1.0; // Предполагаемый ток в амперах
          float power = voltage * current;
          Serial.print(F("Battery power: "));
          Serial.print(power);
          Serial.println(F("W"));

          // Определение единиц измерения тока
          if (current < 1.0) {
              Serial.print(F("Battery current: "));
              Serial.print(current * 1000);
              Serial.println(F("mA"));
          } else {
              Serial.print(F("Battery current: "));
              Serial.print(current);
              Serial.println(F("A"));
          }
        
        } else if (pinCommand.startsWith("sniffers")) {
          listenSignals();
        } else if (pinCommand.startsWith("xray")) {
          // Пины, к которым подключен датчик HC-SR04
          int trigPin = 9; // Trigger
          int echoPin = 10; // Echo
          // Настройка пинов датчика HC-SR04
          pinMode(trigPin, OUTPUT);
          pinMode(echoPin, INPUT);

          // Генерация ультразвукового сигнала
          digitalWrite(trigPin, LOW);
          delayMicroseconds(2);
          digitalWrite(trigPin, HIGH);
          delayMicroseconds(10);
          digitalWrite(trigPin, LOW);

          // Чтение эхо-сигнала
          long duration = pulseIn(echoPin, HIGH);

          // Расчет расстояния
          float distance = duration * 0.034 / 2;

          // Вывод расстояния
          Serial.print(F("Distance: "));
          Serial.print(distance);
          Serial.println(F(" cm"));
        }
    } else if (command.startsWith(F("bluetooth "))) {
      handleBluetoothCommand(command.substring(10));
    } else if (command.startsWith(F("uname"))) {
      uname();
    } else if (command.startsWith(F("reboot"))) {
      reboot();
        } else if (command.startsWith(F("hide "))) {
      if (command.substring(5) == F("mem")) {
        showMemory = false;
      }
    } else if (command.startsWith(F("show "))) {
      if (command.substring(5) == F("mem")) {
        showMemory = true;
      }
    } else if (command.startsWith(F("exit"))) {
      exit();
    } else if (command.startsWith(F("help"))) {
      help();
    } else {
      Serial.println(F("Unknown team."));
    }
  }

  manageLED();
  updateTimeAndDate();
}
