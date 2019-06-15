// This sketch receives NMEA strings from an MQTT publisher
// and uses them to display on a screen. Written for M5Stack
// by Andy Barrow
// Now added to github

#include <WiFi.h>
#include <MQTT.h>
#include <TinyGPS++.h>
#include <M5ez.h>
#include <M5Stack.h>
#include <ezTime.h>
#include "Ubuntu_Regular16pt7b.h"
#include "UbuntuMono_Regular16pt7b.h"

#define DEG2RAD 0.0174532925

// Set up WiFi and MQTT information
const char* ssid = "ATTvCsR48s";
const char* password = "iz#iqr26+b%i";
const char* mqtt_server = "192.168.1.145";
//const char* mqttUser = "pi";
//const char* mqttPassword = "gren67ada";

String DBT = "";
String northSouth = "N ";
String eastWest = "E ";
float latRaw;
float lonRaw;
String latiTude = "";
String longiTude = "";
int circleCenterX;
int circleCenterY;


WiFiClient net;
MQTTClient client;

// The TinyGPS++ object
TinyGPSPlus gps;

// Create some custom strings
TinyGPSCustom windSpeed(gps, "IIMWV", 3);
TinyGPSCustom windReference(gps, "IIMWV", 2);
TinyGPSCustom windAngle (gps, "IIMWV", 1);

unsigned long lastMillis = 0;

// This function connects MQTT
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("inTopic");
}

// Here we are connecting MQTT, and sending NMEA data to the GPS object
void messageReceived(MQTTClient *client, char topic[], char payload[], int payload_length) {
  Serial.print("incoming: ");
  for (int i = 0; i < payload_length; i++) {
    Serial.print(payload[i]);
    gps.encode(payload[i]);
  }

  Serial.println();
}

// Here we connect WiFi, rebooting if we don't get anything for 30 seconds
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int dot_cur = 0;
  int reset_index = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //If WiFi doesn't connect in 30 seconds, do a software reset
    reset_index ++;
    if (reset_index > 60) {
      Serial.println("WIFI Failed - restarting");
      ESP.restart();
    }
  }
}

// This routine builds a LAT and LONG string
String DegreesToDegMin(float x) {
  // The abs function doesn't work on floats, so we do it manually
  if (x < 0) {
    x = x * -1.0;
  }
  int degRaw = x;
  float degFloat = float(degRaw);
  float minutesRemainder = (x - degFloat) * 60.0;
  String degMin = "";
  if (degRaw < 100) degMin = "0";
  if (degRaw < 10) degMin = degMin + "0";
  degMin = degMin + degRaw;
  degMin = degMin + " "; //leave some space for the degree character which comes later
  if (minutesRemainder < 10) degMin = degMin + "0";
  degMin = degMin + String(minutesRemainder, 4);
  degMin = degMin + "\'";

  return (degMin);
}

