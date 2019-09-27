int count = 0;

void setup() {
  //start serial connection
  Serial.begin(9600);
}

void loop() {
  char s[1024];
  memset(s, '0', 1024);
  sprintf(s, "Counter is now %d\n", count++);
  Serial.print(s);
  delay(1000);
}

