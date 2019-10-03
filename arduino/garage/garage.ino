
unsigned long last;

unsigned request_size = 0;
char request_buf[32];

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
    //Serial.flush();
}

bool readRequest(char* buf, unsigned maxSize)
{
  //assert(maxSize >= 2);
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
  if(millis() > last+INTERVAL)
  {
    send("UPDATE", digitalRead(2));
    last = millis();
  }

  if (readRequest(request_buf, sizeof request_buf))
  {
    Serial.println(request_buf);
    send("RESPONSE", digitalRead(2));
  }
}

