// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiClientSecure.h>
// #include <FirebaseClient.h>
// #include <Wire.h>

// #define PROXIMITY_SENSOR_PIN 33
// #define WIFI_SSID "localhost"
// #define WIFI_PASSWORD "akurapaham"

// #define API_KEY "AIzaSyA7YLhvXYZ4a5W6G_Ed8mFiVpIiIYIAzl4"
// #define USER_EMAIL "kawankode2024@gmail.com"
// #define USER_PASSWORD "admin12345"
// #define DATABASE_URL "https://summit-gym-default-rtdb.firebaseio.com/"

// void authHandler();
// void printError(int code, const String &message);
// void printResult(AsyncResult &asyncResult);

// DefaultNetwork defaultNetwork;
// UserAuth userAuth(API_KEY, USER_EMAIL, USER_PASSWORD);
// FirebaseApp firebaseApp;
// WiFiClientSecure wiFiClientSecure;

// using AsyncClient = AsyncClientClass;
// AsyncClient asyncClient(wiFiClientSecure, getNetwork(defaultNetwork));
// RealtimeDatabase Database;
// AsyncResult asyncResultNoCallback;

// void setup()
// {
//   Serial.begin(115200);
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.println("Menyambungkan ke WiFi...");
//   }
//   Serial.println("Terhubung ke WiFi");
//   Serial.println("Alamat IP: ");
//   Serial.println(WiFi.localIP());

//   Firebase.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
//   Serial.println("Menghubungkan ke Firebase RTDB...");

//   wiFiClientSecure.setInsecure();

//   initializeApp(asyncClient, firebaseApp, getAuth(userAuth), asyncResultNoCallback);

//   authHandler();
//   firebaseApp.getApp<RealtimeDatabase>(Database);

//   Database.url(DATABASE_URL);
//   pinMode(PROXIMITY_SENSOR_PIN, INPUT);
// }

// void loop()
// {
//   authHandler();
//   Database.loop();

//   int proximitySensorValue = digitalRead(PROXIMITY_SENSOR_PIN);
//   if (proximitySensorValue == HIGH)
//   {
//     Serial.println("Sensor mendeteksi objek");
//     bool status = Database.update<object_t>(asyncClient, "/sensor", object_t("{\"status\":true}"));
//     if (asyncClient.lastError().code() == 0)
//       Firebase.printf("Mengirim data sensor ke database: %s\n", status);
//     else
//       printError(asyncClient.lastError().code(), asyncClient.lastError().message());
//   }
//   else
//   {
//     Serial.println("Sensor tidak mendeteksi objek");
//     bool status = Database.update<object_t>(asyncClient, "/sensor", object_t("{\"status\":false}"));
//     if (asyncClient.lastError().code() == 0)
//       Firebase.printf("Mengirim data sensor ke database: %s\n", status);
//     else
//       printError(asyncClient.lastError().code(), asyncClient.lastError().message());
//   }
//   delay(1000);
// }

// void authHandler()
// {
//     unsigned long ms = millis();
//     while (firebaseApp.isInitialized() && !firebaseApp.ready() && millis() - ms < 120 * 1000)
//     {
//         JWT.loop(firebaseApp.getAuth());
//         printResult(asyncResultNoCallback);
//     }
// }

// void printResult(AsyncResult &aResult)
// {
//     if (aResult.isEvent())
//     {
//         Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
//     }

//     if (aResult.isDebug())
//     {
//         Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
//     }

//     if (aResult.isError())
//     {
//         Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
//     }
// }

// void printError(int code, const String &message)
// {
//     Firebase.printf("Error, msg: %s, code: %d\n", message.c_str(), code);
// }

/**
 * SYNTAX:
 *
 * RealtimeDatabase::update(<AsyncClient>, <path>, <object_t>, <AsyncResult>);
 * RealtimeDatabase::update(<AsyncClient>, <path>, <object_t>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <path> - The node path to update (patch) the value.
 * <object_t> - The JSON representation data (object_t) to update.
 * <AsyncResult> - The async result (AsyncResult).
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 *
 * The complete usage guidelines, please visit https://github.com/mobizt/FirebaseClient
 */

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W) || defined(ARDUINO_GIGA)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>) || defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#elif __has_include(<WiFiC3.h>) || defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif __has_include(<WiFi.h>)
#include <WiFi.h>
#endif

#include <FirebaseClient.h>
#include <Wire.h>

#define PROXIMITY_SENSOR_PIN 33
#define START_BUTTON_PIN 14
#define PAUSE_BUTTON_PIN 27
#define STOP_BUTTON_PIN 26
#define RESET_BUTTON_PIN 25
#define SAVE_BUTTON_PIN 32

#define WIFI_SSID "localhost"
#define WIFI_PASSWORD "akurapaham"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyA7YLhvXYZ4a5W6G_Ed8mFiVpIiIYIAzl4"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "kawankode2024@gmail.com"
#define USER_PASSWORD "admin12345"
#define DATABASE_URL "https://summit-gym-default-rtdb.firebaseio.com/"

void asyncCB(AsyncResult &aResult);

void printResult(AsyncResult &aResult);

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_GIGA) || defined(ARDUINO_PORTENTA_C33) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiSSLClient.h>
WiFiSSLClient ssl_client;
#endif

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

