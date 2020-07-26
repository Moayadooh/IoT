#include <EasyButton.h>

#define BUTTON_PIN 0
EasyButton button(BUTTON_PIN);

void setup() {
    //pinMode(0, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Turn off
    button.begin();
    button.onPressed(onPressed);
}

void loop() {
    button.read();
}

boolean flag = false;
void onPressed() {
    if(!flag){
      flag = true;
      digitalWrite(LED_BUILTIN, LOW); // Turn on
    } 
    else{
      flag = false;
      digitalWrite(LED_BUILTIN, HIGH); // Turn off
    }
}
