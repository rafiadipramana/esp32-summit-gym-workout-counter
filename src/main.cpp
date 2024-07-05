#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

#define PROXIMITY_SENSOR_PIN 33
#define START_BUTTON_PIN 5
#define RESET_BUTTON_PIN 18
#define SAVE_BUTTON_PIN 19

#define WIFI_SSID "SUMMIT"
#define WIFI_PASSWORD "summitgym"

/*
  API_KEY: Your Firebase API Key
  USER_EMAIL: Your Firebase User Email
  USER_PASSWORD: Your Firebase User Password
  DATABASE_URL: Your Firebase Database URL
*/
#define API_KEY "AIzaSyA7YLhvXYZ4a5W6G_Ed8mFiVpIiIYIAzl4"
#define USER_EMAIL "kawankode2024@gmail.com"
#define USER_PASSWORD "admin12345"
#define DATABASE_URL "https://summit-gym-default-rtdb.firebaseio.com/"
#define REST_API_URL "https://gym-api-six.vercel.app/api/histories"

/*
  Prepare login functions
*/
void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);

DefaultNetwork network;
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));

RealtimeDatabase Database;
bool taskComplete = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  lcd.init();
  lcd.backlight();

  Serial.begin(115200);

  pinMode(PROXIMITY_SENSOR_PIN, INPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SAVE_BUTTON_PIN, INPUT_PULLUP);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan...");
  Serial.print("Menghubungkan...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  lcd.setCursor(0, 1);
  lcd.print("Terkoneksi");
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inisialisasi...");
  Serial.println("Initializing app...");
  ssl_client.setInsecure();

  initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

  lcd.setCursor(0, 1);
  lcd.print("Siap digunakan");\
  delay(2000);
  lcd.clear();
}

