#define BALL_COLOR TFT_GREEN



class Snake {
    Vec vel;
    int buff;
    int body_len;
    int index;
    float x_pos[1000];
    float y_pos[1000];
    int BALL_CLR;
    int BKGND_CLR;
    int RADIUS; //radius of ball
    int DeltaT; //timing for integration
    float old_x;
    float old_y;
    int state; //set to 1 when gameover
    int reset_timer;
    int LEFT_LIMIT; //MAYBE FIND WAY TO NOT HAVE TO HAVE THESE
    int RIGHT_LIMIT;
    int TOP_LIMIT;
    int BOTTOM_LIMIT;
  public:
    Snake(int dt, int rad = 2, float ms = 1,
         int ball_color = TFT_GREEN, int background_color = TFT_BLACK,
         int left_lim = 0, int right_lim = 127, int top_lim = 5, int bottom_lim = 159) {
      x_pos[0] = (float)64; //x position
      y_pos[0] = (float)80; //y position
      vel.x = 0; //x velocity
      vel.y = 0; //y velocity
      index=0;
      body_len=1;
      BALL_CLR = ball_color; //ball color
      BKGND_CLR = background_color;
      RADIUS = rad; //radius of ball
      LEFT_LIMIT = left_lim + RADIUS; 
      RIGHT_LIMIT = right_lim - RADIUS; //right side of screen limit
      TOP_LIMIT = top_lim + RADIUS; //top of screen limit
      BOTTOM_LIMIT = bottom_lim - RADIUS; //bottom of screen limit
      DeltaT = dt; //timing for integration
      buff=0;
      old_x=0;
      state=0;
    }
    
    void step(uint8_t left, uint8_t right, uint8_t up, uint8_t down) {
      
      if(state==1){
        reset();
        if((left==0 or right==0 or up==0 or down==0)and millis()-reset_timer>1000){
          reset();
        }
      }else{
        
        if(left==0){
          vel.x=-35;
          vel.y=0;
        }else if(right==0){
          vel.x=35;
          vel.y=0;
        }else if(up==0){ 
          vel.x=0;
          vel.y=-35;
        }else if(down==0){
          vel.x=0;
          vel.y=35;
        }
  
        if(buff==0){
          int debugrad=RADIUS;
          if(body_len==21){debugrad=RADIUS+2;}
          tft.fillCircle(gettailX(), gettailY(), debugrad, BKGND_CLR);
        }else{
          buff=buff-1;
        }
  
  
        moveSnake();
        float new_x=getheadX();
        //float new_y=getheadY();
        
        if(abs(new_x-old_x)>RADIUS /*|| abs(new_y-old_y)>RADIUS*/){
          float old_x=getheadX();
          //old_y=getheadY();
          tft.fillCircle(getheadX(), getheadY(), RADIUS, TFT_GREEN);
        }
      }
      
      
    }
    
    void reset(int x = 64, int y = 32) { 
      tft.fillScreen(TFT_BLACK);
      x_pos[0] = (float)64; //x position
      y_pos[0] = (float)80; //y position
      vel.x = 0; //x velocity
      vel.y = 0; //y velocity
      index=0;
      body_len=1;
      buff=0;
      old_x=0;
      state=0;
    }
    
    float gettailX() {
      return x_pos[index];
    }
    float gettailY() {
      return y_pos[index];
    }
    float getheadX() {
      int ci=getheadindex();
      return x_pos[ci];
    }
    float getheadY() {
      int ci=getheadindex();
      return y_pos[ci];
    }
    int getheadindex(){
      int current_index;
      if(index==0){
        current_index=body_len-1-buff;
      }else if(body_len==21){ //if we just added more to new body //IF YOU CHANGE BUFF CHANGE THIS NUMBER
        current_index=index-1;
      }else{
        current_index=index-1-buff; //YOU ADDED MINUS BUFF AND NOW IT AUTOMATICALLY ENDS GAME, MUST BE INDEXING INTO ZEROS, TRY REMOVING BUFF AN
      }
      return current_index;
    }
    
    void addbody(){
      int added_len=20;
      body_len=body_len+added_len;
      buff=added_len+1;
      if((index==0 and body_len==added_len+1)){//if we just added more to new body, shift tail to the right one
        index++;
      }else{
        for(int i=body_len-added_len-1; i>=index; i--){ 
          x_pos[i+added_len]=x_pos[i];
          y_pos[i+added_len]=y_pos[i];
        }
        index=index+added_len;
      }
    }
    
