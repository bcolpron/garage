
unsigned long last;
unsigned request_size = 0;
char request_buf[32];
int last_value = -1;
#define INTERVAL 3000

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  last = millis();
}

void send(const char* command, int val)
{
  Serial.print('[');
  Serial.print(millis());
  Serial.print("] ");
  Serial.print(command);
  Serial.print(" state=");
  Serial.println(val);
}

bool readRequest(char* buf, unsigned maxSize)
{
  if (!Serial.available()) return false;
  int c = Serial.read();
  if ( c == 10 || c == 13)
  {
    buf[request_size] = '\0';
    request_size = 0;
    return true;
  }
  if (request_size >= maxSize -1)
  {
    memcpy(request_buf, request_buf+1, maxSize-2);
    request_size--;
  }
  buf[request_size++] = c;
  return false;
}

void loop() {
  int val = digitalRead(2);
  if(val != last_value || millis() > last+INTERVAL)
  {
    send("UPDATE", val);
    last_value = val;
    last = millis();
  }

  if (readRequest(request_buf, sizeof request_buf))
  {
    if (strcmp(request_buf, "GET state") == 0)
    {
      send("RESPONSE", val);
    }
    else
    {
      Serial.print("Invalid command: ");
      Serial.println(request_buf);
    }
  }
}

