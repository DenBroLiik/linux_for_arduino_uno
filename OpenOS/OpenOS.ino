#include <Arduino.h>
#include <pt.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <math.h>


String password = "1234"; // Замените на ваш пароль
#define LED_PIN 13 // Замените на номер пина, к которому подключен ваш светодиод

void checkPassword() {
  String inputPassword;
  while (true) { // Бесконечный цикл
    Serial.println(F("Введите пароль:"));
    while (Serial.available() == 0) {} // Ждем ввода пароля
    inputPassword = Serial.readString(); // Читаем введенный пароль
    if (inputPassword == password) {
      Serial.println(F("Пароль верный"));
      digitalWrite(LED_PIN, HIGH); // Включаем светодиод
      delay(300); // Ждем секунду
      digitalWrite(LED_PIN, LOW); // Выключаем светодиод
      break; // Выходим из цикла, если пароль верный
    } else {
      Serial.println(F("Пароль неверный"));
      for (int i = 0; i < 2; i++) { // Мигаем светодиодом 2 раза
        digitalWrite(LED_PIN, HIGH); // Включаем светодиод
        delay(100); // Ждем полсекунды
        digitalWrite(LED_PIN, LOW); // Выключаем светодиод
        delay(100); // Ждем полсекунды
      }
      // Если пароль неверный, цикл продолжается и пароль запрашивается снова
    }
  }
}

extern unsigned int __heap_start;
extern void *__brkval;

// Функция для получения свободной памяти
int getFreeMemory() {
  int freeValue;
  if((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__heap_start);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  return freeValue;
}

SoftwareSerial BTSerial(9, 10); // RX, TX

struct pt pt, pt1, pt2;
static int counter1 = 0, counter2 = 0;
static unsigned long lastRun1 = 0, lastRun2 = 0;
static String time = "00:00:00", date = "01.01.2024";
static unsigned long lastTimeUpdate = 0, lastDateUpdate = 0;
String username = "User"; // Здесь вы можете установить имя пользователя
bool taskRunning = false, task1Running = false, task2Running = false;

struct Task {
  String name;
  bool isRunning;
  unsigned long startTime;
  // Другие поля, которые вы хотите отслеживать...
};

Task tasks[10]; // Массив для хранения информации о задачах
int taskCount = 0; // Количество активных задач

void startTask(String name) {
  if (taskCount < 10) {
    tasks[taskCount].name = name;
    tasks[taskCount].isRunning = true;
    tasks[taskCount].startTime = millis();
    taskCount++;
  }
}

void endTask(String name) {
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].name == name) {
      tasks[i].isRunning = false;
      break;
    }
  }
}

void ps() {
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].isRunning) {
      Serial.println(tasks[i].name + ": Running");
    }
  }
}

static int task(struct pt *pt, bool &taskRunning, int &counter, unsigned long &lastRun, int delay) {

}


static int task1(struct pt *pt) {
  return task(pt, task1Running, counter1, lastRun1, 1000);
}

static int task2(struct pt *pt) {
  return task(pt, task2Running, counter2, lastRun2, 1500);
}

void printTime() {
  Serial.print(F("Current time: "));
  Serial.println(time);
}

void printDate() {
  Serial.print(F("Current date: "));
  Serial.println(date);
}

void setTime(String newTime) {
  time = newTime;
  lastTimeUpdate = millis();
}

void setDate(String newDate) {
  date = newDate;
  lastDateUpdate = millis();
}

void echo(String message) {
  Serial.println(message);
}

void id() {
  Serial.println(username);
}

void top() {
  int freeMemory = getFreeMemory();
  int usedMemory = 2048 - freeMemory;
  float memoryUsagePercentage = (float)usedMemory / 2048 * 100;

  Serial.print(F("Свободная память: "));
  Serial.println(freeMemory);
  Serial.print(F("Использование памяти: "));
  Serial.print(usedMemory);
  Serial.print(F(" байт ("));
  Serial.print(memoryUsagePercentage);
  Serial.println(F("%)"));
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].isRunning) {
      unsigned long runTimeMillis = millis() - tasks[i].startTime;
      unsigned long runTimeSecs = runTimeMillis / 1000;
      unsigned long runTimeMins = runTimeSecs / 60;
      runTimeSecs = runTimeSecs % 60;
      Serial.println(tasks[i].name + ": Running for " + String(runTimeMins) + " minutes and " + String(runTimeSecs) + " seconds");
    }
  }
}

void uname() {
  #if defined(ARDUINO_AVR_UNO)
    Serial.println(F("Arduino UNO"));
  #elif defined(ARDUINO_AVR_NANO)
    Serial.println(F("Arduino Nano"));
  #elif defined(ARDUINO_AVR_MEGA2560)
    Serial.println(F("Arduino Mega 2560"));
  #else
    Serial.println(F("Unknown"));
  #endif

}