// This routine sends position, speed, time, and date to the display
void displayInfo()
{
  ez.canvas.font(&UbuntuMono_Regular16pt7b);
  if (gps.location.isValid())
  {
    // Latitude
    float latRaw = gps.location.lat();
    if (latRaw < 0) {
      northSouth = "S ";
    }
    else
    {
      northSouth = "N ";
    }
    latiTude = northSouth;
    latiTude += DegreesToDegMin(latRaw);

    //ez.canvas.clear();
    ez.canvas.lmargin(10);
    ez.canvas.y(ez.canvas.top() + 10);
    ez.canvas.x(ez.canvas.lmargin());
    M5.lcd.fillRect(ez.canvas.lmargin(), ez.canvas.top() + 10, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("LAT: ");
    ez.canvas.println(latiTude);
    // Kludge to get a degree symbol
    M5.Lcd.drawEllipse(ez.canvas.lmargin() + 164, ez.canvas.top() + 13, 3, 3, ez.theme->foreground);

    //Longitude
    float lonRaw = gps.location.lng();
    if (lonRaw < 0) {
      eastWest = "W ";
    }
    else
    {
      eastWest = "E ";
    }
    longiTude = eastWest;
    longiTude += DegreesToDegMin(lonRaw);
    ez.canvas.y(ez.canvas.top() + 40);
    ez.canvas.x(ez.canvas.lmargin());
    M5.lcd.fillRect(10, ez.canvas.top() + 40, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("LON: ");
    ez.canvas.print(longiTude);
    // Kludge to get a degree symbol
    M5.Lcd.drawEllipse(ez.canvas.lmargin() + 164, ez.canvas.top() + 43, 3, 3, ez.theme->foreground);

  }
  else
  {
    Serial.print(F("INVALID"));
  }

  // Speed over ground
  if (gps.speed.isValid()) {
    ez.canvas.y(ez.canvas.top() + 70);
    ez.canvas.x(ez.canvas.lmargin());
    M5.lcd.fillRect(10, ez.canvas.top() + 70, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("SOG:");
    if (gps.speed.knots() < 10) ez.canvas.print(" ");
    ez.canvas.print(gps.speed.knots(), 1);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  // Course over ground
  if (gps.course.isValid()) {
    ez.canvas.y(ez.canvas.top() + 70);
    ez.canvas.x(ez.canvas.lmargin() + 160);
    ez.canvas.print("COG:");
    ez.canvas.print(gps.course.deg(), 0);
    M5.Lcd.drawEllipse(ez.canvas.lmargin() + 276, ez.canvas.top() + 73, 3, 3, ez.theme->foreground);

  }
  else
  {
    Serial.print(F("INVALID"));
  }


  // Date
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  // Time
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) {
      Serial.print(F("0"));
    }
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) {
      Serial.print(F("0"));
    }
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) {
      Serial.print(F("0"));
    }
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) {
      Serial.print(F("0"));
    }
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  // We are cheating a little here, by not taking into account units on wind speed (Knots or Meters/sec)
  // Wind
  // Apparent Wind
  if (String(windReference.value()) == "R") {
    ez.canvas.y(ez.canvas.top() + 100);
    ez.canvas.x(ez.canvas.lmargin());
    M5.lcd.fillRect(ez.canvas.lmargin(), ez.canvas.top() + 100, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("AWS:");
    String windSpee = windSpeed.value();
    if (windSpee.toFloat() < 10)  ez.canvas.print(" ");
    ez.canvas.print(windSpeed.value());
    ez.canvas.y(ez.canvas.top() + 100);
    ez.canvas.x(ez.canvas.lmargin() + 160);
    M5.lcd.fillRect(ez.canvas.lmargin() + 160, ez.canvas.top() + 100, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("AWA:");
    String windAng = windAngle.value();
    int trueWindangle = windAng.toInt();
    int realAppWindangle;
    if ((gps.course.deg() + trueWindangle) <= 360) {
      realAppWindangle = gps.course.deg() + trueWindangle;
    } else {
      realAppWindangle = (gps.course.deg() + trueWindangle) - 360;
    }
    ez.canvas.print(realAppWindangle);
    M5.Lcd.drawEllipse(ez.canvas.lmargin() + 276, ez.canvas.top() + 103, 3, 3, ez.theme->foreground);
  }
  // True Wind
  else if (String(windReference.value()) == "T") {
    ez.canvas.y(ez.canvas.top() + 130);
    ez.canvas.x(ez.canvas.lmargin());
    M5.lcd.fillRect(10, ez.canvas.top() + 130, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("TWS:");
    String windSpee = windSpeed.value();
    if (windSpee.toFloat() < 10)  ez.canvas.print(" ");
    ez.canvas.print(windSpee);
    ez.canvas.y(ez.canvas.top() + 130);
    ez.canvas.x(ez.canvas.lmargin() + 160);
    M5.lcd.fillRect(ez.canvas.lmargin() + 160, ez.canvas.top() + 130, 320, 23, ez.theme->background); //erase partial place for updating data
    ez.canvas.print("TWA:");
    String windAng = windAngle.value();
    int trueWindangle = windAng.toInt();
    int realTruWindangle;
    if ((gps.course.deg() + trueWindangle) <= 360) {
      realTruWindangle = gps.course.deg() + trueWindangle;
    } else {
      realTruWindangle = (gps.course.deg() + trueWindangle) - 360;
    }
    ez.canvas.print(realTruWindangle);
    M5.Lcd.drawEllipse(ez.canvas.lmargin() + 276, ez.canvas.top() + 133, 3, 3, ez.theme->foreground);
  }

  Serial.println();
}

// #########################################################################
// Draw an arc with a defined thickness (modified to aid drawing spirals)
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 3 degree segments to draw (120 => 360 degree arc)
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same an arc of a circle is drawn

int fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  // Make the segment size 7 degrees to prevent gaps when drawing spirals
  byte seg = 7; // Angle a single segment subtends (made more than 6 deg. for spiral drawing)
  byte inc = 6; // Draw segments every 6 degrees

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * DEG2RAD);
    float sy = sin((i - 90) * DEG2RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    M5.Lcd.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    M5.Lcd.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
  }
  return 0;
}

void drawWindScreen() {
  // Draw a compass rose
  M5.Lcd.fillEllipse(ez.canvas.lmargin() + 160, ez.canvas.top() + 100, 93, 93, ez.theme->foreground);
  M5.Lcd.fillEllipse(ez.canvas.lmargin() + 160, ez.canvas.top() + 100, 90, 90, ez.theme->background);
  circleCenterX = ez.canvas.lmargin() + 160;
  circleCenterY = ez.canvas.top() + 100;
  // do the small ticks
  int roseAnglemark = 0;
  while (roseAnglemark < 360) {
    float roseAnglemarkradian = ((roseAnglemark - 90) * 71) / 4068.0;
    M5.Lcd.drawLine (int(circleCenterX + (80 * cos(roseAnglemarkradian))), int(circleCenterY + (80 * sin(roseAnglemarkradian))), int(circleCenterX + (90 * cos(roseAnglemarkradian))), int(circleCenterY + (90 * sin(roseAnglemarkradian))), ez.theme->foreground);
    roseAnglemark += 15;
  }
  // do the longer ticks
  roseAnglemark = 0;
  while (roseAnglemark < 360) {
    float roseAnglemarkradian = ((roseAnglemark - 90) * 71) / 4068.0;
    M5.Lcd.drawLine (int(circleCenterX + (70 * cos(roseAnglemarkradian))), int(circleCenterY + (70 * sin(roseAnglemarkradian))), int(circleCenterX + (90 * cos(roseAnglemarkradian))), int(circleCenterY + (90 * sin(roseAnglemarkradian))), TFT_RED);
    roseAnglemark += 45;
  }

  // put red and green on each side
  fillArc(circleCenterX, circleCenterY, 45, 14, 93, 93, 5, TFT_GREEN);
  fillArc(circleCenterX, circleCenterY, 210, 14, 93, 93, 5, TFT_RED);

  // put App and True on left and right
  ez.canvas.pos(ez.canvas.lmargin() + 10, ez.canvas.top() + 10);
  ez.canvas.println("App");
  ez.canvas.pos(ez.canvas.lmargin() + 250, ez.canvas.top() + 10);
  ez.canvas.print("True");
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  // Start M5ez
#include <themes/default.h>
#include <themes/dark.h>
#include <themes/heyya.h>
  ezt::setDebug(INFO);
  ez.begin();
  // Connect MqTT
  client.begin(mqtt_server, net);
  client.onMessageAdvanced(messageReceived);
  connect();
}

void loop() {
  ezMenu mainmenu("HeyYA Info System");
  mainmenu.txtBig();
  mainmenu.addItem("Location", location_display);
  mainmenu.addItem("Wind", wind_display);
  mainmenu.addItem("Engine", battery_display);
  mainmenu.addItem("Electrical", electrical_display);
  mainmenu.addItem("Settings", ez.settings.menu);
  mainmenu.upOnFirst("last|up");
  mainmenu.downOnLast("first|down");
  mainmenu.run();
}

void location_display() {
  ezMenu locationDisplay;
  ez.header.show("Location");
  ez.buttons.show("Main Menu");
  String btnpressed = ez.buttons.poll();
  while (btnpressed != "Main Menu") {
    btnpressed = ez.buttons.poll();
    client.loop();
    if (gps.location.isUpdated()) {
      displayInfo();
    }
    delay(10);  // <- fixes some issues with WiFi stability
    // Reconnect if we lose the MQTT connection
    if (!client.connected()) {
      connect();
    }
  }
}

void wind_display() {
  int windAngleOld;
  int trueWindOld;
  float appWindSpeedOld;
  float truWindSpeedOld;
  int realWindangle;
  int realTrueWindangle;
  String btnpressed = ez.buttons.poll();

  ezMenu windDisplay;
  ez.header.show("Wind");
  ez.buttons.show("Location # Main Menu # Engine");
  ez.canvas.font(&UbuntuMono_Regular16pt7b);

  drawWindScreen();

  // Run until the Main Menu button is pressed

  while (ez.buttons.poll() == "") {
    btnpressed = ez.buttons.poll();
    if (btnpressed == "Location") location_display();
    client.loop(); // look for MQTT data

    if (gps.location.isUpdated()) {

      // Get and print the apparent wind angle
      if (String(windReference.value()) == "R") {
        String windAng = windAngle.value();
        int apparentWindangle = windAng.toInt();

        // only update the pointer if there is a change
        if (apparentWindangle != windAngleOld) {

          // Clear the center of the rose
          M5.Lcd.fillEllipse(ez.canvas.lmargin() + 160, ez.canvas.top() + 100, 69, 69, ez.theme->background);

          // Draw the indicator on the rose
          float apparentWindangleradian = ((apparentWindangle - 90) * 71) / 4068.0;
          
          //float apparentWindangleradian = ((apparentWindangle - 90) * 71) / 4068.0;
          float pointerWidth = 0.1745; // 10 degrees in radians. This will make the pointer 20 degrees on the bottom

          // Get the coords of the pointer
          int xp = int(circleCenterX + (68 * cos(apparentWindangleradian)));
          int yp = int(circleCenterY + (68 * sin(apparentWindangleradian)));
          int xl = int(circleCenterX + (30 * cos(apparentWindangleradian - pointerWidth)));
          int yl = int(circleCenterY + (30 * sin(apparentWindangleradian - pointerWidth)));
          int xr = int(circleCenterX + (30 * cos(apparentWindangleradian + pointerWidth)));
          int yr = int(circleCenterY + (30 * sin(apparentWindangleradian + pointerWidth)));
          M5.Lcd.fillTriangle(xp, yp, xl, yl, xr, yr, ez.theme->foreground);

          // Now print the relative wind angle in the middle of the rose
          ez.canvas.pos(circleCenterX - 25, circleCenterY - 10);
          if (apparentWindangle > 180) {
            if (abs(apparentWindangle - 360) < 10) ez.canvas.print("0");
            if (abs(apparentWindangle - 360) < 100) ez.canvas.print("0");
            ez.canvas.print(abs(apparentWindangle - 360));
          } else {
            if (apparentWindangle < 10) ez.canvas.print("0");
            if (apparentWindangle < 100) ez.canvas.print("0");
            ez.canvas.print(apparentWindangle);
          }
          windAngleOld = apparentWindangle;

          // What I'm calling "apparentWindangle" here is actually relative wind angle.
          // To get actual apparent wind angle, COG must be added/subtracted
          if (gps.location.isUpdated()) { // Not sure why I have to do this again!
            if ((gps.course.deg() + apparentWindangle) <= 360) {
              realWindangle = gps.course.deg() + apparentWindangle;
            } else {
              realWindangle = (gps.course.deg() + apparentWindangle) - 360;
            }
          }

          // print apparent wind angle and speed on the left
          ez.canvas.pos(ez.canvas.lmargin() + 10, ez.canvas.top() + 40);
          M5.lcd.fillRect(ez.canvas.lmargin() + 10, ez.canvas.top() + 40, 50, 23, ez.theme->background); //erase partial place for updating data
          if (realWindangle < 100) ez.canvas.print("0");
          if (realWindangle < 10) ez.canvas.print("0");
          ez.canvas.println(realWindangle);
          M5.Lcd.drawEllipse(ez.canvas.lmargin() + 63, ez.canvas.top() + 43, 3, 3, ez.theme->foreground);
        }

        String windSpee = windSpeed.value();
        float windSpeedFloat = windSpee.toFloat();
        if (windSpeedFloat != appWindSpeedOld) {
          M5.lcd.fillRect(ez.canvas.lmargin() + 10, ez.canvas.top() + 160, 70, 23, ez.theme->background); //erase partial place for updating data
          ez.canvas.pos(ez.canvas.lmargin() + 10, ez.canvas.top() + 160);
          ez.canvas.print(windSpeedFloat, 0);
          ez.canvas.println("KN");
          appWindSpeedOld = windSpeedFloat;
        }
      }
    }

    // Print the true wind on the right
    if (gps.location.isUpdated()) {
      if (String(windReference.value()) == "T") {
        String windAng = windAngle.value();
        int trueWindangle = windAng.toInt();
        Serial.print("True Wind Angle: ");
        Serial.println(trueWindangle);
        if (trueWindangle != trueWindOld) {
          // Print true wind angle on the upper right
          // Again, according to the NMEA spec, True (or Theoretical) is relative to the bow of the boat.
          // To get actual True wind angle, COG must be added/subtracted
          if ((gps.course.deg() + trueWindangle) <= 360) {
            realTrueWindangle = gps.course.deg() + trueWindangle;
          } else {
            realTrueWindangle = (gps.course.deg() + trueWindangle) - 360;
          }
          trueWindOld = trueWindangle;
          ez.canvas.pos(ez.canvas.lmargin() + 255, ez.canvas.top() + 40);
          M5.lcd.fillRect(ez.canvas.lmargin() + 255, ez.canvas.top() + 40, 50, 23, ez.theme->background); //erase partial place for updating data
          ez.canvas.print(realTrueWindangle);
          M5.Lcd.drawEllipse(ez.canvas.lmargin() + 308, ez.canvas.top() + 43, 3, 3, ez.theme->foreground);
        }

        // Print true wind speed on the lower right
        String windSpee = windSpeed.value();
        float windSpeedFloat = windSpee.toFloat();
        if (windSpeedFloat != truWindSpeedOld) {
          M5.lcd.fillRect(ez.canvas.lmargin() + 245, ez.canvas.top() + 160, 70, 23, ez.theme->background); //erase partial place for updating data
          ez.canvas.pos(ez.canvas.lmargin() + 245, ez.canvas.top() + 160);
          ez.canvas.print(windSpeedFloat, 0);
          ez.canvas.println("KN");
          truWindSpeedOld = windSpeedFloat;
        }
      }
    }
    delay(10);  // <- fixes some issues with WiFi stability
    // Reconnect if we lose the MQTT connection
    if (!client.connected()) {
      connect();
    }
  }
}
void battery_display() {

}
void electrical_display() {
  M5.Lcd.fillScreen(WHITE);
  ez.buttons.wait("OK");

}

