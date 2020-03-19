#include <Arduino.h>

// three buttons
const int buttonPin1 = 5;
const int buttonPin2 = 6;
const int buttonPin3 = 7;

// common-anode rgb-led (or three individual leds)
const int ledRed = 9;
const int ledGreen = 10;
const int ledBlue = 11;

unsigned long currentTime = 0;
unsigned long lastTime = 0;
int debounceDelay = 50;

struct RGB {
  int red;
  int green;
  int blue;
};

RGB red = {255, 0, 0};
RGB green = {0, 255,0};
RGB blue = {0, 0, 255};
RGB yellow = {255, 255, 0};
RGB white = {255, 255, 255};
RGB black = {0, 0, 0};

#define num_buttons 3

bool displaying = true;
bool listening = false;

RGB colorCodes[] = {yellow, red, white};

#define code_length 4


int code[code_length] = {0, 1, 2, 1}; //code (sequence of colors) to be repeated
int echo[code_length];                // code that is repeated by pressing buttons

int level = 0;    //to which level codes have been displayed
int listen_level = -1;  //level to which codes have been correctly repeated

bool buttonStates[num_buttons] = {0,0,0};
bool lastButtonStates[num_buttons] = {0,0,0};
int buttons[num_buttons] = {buttonPin1, buttonPin2, buttonPin3};

void copyButtonStates(){
  //copy buttonstates array to lastbuttonstates
  for (int i = 0; i < num_buttons; i++){
    lastButtonStates[i] = buttonStates[i];
  }
}



int buttonPressed(){
  currentTime = millis(); 
  int rv = -1;
  Serial.println("buttons");
  for (int i = 0; i < num_buttons; i++){
    buttonStates[i] = digitalRead(buttons[i]);
    
    Serial.print(buttonStates[i]);
    Serial.println();
  }

  

  //check which button changed
  for (int i = 0; i < num_buttons; i++){
    // if there is a change in button state, and the button is pressed, return which button is pressed
    if (currentTime > (lastTime + debounceDelay) && lastButtonStates[i] != buttonStates[i] && buttonStates[i] == true){
      lastButtonStates[i] = buttonStates[i];

      rv = i;
    }

    if(buttonStates[i]){
      lastTime = currentTime;
    }
  }  

  copyButtonStates();
  return rv;
}


//the rgb led is on when pulled low, this is a helper function so 255 will be on. 
int invertValue(int value){
  return (value - 255)*-1;
}

void writeColor(RGB color){
  int red = invertValue(color.red);
  int green = invertValue(color.green);
  int blue = invertValue(color.blue);

  analogWrite(ledRed, red);
  analogWrite(ledGreen, green);
  analogWrite(ledBlue, blue);

}

void flashColor(int times, RGB color){
  for (int i = 0; i < times; i++){
    writeColor(color);
    delay(100);
    writeColor(black);
    delay(100);
  }
  delay(1000);

}



void setup() {
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  Serial.begin(9600);

  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledBlue, HIGH);

  flashColor(10, green);
}

void loop() {
  //delay(1000);
  //display the sequence of colors to be repeated
  if (displaying == true){
    for (int i = 0; i <= level; i++){
      writeColor(colorCodes[code[i]]);
      delay(1000);
      writeColor(black);
      delay(500);
    }
    writeColor(black);
    displaying = false;
    listening = true;
  }

  //listen to the next button input
  if(listening == true){
    int input = buttonPressed();
    if (input != -1 && listen_level <= level){
        listen_level ++;
        echo[listen_level] = input;
        Serial.print("entered ");
        Serial.println(input);
        
      
      //check if the next button in the code is correctly repeated
      for (int i = 0; i <= listen_level; i++){
        if (echo[i] != code[i]){
          Serial.println("Wrong! Resetting back to level 0");
          level = 0;
          listen_level = -1;
          displaying = true;
          listening = false;

          
          flashColor(5, red);
        }
      }
      //code is entirely correctly repeated 
      if (listen_level == level){
        if(level + 1 == code_length){
          Serial.println("won the game!");
          flashColor(25, green);
          level = 0;
          listen_level = -1;
          displaying = true;
          listening = false;

        }else{
          Serial.print("Correct! Progressing to level ");
          Serial.println(level + 1 );
          level ++;
          listen_level = -1;
          listening = false;
          displaying = true;
          
          flashColor(5, green);
        }
      }
    }
  }
}