void help() {
  Serial.println(F("Список доступных команд:"));
  Serial.println(F("1 - Запустить задачу 1."));
  Serial.println(F("2 - Запустить задачу 2."));
  Serial.println(F("time - Вывести текущее время."));
  Serial.println(F("date - Вывести текущую дату."));
  Serial.println(F("settime [время] - Установить время. Пример: 'settime 12:34:56'"));
  Serial.println(F("setdate [дата] - Установить дату. Пример: 'setdate 31.12.2023'"));
  Serial.println(F("echo [сообщение] - Вывести сообщение."));
  Serial.println(F("id - Вывести имя пользователя."));
  Serial.println(F("top - Вывести количество выполненных задач."));
  Serial.println(F("ps - Вывести статус задач."));
  Serial.println(F("calc [операция] [число1] [число2] - Выполнить математическую операцию. Операция может быть '+', '-', '*' '/'."));
  Serial.println(F("kill [номер задачи] - Остановить задачу."));
  Serial.println(F("usermod [text] - Изменить название пользователя."));
  Serial.println(F("sudo [pin (pin1/pin13) or (pinA0/pinA5) or (pinTX/pinRX)] [value (true/false)] - Упровлять пинами."));
  Serial.println(F("help - Вывести эту справку."));
  Serial.println(F("ДОКУМЕНТАЦИЯ"));
  Serial.println(F("Bluetooth модуль (HC-05 или HC-06):"));
  Serial.println(F("VCC подключается к 5V"));
  Serial.println(F("GND подключается к GND"));
  Serial.println(F("TXD подключается к RX"));
  Serial.println(F("RXD подключается к TX"));
  Serial.println(F("Ультразвуковой датчик расстояния HC-SR04"));
  Serial.println(F("VCC подключается к 5V"));
  Serial.println(F("GND подключается к GND"));
  Serial.println(F("Trig подключается к цифровому пину 9"));
  Serial.println(F("Echo подключается к другому цифровому пину 10"));
  Serial.println(F("Bluetooth HC-05"));
  Serial.println(F("VCC к 5v"));
  Serial.println(F("GND к GND"));
  Serial.println(F("TXD к 10"));
  Serial.println(F("RXD к 11"));
  
}

void calculate(String operation) {
  operation.trim(); // Удаляем пробелы по краям строки
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
      Serial.println(F("Ошибка: деление на ноль"));
    }
  } else if (op == "^") {
    Serial.println(pow(operand1, operand2));
  } else if (op == "%") {
    Serial.println(fmod(operand1, operand2));
  } else {
    Serial.println(F("Неизвестная операция"));
  }
}


void kill(String taskName) {
  // Initialize an empty array to store task names
  String taskNames[taskCount];

  // Copy task names from the tasks array
  for (int i = 0; i < taskCount; i++) {
    taskNames[i] = tasks[i].name;
  }

  // Search for the matching task name
  int index = -1;
  for (int i = 0; i < taskCount; i++) {
    if (taskNames[i] == taskName) {
      index = i;
      break;
    }
  }

  // If the task is found, remove it and stop its execution
  if (index != -1) {
    tasks[index].isRunning = false;
    taskCount--;
    Serial.println(taskName + " остановлена");
  } else {
    Serial.println(String(F("Неизвестная задача: ")) + taskName);
  }
}


void reboot() {
  delay(100); // Задержка перед перезагрузкой
  asm volatile ("jmp 0");
  delay(100); // Задержка после перезагрузки
}

void exit(){
  checkPassword(); // Проверяем пароль
}

void setup() {
  Serial.begin(9600);
  PT_INIT(&pt1);
  PT_INIT(&pt2);
  pinMode(LED_PIN, OUTPUT);
  BTSerial.begin(38400); // Инициализируем Bluetooth
  Serial.println(F("Started"));
  checkPassword(); // Проверяем пароль
}

