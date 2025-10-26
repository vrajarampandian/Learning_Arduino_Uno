// traffic_state_serial_periodic.ino
const int GREEN_PIN  = 8;
const int YELLOW_PIN = 10;
const int RED_PIN    = 12;

const unsigned long GREEN_MS  = 10000UL;
const unsigned long YELLOW_MS = 3000UL;
const unsigned long RED_MS    = 5000UL;

enum State { S_GREEN, S_YELLOW, S_RED };
State state = S_GREEN;
unsigned long stateStart = 0;

unsigned long lastSent = 0;
const unsigned long SEND_INTERVAL = 250; // ms
const unsigned long BAUD = 115200;

void setup() {
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  setStateOutputs(state);
  Serial.begin(BAUD);
  while (!Serial) { /* wait on some boards */ }
  stateStart = millis();
  lastSent = 0;
}

void setStateOutputs(State s) {
  digitalWrite(GREEN_PIN,  s == S_GREEN ? HIGH : LOW);
  digitalWrite(YELLOW_PIN, s == S_YELLOW ? HIGH : LOW);
  digitalWrite(RED_PIN,    s == S_RED ? HIGH : LOW);
}

void sendState() {
  unsigned long now = millis();
  unsigned long elapsed = now - stateStart;
  unsigned long remain;
  switch (state) {
    case S_GREEN:  remain = (elapsed >= GREEN_MS) ? 0 : (GREEN_MS - elapsed);  break;
    case S_YELLOW: remain = (elapsed >= YELLOW_MS) ? 0 : (YELLOW_MS - elapsed); break;
    case S_RED:    remain = (elapsed >= RED_MS) ? 0 : (RED_MS - elapsed);    break;
  }
  // Format: STATE:GREEN:9000
  const char *name = (state == S_GREEN) ? "GREEN" : (state == S_YELLOW) ? "YELLOW" : "RED";
  Serial.print("STATE:"); Serial.print(name); Serial.print(':'); Serial.println(remain);
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - stateStart;

  // state transitions
  switch (state) {
    case S_GREEN:
      if (elapsed >= GREEN_MS) {
        state = S_YELLOW;
        stateStart = now;
        setStateOutputs(state);
      }
      break;
    case S_YELLOW:
      if (elapsed >= YELLOW_MS) {
        state = S_RED;
        stateStart = now;
        setStateOutputs(state);
      }
      break;
    case S_RED:
      if (elapsed >= RED_MS) {
        state = S_GREEN;
        stateStart = now;
        setStateOutputs(state);
      }
      break;
  }

  // Read incoming serial commands (non-blocking)
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.equalsIgnoreCase("REQ")) {
      // immediate reply
      sendState();
    }
  }

  // Periodic broadcast (every SEND_INTERVAL ms)
  if (now - lastSent >= SEND_INTERVAL) {
    sendState();
    lastSent = now;
  }
}