    int getstatus(){ 
      return state;
    }
    int setstatus(int val){
      state=val;
    }
  private:
    
    
    void moveSnake() {
      float new_x=getheadX() + .001*DeltaT*vel.x; 
      float new_y=getheadY() + .001*DeltaT*vel.y;
      for(int i=0; i<body_len; i++){ //can alter to check radius rather than point
        if(body_len>1 and buff==0 and (x_pos[i]==new_x and y_pos[i]==new_y)){
          reset_timer=millis();
          state=1;
        }
      }

      if(buff==0 || body_len==21){ //
        x_pos[index]=new_x;
        y_pos[index]=new_y;
      }else{
        Serial.print("FILLING NEW HEAD VAL AT INDEX: ");
        Serial.println(index-buff-1);
        x_pos[index-buff]=new_x;
        y_pos[index-buff]=new_y;
      }
      int expected_head=index-1;
      if(index==0){expected_head=body_len-1;}
      
      if(getheadindex() != expected_head){
        //do nothing
      }else if(index==body_len-1){
        Serial.print("SAID INDEX REACHED MAX: ");
        Serial.println(index);
        index=0;
      }else{
        index++;
      }
      
      if(new_x > RIGHT_LIMIT or new_x< LEFT_LIMIT or new_y > BOTTOM_LIMIT or new_y< TOP_LIMIT){ //TRY SWITCHING SIGNS IF HAVING ISSUES
        Serial.println("WENT OUT OF BOUNDS");
        state=1;
      }
    }
};

class SnakeG: public Game {

    Vec food_pos;  //pos of food
    bool party_mode=false;
    int food_half_width; // Like "radius", except it's a square
    int player_radius = 3;
    int showscreen; //do you show gameover screen? 0==yes, 1==no
    int step_timer;
  public:
    Snake player;
    SnakeG(): 
      player(LOOP_SPEED, player_radius, 1, TFT_RED, TFT_BLACK,
             left_limit, right_limit, top_limit, bottom_limit) //DOES THIS TAKE THE GAME CLASSES VALUES
    {
      food_pos.x = 40; 
      food_pos.y = 40; 
      food_half_width = 3;
      showscreen=0;
      start_game();
      step_timer=millis();
    }

    void setmode(bool party){
      party_mode=party;
    }
    
    bool getgamestate(){
      if(player.getstatus()==1){
        return false;
      }
      return true;
    }

    void step(uint8_t up, uint8_t down, uint8_t left, uint8_t right, float x =0) {
      if(millis()-step_timer>25){
        player.step(left, right, up, down); 
        int gamestate=player.getstatus();
  
        if(gamestate==1 and showscreen==0){
          post_score(party_mode);
          end_game();
          showscreen=1;
        }
  
        else if(gamestate==0){
          showscreen=0;
          int new_score = collisionDetect(); //checks if collision occurred (food found/lost)
          if (new_score != getscore()) {
            setscore(new_score);
            tft.fillRect(food_pos.x - food_half_width, food_pos.y - food_half_width, 2 * food_half_width, 2 * food_half_width, TFT_BLACK); //writes over old food
            food_pos.x = random(right_limit - left_limit - 2 * food_half_width) //gets new food coord
                         + left_limit + food_half_width;
            food_pos.y = random(bottom_limit - top_limit - 2 * food_half_width)
                         + top_limit + food_half_width;
          }
          
          int top_left_x = food_pos.x - food_half_width;
          int top_left_y = food_pos.y - food_half_width;
          int side = 2 * food_half_width;
          tft.fillRect(top_left_x, top_left_y, side, side, TFT_RED); //places new food
          tft.setCursor(0, 0, 1);
          char output[30];
          sprintf(output, "Score: %d    ", getscore());
          tft.print(output);
        }
        step_timer=millis();
    }
    }
    
      
    int collisionDetect() {
      float playerx=player.getheadX();
      float playery=player.getheadY();
      float foodx=food_pos.x;
      float foody=food_pos.y;
      float ydiff=abs(foody-playery);
      float xdiff=abs(foodx-playerx);
      float thres=player_radius+food_half_width;
      
      if(ydiff>=thres or xdiff>=thres){
        return getscore();
      }
      
      player.addbody();
      return getscore()+1; 
  
    }
   
};
