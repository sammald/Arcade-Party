#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <mpu6050_esp32.h>
#include<math.h>

#define CLK 34
#define DT 35
#define SW 25
#define X_pin 32
#define Y_pin 33
#define AUDIO A0


float x, y, z; //variables for grabbing x,y,and z values

const uint8_t BUTTON1 = 25; // JOE IT LEFT

class Joe_It: public Game {
    bool party_mode=false;
    int game_state;
    unsigned long timer;
    bool gameOver=false;

    static const uint8_t STABLE = 0;
    static const uint8_t PEAK = 1;
    static const uint8_t TROUGH = 2;

    static const uint8_t UP = 3;
    static const uint8_t DOWN = 4;
    static const uint8_t LEFT = 5;
    static const uint8_t RIGHT = 6;

    static const uint8_t PRE = 28;
    static const uint8_t START = 20;
    static const uint8_t GO = 26;
    static const uint8_t COMMAND = 21;
    static const uint8_t WAIT = 22;
    static const uint8_t UNPRESS = 27;
    static const uint8_t CHECK = 23;
    static const uint8_t UPDATE = 24;
    static const uint8_t LOSE = 25;
    
    // input readings
    uint8_t button_state;
    uint8_t motion_state = STABLE;
    uint8_t currentStateCLK;
    uint8_t lastStateCLK;
    bool spun;
    uint8_t joyDir;
    int volume;

    float old_acc_mag;  //previous acc mag
    float older_acc_mag;  //previous prevoius acc mag
    
    float peak_level = 14.0/9.81;
    float trough_level = 9.8/9.81;

    char current_move[20];
    char chosen_move[20];
    char moves[7][20] = {
                         "Joe it LEFT!",
                         "Joe it RIGHT!",
                         "Joe it UP!",
                         "Joe it DOWN!",
                         "Shake it!",
                         "Twist it!",
                         "Yell it!",
                     };
    
    int left_limit = 0; //left side of screen limit
    int right_limit = 127; //right side of screen limit
    int top_limit = 5; //top of screen limit
    int bottom_limit = 159; //bottom of screen limit

    int showscreen; //do you show gameover screen? 0==yes, 1==no
    
  public:
    Joe_It() {
      setscore(0);
      game_state = PRE;
      timer = 0;
      spun = false;
      chosen_move[0] = '\0';
      gameOver=false;
      
    }
    void setmode(bool party){
      party_mode=party;
    }
    
    void step(uint8_t up=0, uint8_t down=0, uint8_t left=0, uint8_t right=0, float x=0) {
//      Serial.println(game_state);
      game_fsm();
      
    }

    bool getgamestate() {
      return !gameOver;
    }

    
  private:

