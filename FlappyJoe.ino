#include <SPI.h>
#include <TFT_eSPI.h>
#include <stdlib.h> 
#include <WiFiClientSecure.h>
#include <WiFiClient.h>


class Obstacle {
  int heigt; 
  int widt; 
  int y = 159; 
  int x = 0; 
  bool loc; 
  int min_heigt = 20;
  int min_wid = 10;
  int max_wid = 70; 
  TFT_eSPI* local_tft; 
  public: 
  Obstacle(int h = 20, int w = 50, int deltay = 0, bool top = true){ 
      heigt= h;
      widt = rand() % max_wid + min_wid;
      y += rand() % min_heigt + h*2;
      if (rand() %2 == 0){
          loc = true;
      } else{
          loc = false;
    }
  }
  void move_obstacle (){
    if (loc == true){
      x = 127 - widt;
    }
    else{
      x = 0; 
    }
    tft.fillRect(x,y,widt,heigt,TFT_BLACK); 
    y -= 1; 
    tft.fillRect(x,y, widt, heigt, TFT_GREEN);
    if (y < 0){
      y = 159; 
      widt = rand() % max_wid + min_wid;
      if (rand() %2 == 0){
          loc = true;
      } else{
          loc = false;
    }
    }
  }
  int get_width(){
    return widt; 
  }
  int get_y(){
    return y;
  }
  bool get_orient(){
    return loc; 
  }
  void set_y(int new_y){
    y = new_y;
  }
};
class Player{
    float MASS = 1.0; 
    int radius = 2;
    int TOP_LIMIT = 0; 
    int BOTTOM_LIMIT = 127; 
    Vec pos; 
    Vec vel; 
    Vec accel;
    public: 
    Player(){
       pos.y = 20; 
       pos.x = 127/2;
       accel.x = 9.8; 
       vel.x = MASS * accel.y;  
    }
    bool player_step(int BUTTON1){
      if (pos.x < 0 or pos.x > 127) {
        return true;
      }
      tft.fillCircle(pos.x, pos.y,radius, TFT_BLACK); 
      if (BUTTON1 == 1){
          pos.x = pos.x - 1; 
      }
      if (BUTTON1 == 0){
        pos.x = pos.x + 1; //moves up
      }
      tft.fillCircle(pos.x, pos.y, radius, TFT_GREEN); 
      return false;  
    }
    int get_x(){
      return pos.x;
    }
    int get_radius(){
      return radius;
    }
    void reset_x(){
      pos.x = 127/2;
    }
};

class Flappy_Joe : public Game { //Flappy Joe
  bool start=true;
  bool game_over = false;
  bool party_mode=false;
  TFT_eSPI* game_tft = &tft;
  int showscreen = 0; 
  bool collisionDetect (Obstacle obstacle){
      float radius = player.get_radius();
      float center = player.get_x(); 
      bool obstacle_orientation = obstacle.get_orient();
      int obstacle_location  = obstacle.get_width();
      Serial.println(obstacle_location, center);
      if (obstacle_orientation == false){ //on the bottom of the screen
        if ((center - radius) <= obstacle_location){
          return true;
        }
      }
      else if (obstacle_orientation == true){
        if ((center + radius) >= 127 - obstacle_location){
          return true; 
        }
      }
     return false; 
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
        tft.print("press any button to start over");
   }
   void restart(){
      tft.fillScreen(TFT_BLACK);
      //also update obstacle location??
      obstacle1.set_y(bottom_limit); 
      obstacle2.set_y(bottom_limit + 40);
      obstacle3.set_y(bottom_limit + 80);
      obstacle4.set_y(bottom_limit + 120);
      game_over = false; 
      player.reset_x();
      showscreen = 0;
  }
  public: 
  Obstacle obstacle1;
  Obstacle obstacle2; 
  Obstacle obstacle3; 
  Obstacle obstacle4;
 Player player;

  void setmode(bool party){
    party_mode=party;
  }
  
  void step(uint8_t up, uint8_t down, uint8_t left=0, uint8_t right=0, float x=0){
        if(start){
          obstacle1.set_y(bottom_limit);
          obstacle2.set_y(bottom_limit+40);
          obstacle3.set_y(bottom_limit+80);
          obstacle4.set_y(bottom_limit+120);
          start=false;
        }
        if (game_over != true){
          game_over = player.player_step(up);
          obstacle1.move_obstacle();
          if (obstacle1.get_y() <= 22){
            game_over = collisionDetect(obstacle1);
            if (obstacle1.get_y() == 0 && !game_over){
                increment_score(1);
            }
          }
          obstacle2.move_obstacle(); 
          if (obstacle2.get_y() <= 22){
            game_over = collisionDetect(obstacle2); 
            if (obstacle2.get_y() == 0 && !game_over){
                increment_score(1);
            }
          }
          obstacle3.move_obstacle(); 
          if (obstacle3.get_y() <= 22){
            game_over = collisionDetect(obstacle3); 
            if (obstacle3.get_y() == 0 && !game_over){
                increment_score(1);
            }
          }
          obstacle4.move_obstacle(); 
          if (obstacle4.get_y() <= 22){
            game_over = collisionDetect(obstacle4); 
            if (obstacle4.get_y() == 0 && !game_over){
                increment_score(1);
            }
          }
          
          }
        else{
          if (showscreen == 0){
            showscreen = 1; 
            tft.fillScreen(TFT_BLACK); 
            tft.println(getscore());
            gameover();
            post_score(party_mode);
            setscore(0);
         }
         if (down == 0){
          restart();
         }
        }
   }
   bool getgamestate(){
        if(game_over){ post_score(party_mode);}
        return !game_over; 
   }
   
};
