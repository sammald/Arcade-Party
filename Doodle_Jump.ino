#include <SPI.h>
#include <TFT_eSPI.h>
#include <stdlib.h> 
#include <mpu6050_esp32.h>
#include <list>

#define BACKGROUND TFT_BLACK


//TAKE OUT SCORE
class Obstacle_Doodle {
  int heigt; 
  int widt; 
  struct Vec pos; 
  int min_heigt = 3;
  int min_wid = 10;
  int max_wid = 20; 
  int bottom_limit = 159; //bottom of screen limit
  TFT_eSPI* local_tft; 
  public: 
  Obstacle_Doodle(){ 
      heigt= 3;
      widt = rand() % max_wid + min_wid;
      pos.x = rand() % 127-2*widt + widt;
      pos.y = 0;
  }
  void move_obstacle (int jump_height){
    tft.fillRect(pos.x,pos.y,widt,heigt,TFT_BLACK);
    pos.y += jump_height; 
    if (pos.y > bottom_limit){ 
      pos.y = 0; 
      pos.x  = rand() % 127-2*widt + widt; 
    }
    tft.fillRect(pos.x,pos.y, widt, heigt, TFT_GREEN);
    
  }
  int get_width(){
    return widt; 
  }
  struct Vec get_location(){
    return pos;
  }
  void sety(int y){
    pos.y = y;
  }
};


class Player_Doodle{ //moves with IMU readings. 
    struct Vec pos; //position of player
    struct Vec vel; //velocity of player
    struct Vec accel; //acceleration of player
  
    const float MASS = 1; //for starters
    const int RADIUS = 3; //radius of ball
    const int LEFT_LIMIT = RADIUS; //left side of screen limit
    const int RIGHT_LIMIT = 127-RADIUS; //right side of screen limit
    float K_SPRING = 1;  //spring coefficient
    const float K_FRICTION = 0.15;  //friction coefficient
    const int DT = 40; //milliseconds
    bool rising; 
    bool bouncing; 
    int height_at_bounce; 
    const int fall_speed = 1; 
    void moveBall(){ //only moves in the x direction 
      float tomovex = .001*vel.x*DT;
      float canmove;
      // y direction motion
      if (bouncing == true){
        pos.y -= 1;
        //has_risen = 0;
        rising = true;
      } else {
        pos.y += fall_speed; 
        rising = false; 
      }
      // x direction motion
      if ((pos.x + tomovex) >= RIGHT_LIMIT) {
        canmove = RIGHT_LIMIT-pos.x; 
        pos.x = pos.x + canmove;
        tomovex = tomovex - canmove;  
        pos.x = tomovex;
      } else if ((pos.x + tomovex) <= LEFT_LIMIT){
        canmove = -pos.x; 
        pos.x = pos.x + canmove;
        tomovex = tomovex - canmove; 
        pos.x = RIGHT_LIMIT - tomovex;
      }else{
        pos.x = pos.x + tomovex;
      }
    }
    public: 
    Player_Doodle(){
      pos.y = 100;
      pos.x = 62;
      rising = true; 
      bouncing = false; 
    }
    void player_step(float x_force, bool bounce){
        tft.fillCircle(pos.x, pos.y, RADIUS, BACKGROUND);
        x_force = x_force - K_FRICTION*vel.x; //imu only moves the player left and right
        accel.x = x_force/MASS;
        vel.x = vel.x + 0.001*DT*accel.x; //integrate, 0.001 is conversion from milliseconds to seconds
        bouncing = bounce;
        moveBall(); 
        if (bouncing){
          bouncing = false;
        }
        tft.fillCircle(pos.x, pos.y, RADIUS, TFT_BLUE); //draw new ball location   
}
  int get_radius(){
    return RADIUS;
  }
  struct Vec get_pos(){
    return pos;
  }
  void set_pos(int x, int y){
    pos.x = x;
    pos.y = y; 
  }
  bool isrising(){
    return rising; 
  }
};
class Doodle_Jump : public Game {
  bool game_over = false;
  bool party_mode=false;
  TFT_eSPI* game_tft = &tft;
  int showscreen = 0; 
  const int SCALER = 500; //how much force to apply to ball
  int state;
  static const uint8_t START = 0;
  static const uint8_t COLLISION  = 1;  
  static const uint8_t PLAYER = 2; 
  static const uint8_t OBSTACLE = 3;
  static const uint8_t LOSE = 4; 
  static const uint8_t RESTART = 5;
  static const uint8_t BOUNCE = 6; 
  bool bounce = false; 
  const int num_obstacles = 8; 
  int bounce_num = 0;
  const int bounce_amt = 30; 