    void game_fsm() {

      switch(game_state) {

        case PRE:
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.println("Joe-It to start!");
          game_state = START;
        break;

        case START:
          // upon button press to start
          if (digitalRead(BUTTON1) == 0) {
            Serial.println("here");
            game_state = GO;
            Serial.println(game_state);
          }
        break; 

        case GO:
          if (digitalRead(BUTTON1) == 1) {
            game_state = COMMAND;
          }
        break;
        
        case COMMAND:
          Serial.println("reached");
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 10, 1);
          char output[50];
          sprintf(output, "Score: %d", getscore());
          tft.println(output);

          chosen_move[0] = '\0';
          select_move();
          tft.println(current_move);
          Serial.println(current_move);

          lastStateCLK = digitalRead(CLK);
          timer = millis();
          Serial.println(timer);
          game_state = WAIT;
        break;

        case WAIT:
//          Serial.println(game_state);
          read_inputs();
          
          if (millis() - timer > 5000) {
            Serial.println("time out");
            Serial.println(millis()-timer);
            gameover();
            timer = millis();
            game_state = LOSE;
            gameOver = true;
            break;
          }
          
          if (motion_state == TROUGH) {
            Serial.println("shake");
            strcpy(chosen_move, moves[4]);
            game_state = CHECK;
          } else if (joyDir == RIGHT) {
            Serial.println("joystick right");
            strcpy(chosen_move, moves[1]);
            Serial.println(chosen_move);
            Serial.println(current_move);
            game_state = CHECK;
          } else if (joyDir == LEFT) {
            Serial.println("joystick left");
            strcpy(chosen_move, moves[0]);
            Serial.println(chosen_move);
            Serial.println(current_move);
            game_state = CHECK;
          } else if (joyDir == UP) {
            Serial.println("joystick UP");
            strcpy(chosen_move, moves[2]);
            Serial.println(chosen_move);
            Serial.println(current_move);
            game_state = CHECK;
          } else if (joyDir == DOWN) {
            Serial.println("joystick down");
            strcpy(chosen_move, moves[3]);
            Serial.println(chosen_move);
            Serial.println(current_move);
            game_state = CHECK;
          } else if (spun) {
            Serial.println("twisted");
            strcpy(chosen_move, moves[5]);
            game_state = CHECK;
          } else if (volume > 2800) {
            Serial.println("yelled");
            strcpy(chosen_move, moves[6]);
            game_state = CHECK;
          }
        break;

        case CHECK:
          if (strcmp(chosen_move, current_move) == 0) {
            game_state = UPDATE;
            timer = millis();
          } else {
            gameover();
            timer = millis();
            gameOver = true;
            game_state = LOSE;
          }
        break;

        case UPDATE:
          if (millis() - timer > 500) {
            increment_score(1);
            game_state = COMMAND;
          }   
        break;

        case LOSE:
          if (millis() - timer > 5000) {
            game_state = PRE;
            gameOver = false;
          }
        break;

      }
    }

    void select_move() {
      int index = random(7);
      Serial.println(index);
      strcpy(current_move, moves[index]);
      Serial.println("move");
      Serial.println(current_move);
      Serial.println("end");
    }


    void read_inputs() {
      button_state = digitalRead(BUTTON1);
      volume = analogRead(AUDIO);
      Serial.println(volume);

      // read joystick
      int x_joy = analogRead(X_pin);
      int y_joy = analogRead(Y_pin);
    
      if (x_joy >3000) {
        joyDir = RIGHT;
      } else if (x_joy < 800) {
        joyDir = LEFT;
      } else if (y_joy > 3000) {
        joyDir = DOWN;
      } else if (y_joy < 800) {
        joyDir = UP;
      } else {
        joyDir = 0;
      }

      // read rotary encoder
      currentStateCLK = digitalRead(CLK);

      if (currentStateCLK != lastStateCLK){
        spun = true;
      } else {
        spun = false;
      }

      lastStateCLK = currentStateCLK;

      //get IMU information:
      imu.readAccelData(imu.accelCount);
      float x, y, z;
      x = imu.accelCount[0] * imu.aRes;
      y = imu.accelCount[1] * imu.aRes;
      z = imu.accelCount[2] * imu.aRes;
      float acc_mag = sqrt(x * x + y * y + z * z);
      float avg_acc_mag = 1.0 / 3.0 * (acc_mag + old_acc_mag + older_acc_mag);
      older_acc_mag = old_acc_mag;
      old_acc_mag = acc_mag;

      motion_fsm(avg_acc_mag);

    }

    void motion_fsm(float avg_acc_mag) {
      switch(motion_state) {
        case STABLE:
          //check for peak
          if (avg_acc_mag > peak_level) {
            motion_state = PEAK;
          }
          break;
        case PEAK:
          //check for trough
          if (avg_acc_mag < trough_level) {
            motion_state = TROUGH;
          }
          break;
        case TROUGH:
          // this state signals completed gesture
          motion_state = STABLE;
          break;
      }
    }

    void gameover(){ 
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(30, 30, 1);
        char output[30];
        sprintf(output, "Score: %d    ", getscore());
        tft.print("GAME OVER");
        tft.setCursor(30, 50, 1);
        tft.print(output);
        tft.setCursor(0, 70, 1);

        
        post_score(party_mode);
        setscore(0);
      }
      
   
};