void loop() {
  String command;
  if (getFreeMemory() < 30) {
    // Если свободная память меньше 30 байт, завершаем самую старую активную задачу
    for (int i = 0; i < taskCount; i++) {
      if (tasks[i].isRunning) {
        kill(tasks[i].name);
        break;
      }
    }
  }
  if (getFreeMemory() <= 6) {
    asm volatile ("jmp 0"); // Перезапускаем контроллер
  }
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    Serial.print(username); Serial.print(F("@")); uname(); Serial.println(F(":~$ "));
    Serial.println(command);
    if (command.startsWith("1")) {
      task1Running = true;
      startTask("Task 1");
      task1(&pt1);
    } else if (command.startsWith("2")) {
      task2Running = true;
      startTask("Task 2");
      task2(&pt2);
    } else if (command.startsWith("time")) {
      printTime();
    } else if (command.startsWith("date")) {
      printDate();
    } else if (command.startsWith("settime ")) {
      setTime(command.substring(8));
    } else if (command.startsWith("setdate ")) {
      setDate(command.substring(8));
    } else if (command.startsWith("echo ")) {
      echo(command.substring(5));
    } else if (command.startsWith("id")) {
      id();
    } else if (command.startsWith("top")) {
      top();
    } else if (command.startsWith("ps")) {
      ps();
    } else if (command.startsWith("calc ")) {
      calculate(command.substring(5));
    } else if (command.startsWith("kill ")) {
      kill(command.substring(5));
    } else if (command.startsWith("usermod ")) {
      username = command.substring(8);
      Serial.println("Имя изменено на " + username);
    } else if (command.startsWith("sudo ")) {
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
        float referenceVoltage = 5.0; // Опорное напряжение Arduino
        int analogValue = analogRead(analogPin);
        float voltage = (analogValue / 1023.0) * referenceVoltage;
        Serial.print(F("Напряжение на аккумуляторе: "));
        Serial.print(voltage);
        Serial.println(F("V"));
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
        Serial.print(F("Расстояние: "));
        Serial.print(distance);
        Serial.println(F(" cm"));
      }
    } else if (command.startsWith("bluetooth ")) {
        String bluetoothCommand = command.substring(10);
        // BTSerial.write(Serial.read());
        // Serial.write(BTSerial.read());

        if (BTSerial.available())
          Serial.write(BTSerial.read());
        if (Serial.available())
          BTSerial.write(Serial.read()); // Добавлено: отправка данных из Serial в BTSerial
        
        // if (bluetoothCommand.startsWith("info")) {
        //   BTSerial.println("AT+NAME?");  // Запросить имя Bluetooth устройства
        //   BTSerial.println("AT+ADDR?");  // Запросить адрес Bluetooth устройства
        //   BTSerial.println("AT+VERSION?"); // Запросить версию прошивки Bluetooth устрой
        // } else if (bluetoothCommand.startsWith("scan")) {
        //   BTSerial.println("AT+INQ");  // Запустить процесс сканирования Bluetooth устройств
        // } else if (bluetoothCommand.startsWith("add ")) {
        //   String deviceAddress = bluetoothCommand.substring(4);
        //   BTSerial.println("AT+LINK=" + deviceAddress);  // Подключиться к Bluetooth устройству по адресу
        // } else if (bluetoothCommand.startsWith("remove ")) {
        //   String deviceAddress = bluetoothCommand.substring(7);
        //   BTSerial.println("AT+REMOVE=" + deviceAddress);  // Удалить Bluetooth устройство по адресу
        // }
    } else if (command.startsWith("uname")) {
      Serial.print(F("Используется плата "));
      uname();
    } else if (command.startsWith("reboot")) {
      reboot();
    } else if (command.startsWith("exit")) {
      Serial.println(F("Заблокировано."));
      exit();
    } else if (command.startsWith("help")) {
      help();
    } else {
      Serial.println(F("Неизвестная команда."));
    }
  }
  
  int freeMemory = getFreeMemory(); // Получаем количество свободной памяти
  // Управляем светодиодом в зависимости от количества свободной памяти
  if (freeMemory >= 900) {
    digitalWrite(LED_PIN, LOW); // Выключаем светодиод
  } else if (freeMemory < 900 && freeMemory >= 600) {
    if (millis() % 4000 < 2000) { // Мигаем каждые 4 секунды
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else if (freeMemory < 600 && freeMemory >= 150) {
    if (millis() % 2000 < 1000) { // Мигаем каждые 2 секунды
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else if (freeMemory < 150) {
    digitalWrite(LED_PIN, HIGH); // Включаем светодиод
  }

  // Увеличиваем время и дату
  int hour = time.substring(0, 2).toInt();
  int minute = time.substring(3, 5).toInt();
  int second = time.substring(6, 8).toInt();
  if (millis() - lastTimeUpdate >= 1000) { // Каждую секунду
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
          // Увеличиваем дату
          int day = date.substring(0, 2).toInt();
          int month = date.substring(3, 5).toInt();
          int year = date.substring(6, 10).toInt();
          if (millis() - lastDateUpdate >= (1000 * 60 * 60 * 24)) { // Каждый день
            day++;
            lastDateUpdate = millis();
            // Здесь нужно добавить проверку на количество дней в месяце и високосный год
            if (day > 31) {
              day = 1;
              month++;
              if (month > 12) {
                month = 1;
                year++;
              }
            }
            date = (day < 10 ? "0" : "") + String(day) + "." + (month < 10 ? "0" : "") + String(month) + "." + String(year);
          }
        }
      }
    }
    time = (hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + ":" + (second < 10 ? "0" : "") + String(second);
  }
}