  int last_platform_position = 159;
  const int min_distance_away = 10;  
  void gameover(){ 
        game_tft->fillScreen(TFT_BLACK);
        game_tft->setCursor(30, 30, 1);
        char output[30];
        sprintf(output, "Score: %d    ", getscore());
        game_tft->print("GAME OVER");
        game_tft->setCursor(30, 50, 1);
        game_tft->print(output);
        game_tft->setCursor(0, 70, 1);
        game_tft->print("press any button to start over");
   }
  bool collisionDetect (Obstacle_Doodle obstacle, Player_Doodle player){
      struct Vec pos = player.get_pos(); 
      int RADIUS = player.get_radius(); 
      float right = RADIUS + pos.x; 
      float left = pos.x - RADIUS; 
      float bottom = RADIUS + pos.y; 
      int obstacle_width  = obstacle.get_width();
      struct Vec obstacle_location = obstacle.get_location(); 
      if (obstacle_location.y == bottom){ //bottom of ball in line with obstacle
          if (obstacle_location.x > right){
            return false; 
          }else if (obstacle_location.x + obstacle_width < left){
            return false; 
          } else{
            return true; 
          }
      }else{
          return false; 
      }
  }
  public: 
  Player_Doodle player;
  Obstacle_Doodle obstacles[24]; 
  Doodle_Jump(){
   state = START;
  }
  void step(uint8_t up=1, uint8_t down = 1, uint8_t left=1, uint8_t right=1, float x=0){
    switch (state){
      case START: 
       for (int i; i < num_obstacles; i++){
          
          obstacles[i].sety(rand() % min_distance_away + last_platform_position - bounce_amt);
          last_platform_position = rand() % 100 + 60;
          obstacles[i].move_obstacle(0);
        }
        for (int i = num_obstacles; i < 2*num_obstacles; i++){
          obstacles[i].sety(rand() % 60);
          obstacles[i].move_obstacle(0);
        }
        for (int i =2*num_obstacles; i < 3*num_obstacles; i++){
          obstacles[i].sety(-(rand() % 60));
        }
        state = PLAYER;
       break;
       case PLAYER: 
          bounce = false;
          for (int i; i < 2*num_obstacles; i++){
            if (bounce == false){
              bounce = collisionDetect(obstacles[i], player);
          }
          }
          player.player_step(x*SCALER, bounce);
          for (int i; i < 2*num_obstacles; i++){ //redraw obstacles after player motion
            obstacles[i].move_obstacle(0);
          }
          if (bounce){
            state = BOUNCE; 
          }
          if (player.isrising()){
             if (player.get_pos().y < 79){ //if the player is going up check if is above 79
               state = OBSTACLE;
              }
          } else{
            if (player.get_pos().y > bottom_limit){
            state = LOSE;
          }
          }
       break; 
       case OBSTACLE: 
       if (bounce_num < bounce_amt){
        for (int i; i < 2*num_obstacles; i++){
            obstacles[i].move_obstacle(1);
        }
        bounce_num +=1;
       }else{
        state = PLAYER;
        increment_score(1); 
        bounce_num = 0;
       }
       break;
       case LOSE: 
          gameover(); 
          post_score(party_mode);
          setscore(0);
          state = RESTART; 
      break; 
      case RESTART: 
          if (down == 0|| up == 0 || left == 0|| right ==0 ){
            tft.fillScreen(TFT_BLACK);
            player.set_pos(62,100);
            last_platform_position = 159;
            state = START;
          }  
      break; 
      case BOUNCE:
         if (bounce_num < bounce_amt){
          player.player_step(x*SCALER, true);  
          bounce_num += 1;
         } else{
          state = PLAYER;
          bounce_num = 0;
         }
          
    }   
   }
   void setmode(bool party){
    party_mode=party;
  }
  bool getgamestate(){
      if (state == RESTART){
        return false; 
      } else {
        return true; 
      }
   }

};
