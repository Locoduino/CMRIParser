#include <CMRIBoard.h>
#include <CMRIParser.h>

CMRIParser myParser;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    myParser << Serial.read();
  }
}
