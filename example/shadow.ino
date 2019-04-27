#include "mjson.c"  // Sketch -> Add File -> Add mjson.c

static int ledOn = 0;  // Current LED status

// Gets called by the RPC engine to send a reply frame
static int sender(const char *frame, int frame_len, void *privdata) {
  return Serial.write(frame, frame_len);
}

static void reportState(void) {
  jsonrpc_call(sender, NULL, "{\"method\":\"Shadow.Report\",\"params\":{\"on\":%s}}",
               ledOn ? "true" : "false");
}

void setup() {
  jsonrpc_init(NULL, NULL);

  // Export "Shadow.Delta". Pass a callback that updates ledOn
  jsonrpc_export("Shadow.Delta", [](struct jsonrpc_request *r) {
    ledOn = mjson_get_bool(r->params, r->params_len, "$.on", 0);
    digitalWrite(LED_BUILTIN, ledOn); // Set LED to the "on" value
    reportState();                    // Let shadow know our new state
    jsonrpc_return_success(r, NULL);  // Report success
  }, NULL);

  pinMode(LED_BUILTIN, OUTPUT);   // Configure LED pin
  Serial.begin(115200);           // Init serial comms
  reportState();                  // Let shadow know our state
}

// static int uptime = 0;   // Time after last reboot in seconds
void loop() {
  if (Serial.available() > 0) jsonrpc_process_byte(Serial.read(), sender, NULL);
  // if (millis() / 1000 > uptime) {
  //   uptime = millis() / 1000;
  //   jsonrpc_call("{\"method\":\"Shadow.Report\",\"params\":{\"uptime\":%d}}", uptime);
  // }
}