RealtimeDatabase Database;

bool taskComplete = false;

void setup()
{
  Serial.begin(115200);
  pinMode(PROXIMITY_SENSOR_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SAVE_BUTTON_PIN, INPUT_PULLUP);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    Serial.println("Initializing app...");

  #if defined(ESP32) || defined(ESP8266) || defined(PICO_RP2040)
    ssl_client.setInsecure();
  #if defined(ESP8266)
    ssl_client.setBufferSizes(4096, 1024);
  #endif
  #endif

    initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
}

void loop() {
  // The async task handler should run inside the main loop
  // without blocking delay or bypassing with millis code blocks.

  // app.loop();      // Run the app's loop handler
  // Database.loop(); // Run the database's loop handler

  static unsigned long lastUpdate = 0;    // Static variable to keep track of the last update time
  unsigned long currentMillis = millis(); // Get the current time

  static int movementCounter = 0;
  int proximitySensorValue = digitalRead(PROXIMITY_SENSOR_PIN);

  static int lastStartButtonState = HIGH;
  static int lastStopButtonState = HIGH;
  static int lastResetButtonState = HIGH;
  static int lastSaveButtonState = HIGH;
  static int lastPauseButtonState = HIGH; // Track the previous state of the Pause button

  int currentStartButtonState = digitalRead(START_BUTTON_PIN);
  int currentStopButtonState = digitalRead(STOP_BUTTON_PIN);
  int currentResetButtonState = digitalRead(RESET_BUTTON_PIN);
  int currentSaveButtonState = digitalRead(SAVE_BUTTON_PIN);
  int currentPauseButtonState = digitalRead(PAUSE_BUTTON_PIN); // Read the current state of the Pause button

  static bool isCounting = false;         // Track whether counting is active
  static bool isPaused = false;           // Track whether counting is paused
  static unsigned long highStartTime = 0; // Track the time when proximity sensor is activated
  static bool waitingForLow = false;      // Track whether we are waiting for the sensor to go LOW
  static unsigned long countTime = 0;     // Track the last time the start button was pressed

  if (currentStartButtonState == LOW && lastStartButtonState == HIGH)
  {
    Serial.println("Start button clicked");
    isCounting = true; // Start counting when the button is pressed
    isPaused = false;  // Ensure counting is not paused when starting
  }

  if (isCounting && !isPaused)
  {
    countTime = 0; // Reset the count time if counting is active
    countTime = millis() - countTime; // Calculate the time since the start button was pressed
    if (waitingForLow)
    {
      // Wait for the sensor to go LOW before resetting the waitingForLow flag
      if (proximitySensorValue == LOW)
      {
        waitingForLow = false;
        highStartTime = 0; // Reset the high start time
      }
    }
    else
    {
      if (proximitySensorValue == HIGH)
      {
        if (highStartTime == 0)
        {
          highStartTime = millis();
        }
        else
        {
          unsigned long currentTime = millis();
          if (currentTime - highStartTime >= 3000)
          {
            movementCounter++;
            Serial.println("Counter increased");
            Serial.println(movementCounter);
            highStartTime = 0;    // Reset the start time for the next counting cycle
            waitingForLow = true; // Now wait for the sensor to go LOW before counting again
          }
        }
      }
      else
      {
        highStartTime = 0; // Reset the start time if the sensor is not triggered
      }
    }
  }

  // Update Firebase every 2500 ms if app is ready
  if (app.ready() && (currentMillis - lastUpdate >= 2500)) {
    lastUpdate = currentMillis;

    object_t repetitionJson, timeInMillisJson, json;
    JsonWriter writer;

    writer.create(repetitionJson, "repetition", movementCounter);
    writer.create(timeInMillisJson, "timeInMillis", countTime);
    writer.join(json, 2, repetitionJson, timeInMillisJson);

    Database.update(aClient, "/", json, asyncCB, "updateTask");
  }

  if (currentStopButtonState == LOW && lastStopButtonState == HIGH)
  {
    Serial.println("Stop button clicked");
    isCounting = false; // Stop counting when the stop button is pressed
  }

  if (currentResetButtonState == LOW && lastResetButtonState == HIGH)
  {
    Serial.println("Reset button clicked");
    movementCounter = 0; // Reset the counter
    Serial.println(movementCounter);
  }

  if (currentSaveButtonState == LOW && lastSaveButtonState == HIGH)
  {
    Serial.println("Save button clicked");
    // Implement your save logic here
  }

  if (currentPauseButtonState == LOW && lastPauseButtonState == HIGH)
  {
    Serial.println("Pause button clicked");
    isPaused = !isPaused; // Toggle the paused state
    if (isPaused)
    {
      Serial.println("Counting paused");
    }
    else
    {
      Serial.println("Counting resumed");
    }
  }

  lastStartButtonState = currentStartButtonState;
  lastStopButtonState = currentStopButtonState;
  lastResetButtonState = currentResetButtonState;
  lastSaveButtonState = currentSaveButtonState;
  lastPauseButtonState = currentPauseButtonState;
}


void asyncCB(AsyncResult &aResult)
{
  // WARNING!
  // Do not put your codes inside the callback and printResult.

  printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
  if (aResult.isEvent())
  {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug())
  {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError())
  {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available())
  {
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}