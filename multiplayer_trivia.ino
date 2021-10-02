#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <mpu6050_esp32.h>
#include<math.h>


class Trivia: public Game {
    bool party_mode=false;
    int game_state;
    unsigned long timer;

    bool gameOver;

    char info_str[1000] = "";
    char new_str[1000];
    
    char question[1000];
    char correct_answer[100];
    char user_answer[100];

    static const uint8_t ASK = 0; 
    static const uint8_t ANSWER = 1; 
    static const uint8_t TRUE = 2; 
    static const uint8_t FALSE = 3;
    static const uint8_t REVEAL = 4;
    static const uint8_t NEXT = 5;
    static const uint8_t END = 6;

    uint8_t question_num;

    uint8_t num_count; 
    unsigned long reveal_time; 

    unsigned long answer_duration = 10000;
    unsigned long answer_time;
    
    int left_limit = 0; //left side of screen limit
    int right_limit = 127; //right side of screen limit
    int top_limit = 5; //top of screen limit
    int bottom_limit = 159; //bottom of screen limit

    int showscreen; //do you show gameover screen? 0==yes, 1==no
    
  public:
    Trivia() {
      setscore(0);
      game_state = ASK;
      gameOver = false;
      timer = 0;
      question_num = 1;
      
    }
    
    void step(uint8_t up=0, uint8_t down=0, uint8_t left=0, uint8_t right=0,float x=0) {
//      Serial.println(game_state);
      game_fsm(left,right);
      
    }

    bool getgamestate() {
      return !gameOver;
    }
    void setmode(bool party){
      party_mode=party;
    }
  private:

    void game_fsm(uint8_t left, uint8_t right) {

      switch(game_state) {

        case ASK:
          get_question();
          // display question      
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0,0,1);
          char score_str[500];
          sprintf(score_str, "%s's Score: %d", user, getscore());
          tft.println(score_str);
          tft.println(question);
          tft.println("\r\nTrue or False?");
          timer = millis();
          game_state = ANSWER;
        break; 

        case ANSWER:
          // wait for answer
          if (millis() - timer > answer_duration) {
            Serial.println("time out");
            Serial.println(millis()-timer);
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0,0,1);
            tft.println("Time's up!");
            tft.println("The correct answer was:");
            tft.println(correct_answer);
            timer = millis();
            game_state = NEXT;
            reveal_time = millis();
            break;
          }
          
          if (right == 0) {
            Serial.println("button right press");
            strcpy(user_answer, "False\n");
            Serial.println(user_answer);
            Serial.println(correct_answer);
            tft.println("You answered False");
            answer_time = millis() - timer;
            game_state = FALSE;
          } else if (left == 0) {
            Serial.println("button left press");
            strcpy(user_answer, "True\n");
            Serial.println(user_answer);
            Serial.println(correct_answer);
            Serial.println(strcmp(user_answer, correct_answer));
            tft.println("You answered True");
            answer_time = millis() - timer;
            game_state = TRUE;
          }
        break;

        case TRUE:
          if (millis() - timer > answer_duration) {
            if (left == 1) {
              game_state = REVEAL;
            }
          }
        break;
    
        case FALSE:
          if (millis() - timer > answer_duration) {
            if (right == 1) {
              game_state = REVEAL;
            }
          }
        break;

        case REVEAL:
          //display correct answer
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0,0,1);
          if (strcmp(user_answer, correct_answer) == 0) {
            tft.println("Correct!");
            int score_addition = (int) (10*(1.0 - (float)answer_time/(float)answer_duration));
            Serial.println(score_addition);
            if (score_addition >= 8) {
              tft.println("Wow, that was fast!");
            } else if (score_addition <= 2) {
              tft.println("Answered just in time!");
            }
            char plus[500];
            sprintf(plus, "+ %d!", score_addition+1);
            tft.println(plus);
            setscore(getscore() + 1 + score_addition);
          } else {
            tft.println("Incorrect");
            tft.println("The correct answer was:");
            tft.println(correct_answer);
          }
          char ans_score_str[500];
          sprintf(ans_score_str, "%s's Score: %d", user, getscore());
          tft.println(ans_score_str);
          game_state = NEXT;
          reveal_time = millis();
        break;
        
        case NEXT:
          if (millis() - reveal_time > 2500) {
            question_num++;
            if (question_num > 10) {
              game_state = END;
              gameOver = true;
              gameover();
              timer = millis();
            } else {
              game_state = ASK;
            }
          }
        break;
        
        case END:
        break;

      }
    }

    void get_question() {
      sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/game_files/trivia/multiplayer_trivia_server.py?mode=question&partyid=%s&number=%d HTTP/1.1\r\n", roomid, question_num);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer, "\r\n");
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response_buffer);
      Serial.println("here");
      Serial.println(response_buffer);
    
      char* ending = strrchr(response_buffer, '*');
      Serial.println("strchr");
      Serial.println(ending);
      ending[0] = '\0';
      Serial.println("cleared at ending");
      ending++;
      strcpy(correct_answer, ending);
      Serial.println("correct answer");
      strcpy(question, response_buffer);
      Serial.println("question");
      
    }


    void gameover(){ 
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        tft.println("No more questions!");
        tft.println("Final Score:");
        tft.println(getscore());

        post_score(party_mode);
        setscore(0);
      }
      
   
};