void loop()
{
  static unsigned long lastLCDUpdate = 0;    // Static variable to keep track of the last LCD update time
  static unsigned long lastUpdate = 0;       // Static variable to keep track of the last update time
  unsigned long currentMillis = millis();    // Get the current time

  static int movementCounter = 0;
  int proximitySensorValue = digitalRead(PROXIMITY_SENSOR_PIN);

  static int lastStartButtonState = HIGH;    // Track the previous state of the Start button
  static int lastResetButtonState = HIGH;    // Track the previous state of the Reset button
  static int lastSaveButtonState = HIGH;     // Track the previous state of the Save button

  int currentStartButtonState = digitalRead(START_BUTTON_PIN);
  int currentResetButtonState = digitalRead(RESET_BUTTON_PIN);
  int currentSaveButtonState = digitalRead(SAVE_BUTTON_PIN);

  static bool isCounting = false;            // Track whether counting is active
  static bool isPaused = false;              // Track whether counting is paused
  static unsigned long highStartTime = 0;    // Track the time when proximity sensor is activated
  static bool waitingForLow = false;         // Track whether we are waiting for the sensor to go LOW
  static unsigned long startTime = 0;        // Track the time when the start button was pressed
  static unsigned long elapsedTime = 0;      // Track the total elapsed time when counting is active

  if (currentMillis - lastLCDUpdate >= 1000)
  {
    lastLCDUpdate = currentMillis;
    unsigned long totalElapsedTime = elapsedTime;
    if (isCounting && !isPaused)
    {
      totalElapsedTime += millis() - startTime;
    }
    unsigned long seconds = (totalElapsedTime / 1000) % 60;
    unsigned long minutes = (totalElapsedTime / 1000) / 60;

    lcd.setCursor(0, 0);
    lcd.print("Gerakan: ");
    lcd.print(movementCounter);
    lcd.print(" kali");
    lcd.setCursor(0, 1);
    lcd.print("Waktu: ");
    lcd.print(minutes);
    lcd.print(":");
    if (seconds < 10) {
      lcd.print("0"); // Add leading zero for single-digit seconds
    }
    lcd.print(seconds);
  }

  if (currentStartButtonState == LOW && lastStartButtonState == HIGH)
  {
    if (!isCounting)
    {
      lcd.setCursor(0, 0);
      Serial.println("Tombol mulai ditekan");
      isCounting = true;                     // Start counting when the button is pressed
      startTime = millis();
      elapsedTime = 0;
      isPaused = false;                      // Ensure counting is not paused when starting
    }
    else
    {
      isPaused = !isPaused;                  // Toggle the paused state when the button is pressed again
      if (isPaused)
      {
        Serial.println("Perhitungan dijeda");
        elapsedTime += millis() - startTime; // Update the elapsed time when paused
      }
      else
      {
        Serial.println("Perhitungan dilanjutkan");
        startTime = millis();                // Reset the start time when resuming
      }
    }
  }

  if (isCounting && !isPaused)
  {
    if (waitingForLow)
    {
      // Wait for the sensor to go LOW before resetting the waitingForLow flag
      if (proximitySensorValue == LOW)
      {
        waitingForLow = false;
        highStartTime = 0;                   // Reset the high start time
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
          if (currentTime - highStartTime >= 2000)
          {
            movementCounter++;
            Serial.println("Gerakan terdeteksi sejumlah " + String(movementCounter));
            highStartTime = 0;                // Reset the start time for the next counting cycle
            waitingForLow = true;             // Now wait for the sensor to go LOW before counting again
          }
        }
      }
      else
      {
        highStartTime = 0;                    // Reset the start time if the sensor is not triggered
      }
    }
  }

  // Lakukan update ke Firebase setiap 2.5 detik (untuk menghindari limit API)
  if (app.ready() && (currentMillis - lastUpdate >= 2500))
  {
    lastUpdate = currentMillis;

    object_t repetitionJson, timeInMillisJson, json;
    JsonWriter writer;

    unsigned long totalElapsedTime = elapsedTime; // Use the accumulated elapsed time
    if (isCounting && !isPaused)
    {
      totalElapsedTime += millis() - startTime;   // Add the current interval if not paused
    }

    writer.create(repetitionJson, "repetition", movementCounter);
    writer.create(timeInMillisJson, "timeInMillis", totalElapsedTime);
    writer.join(json, 2, repetitionJson, timeInMillisJson);

    Database.update(aClient, "/", json, asyncCB, "updateTask");
  }

  if (currentResetButtonState == LOW && lastResetButtonState == HIGH)
  {
    Serial.println("Tombol reset ditekan");
    movementCounter = 0;
    startTime = 0;                             // Reset the start time when reset is pressed
    elapsedTime = 0;                           // Reset the elapsed time when reset is pressed
    isCounting = false;                        // Stop counting
    isPaused = true;                           // Ensure counting is paused when reset
    Serial.println("Penghitungan direset dan timer direset");
  }

  if (currentSaveButtonState == LOW && lastSaveButtonState == HIGH)
  {
    Serial.println("Tombol simpan ditekan");
    HTTPClient http;
    http.begin(REST_API_URL); // Replace YOUR_REST_API_URL with the actual URL
    http.addHeader("Content-Type", "application/json");
    unsigned long totalElapsedTime = elapsedTime;
    if (isCounting && !isPaused)
    {
        totalElapsedTime += millis() - startTime;
    }
    String jsonPayload = "{\"duration\": " + String(totalElapsedTime) + 
                     ", \"repetition\": " + String(movementCounter) + 
                     ", \"userId\": \"mbmwwNSI2lRBVH5usr1qYNrJyR12\"}";
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Kode Status HTTP : " + String(httpResponseCode));
        Serial.println("Respon : " + response);
    } else {
        Serial.println("Galat menyimpan data : " + String(httpResponseCode));
    }

    http.end();
  }

  lastStartButtonState = currentStartButtonState;
  lastResetButtonState = currentResetButtonState;
  lastSaveButtonState = currentSaveButtonState;
}

// ! DO NOT MODIFY THE CODE BELOW
void asyncCB(AsyncResult &aResult)
{
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
