
unsigned long last;

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  last = millis();
}

void send(const char* command, int val)
{
    char s[1024];
    sprintf(s, "RESPONSE state=%d\n", val);
    Serial.print(s);
}

void loop() {
  if(millis() > last+10000)
  {
    send("UPDATE", digitalRead(2));
    last = millis();
  }
}

