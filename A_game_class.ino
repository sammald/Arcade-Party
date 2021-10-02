#include <SPI.h>
#include <TFT_eSPI.h>
#include <mpu6050_esp32.h>
#include <ArduinoJson.h>


/////////////////////////////////////////
//FIRST GAME ALPHABETICALLY INCLUDE:

TFT_eSPI tft = TFT_eSPI();
MPU6050 imu; //imu object called, appropriately, imu

char user[50]; // to hold inputted username
int current_score;
char roomid[100];
char current_game[50];

struct Vec { //C struct to represent 2D vector (position, vel)
  float x;
  float y;
};

/////////////////////////////////////////////////////////////////////////

const int LOOP_SPEED = 25;
class Game {
  protected:
    int score=0;
    int game_type; //0=snake, 1=pong, 2=ham, 3=jade
    int left_limit = 0; //left side of screen limit
    int right_limit = 127; //right side of screen limit
    int top_limit = 5; //top of screen limit
    int bottom_limit = 159; //bottom of screen limit
    
  public:
  
    virtual void step(uint8_t up=0, uint8_t down=0, uint8_t left=0, uint8_t right=0, float x=0); //each game needs its own step funtion
    virtual bool getgamestate(); //SAYS IF GAME IS IN GAME OVER STATE (false) OR RUNNING (true)

    int getscore(){
      return score;
    }

    void setscore(int val){
      score=val;
    }
    void start_game(){
      score=0;
      //INCLUDE RECTANGLE WITH WORDS "PRESS ANY KEY TO START" ON IT
    }

    void increment_score(int val){
      score= score+val;
    }

    void post_score(bool p_mode){
      Serial.println("GOT TO POST");
      if(p_mode){
        Serial.println("IT POSTED IN PARTY");
        Serial.println(roomid);
        Serial.println(user);
        Serial.println(current_game);
        Serial.println(getscore());
        char body[100]; //for body
        sprintf(body,"room_id=%s&player=%s&game=%s&score=%d", roomid, user, current_game, score);
        int body_len = strlen(body); //calculate body length (for header reporting)
      
        sprintf(request_buffer,"POST /sandbox/sc/team67/Party/party_databases.py HTTP/1.1\r\n");
        strcat(request_buffer,"Host: 608dev-2.net\r\n");
        strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
        sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
        strcat(request_buffer,"\r\n"); //new line from header to body
        strcat(request_buffer,body); //body
        strcat(request_buffer,"\r\n"); //new line
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
        
      }
      char body[100]; //for body
      sprintf(body,"game=%s&score=%d&player=%s",current_game, score,user);
        
      Serial.println("Ready to post");
      int body_len = strlen(body); //calculate body length (for header reporting)
  
      sprintf(request_buffer,"POST /sandbox/sc/team67/Leaderboard/leaderboard.py HTTP/1.1\r\n");
      strcat(request_buffer,"Host: 608dev-2.net\r\n");
      strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //new line
      Serial.println(request_buffer);
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
    }
    void end_game(){
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(30, 30, 1);
        char output[30];
        sprintf(output, "Score: %d    ", score);
        tft.print("GAME OVER");
        tft.setCursor(30, 50, 1);
        tft.print(output);
        tft.setCursor(0, 70, 1);
        tft.print("press next button to       start over");
        score=0;
    }
    
    
};
