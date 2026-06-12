#include <ESP8266WiFi.h>

WiFiServer server(80);

#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
#define ENA D7
#define ENB D8

#define TRIG D5
#define ECHO D6

int speedVal = 135;    
int autoSpeed = 135;   

bool obstacleMode = false;

long duration;
int distance;

void setSpeed() {
  analogWrite(ENA, speedVal);
  analogWrite(ENB, speedVal);
}

void setAutoSpeed() {
  analogWrite(ENA, autoSpeed);
  analogWrite(ENB, autoSpeed);
}

void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void forward() {
  setSpeed();
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void backward() {
  setSpeed();
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void left() {
  setSpeed();
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void right() {
  setSpeed();
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

int getDistance() {
  digitalWrite(TRIG LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 30000);

  if (duration == 0) return 100;
  return duration * 0.034 / 2;
}


void obstacleAvoid() {
  int dist = getDistance();

  setAutoSpeed();   

  if (dist < 20) {
    stopCar();
    delay(100);

    backward();
    delay(200);

    stopCar();
    delay(80);

    if (millis() % 2 == 0) left();
    else right();

    delay(300);

  } else {
    forward();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  stopCar();

  WiFi.softAP("ESP_CAR", "12345678");
  Serial.println(WiFi.softAPIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (obstacleMode) {
    obstacleAvoid();
  }

  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/toggle") != -1) {
      obstacleMode = !obstacleMode;
      stopCar();
    }

    if (!obstacleMode) {
      if (request.indexOf("/forward") != -1) forward();
      else if (request.indexOf("/back") != -1) backward();
      else if (request.indexOf("/left") != -1) left();
      else if (request.indexOf("/right") != -1) right();
      else if (request.indexOf("/stop") != -1) stopCar();
    }

    int dist = getDistance();

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");

    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP Car</title>

<style>
body { font-family: Arial; text-align:center; background:#0f172a; color:white; }
h2 { color:#38bdf8; }
button {
  width:110px; height:50px; margin:8px;
  font-size:15px; border-radius:12px;
  border:none; background:#22c55e; color:white;
}
.toggle { background:#f59e0b; }
.stop { background:#ef4444; }
</style>

</head>

<body>

<h2> ESP Smart Car</h2>

<p>Obstacle Mode: )rawliteral");

    client.print(obstacleMode ? "ON" : "OFF");

    client.println(R"rawliteral(</p>

<a href="/toggle"><button class="toggle">Toggle Mode</button></a><br>

<a href="/forward"><button>Forward</button></a><br>
<a href="/left"><button>Left</button></a>
<a href="/stop"><button class="stop">Stop</button></a>
<a href="/right"><button>Right</button></a><br>
<a href="/back"><button>Backward</button></a><br>

<p>Distance: )rawliteral");

    client.print(dist);

    client.println(R"rawliteral( cm</p>

</body>
</html>
)rawliteral");
  }
}