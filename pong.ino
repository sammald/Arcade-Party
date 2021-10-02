


class Pong: public Game { // Pong Game
    Vec ball_pos;
    Vec ball_vel;
    Vec pad_pos;
    Vec AI_pos;
    bool party_mode=false;
    float pad_speed = 2; 
    float AI_speed = 0.4;
    int ball_rad = 2;
    int HALF_WIDTH = 10;
    int PAD_CLR = TFT_WHITE;
    int BALL_CLR = TFT_WHITE;
    int BKGND_CLR = TFT_BLACK;
    int ball_top_limit = 20+ball_rad;
    int ball_bottom_limit = 159-ball_rad;
    int top_limit = 20+HALF_WIDTH; // limits for paddle and AI
    int bottom_limit = 159-HALF_WIDTH;
    TFT_eSPI* game_tft = &tft;
    int lives;
    bool game_over; //for condition check
    uint32_t reset_timer; // to pause after each point or loss of life
    uint32_t game_over_timer;
    int reset_buffer = 1000;
    bool start=true;
    
  public:
 
    Pong(){
      pad_pos.x = 20;
      pad_pos.y = 80;
      ball_pos.x = 64;
      ball_pos.y = 80;
      int vel_x = rand()%2;
      int vel_y = rand()%2;
      if (vel_x == 0) ball_vel.x = -10;
      else ball_vel.x = 10;
      if (vel_y == 0) ball_vel.y = 10;
      else ball_vel.y = -10;
      AI_pos.x = 107;
      AI_pos.y = 80;
      reset_timer = millis();
      lives = 3;
    }
    void setmode(bool party){
      party_mode=party;
    }
    bool getgamestate(){
      return !game_over;
    }
    
    void step(uint8_t up_inp, uint8_t down_inp,uint8_t left=0, uint8_t right=0,float x=0) {
      if(start==true){
        game_tft->fillScreen(TFT_BLACK);
        game_tft->setCursor(0, 0, 1);
        game_tft->print("Player Score: ");
        game_tft->println(0);
        game_tft->print("Lives: ");
        game_tft->println(3);
        start=false;
      }
      if (game_over == true) { // Stays displaying game over scree
        lives = 3;
          game_over = false;
          game_tft->fillScreen(TFT_BLACK);
          game_tft->setCursor(0, 0, 1);
          game_tft->print("Player Score: ");
          game_tft->println(0);
          game_tft->print("Lives: ");
          game_tft->println(3);
        if ((up_inp == 0 || down_inp == 0) && millis()-reset_timer>1000) { // Checks for button press to restart
          lives = 3;
          game_over = false;
          game_tft->fillScreen(TFT_BLACK);
          game_tft->setCursor(0, 0, 1);
          game_tft->print("Player Score: ");
          game_tft->println(0);
          game_tft->print("Lives: ");
          game_tft->println(3);
        }
      } else if (millis() - reset_timer > reset_buffer) { // Checks if buffer has completed
        player_step(up_inp, down_inp); // Updates player
        ball_step(); // Updates Ball
        AI_step(); // Updates AI
        int new_player_score = player_score_detect(); // Checks for point
        int new_lives = lose_life_detect(); //
        if (new_player_score != getscore()) {
          setscore(new_player_score);
          reset();
        }
        if (new_lives != lives) {
          lives = new_lives;
          reset();
        }
      }
    }
    
    void reset() {
      pad_pos.y = 80;
      ball_pos.x = 64;
      ball_pos.y = 80;
      int vel_x = rand()%2;
      int vel_y = rand()%2;
      if (vel_x == 0) ball_vel.x = -10;
      else ball_vel.x = 10;
      if (vel_y == 0) ball_vel.y = 10;
      else ball_vel.y = -10;
      AI_speed = 0.4;
      AI_pos.y = 80;
      reset_timer = millis();
      game_tft->fillScreen(TFT_BLACK);
      game_tft->drawCircle(ball_pos.x, ball_pos.y, ball_rad, BALL_CLR);
      game_tft->drawLine(AI_pos.x, AI_pos.y + HALF_WIDTH, AI_pos.x, AI_pos.y - HALF_WIDTH, PAD_CLR);
      game_tft->drawLine(pad_pos.x, pad_pos.y + HALF_WIDTH, pad_pos.x, pad_pos.y - HALF_WIDTH, PAD_CLR);
      game_tft->setCursor(0, 0, 1);
      game_tft->print("Player Score: ");
      game_tft->println(getscore());
      game_tft->print("Lives: ");
      game_tft->println(lives);
      if (lives == 0) {
        game_over=true;
        post_score(party_mode);
        end_game();
      }
    }
  private:
    void player_step(uint8_t up_inp, uint8_t down_inp) {
      if (up_inp == 0 && down_inp == 0){
      } else if (up_inp == 0) {
        if ((pad_pos.y - pad_speed) < top_limit) {
        } else {
          game_tft->drawLine(pad_pos.x, pad_pos.y + HALF_WIDTH, pad_pos.x, pad_pos.y - HALF_WIDTH, BKGND_CLR);
          pad_pos.y = pad_pos.y - pad_speed;
        }
      } else if (down_inp == 0) {
        if ((pad_pos.y + pad_speed) > bottom_limit) {
        } else {
          game_tft->drawLine(pad_pos.x, pad_pos.y + HALF_WIDTH, pad_pos.x, pad_pos.y - HALF_WIDTH, BKGND_CLR);
          pad_pos.y = pad_pos.y + pad_speed;
        }
      }
      game_tft->drawLine(pad_pos.x, pad_pos.y + HALF_WIDTH, pad_pos.x, pad_pos.y - HALF_WIDTH, PAD_CLR);
    }
    
    void ball_step(){
      game_tft->drawCircle(ball_pos.x, ball_pos.y, ball_rad, BKGND_CLR);
      if (ball_vel.x > 0 && (ball_pos.x + 0.04*ball_vel.x) >= (AI_pos.x-ball_rad-1)) { //Update ball x position
        if (AI_pos.y - HALF_WIDTH < ball_pos.y && ball_pos.y < AI_pos.y + HALF_WIDTH) { 
          float diff = (ball_pos.x + 0.04*ball_vel.x) - (AI_pos.x-ball_rad-1);
          ball_pos.x = ball_pos.x - diff;
          ball_vel.x = ball_vel.x*(-1);
          int AI_rand = rand()%10;
          if (AI_rand < 4) AI_speed = 0.3;
          else AI_speed = 0.4;
        } else {
          ball_pos.x = ball_pos.x + 0.04*ball_vel.x;
        }
      } else if (ball_vel.x < 0 && (ball_pos.x + 0.04*ball_vel.x) <= (pad_pos.x+ball_rad+1)) {
        if (pad_pos.y - HALF_WIDTH < ball_pos.y && ball_pos.y < pad_pos.y + HALF_WIDTH) {
          float diff = (pad_pos.x+ball_rad+1) - (ball_pos.x + 0.04*ball_vel.x);
          ball_pos.x = ball_pos.x + diff;
          ball_vel.x = ball_vel.x*(-1);
        } else {
          ball_pos.x = ball_pos.x + 0.04*ball_vel.x;          
        }
      } else {
        ball_pos.x = ball_pos.x + 0.04*ball_vel.x;
      }

      if (ball_vel.y > 0 && (ball_pos.y + 0.04*ball_vel.y) > ball_bottom_limit) { //Update ball y position
        float diff = (ball_pos.y + 0.04*ball_vel.y) - ball_bottom_limit;
        ball_pos.y = ball_bottom_limit - diff;
        ball_vel.y = ball_vel.y*(-1);
      } else if (ball_vel.y < 0 && (ball_pos.y + 0.04*ball_vel.y) < ball_top_limit) {
        float diff = ball_top_limit - (ball_pos.y + 0.04*ball_vel.y);
        ball_pos.y = ball_top_limit + diff;
        ball_vel.y = ball_vel.y*(-1);
      } else {
        ball_pos.y = ball_pos.y + 0.04*ball_vel.y;
      }
      game_tft->drawCircle(ball_pos.x, ball_pos.y, ball_rad, BALL_CLR);
    }
    
    void AI_step() {
      if (ball_pos.y < AI_pos.y) {
        if ((AI_pos.y - AI_speed) < top_limit) {
        } else {
          game_tft->drawLine(AI_pos.x, AI_pos.y + HALF_WIDTH, AI_pos.x, AI_pos.y - HALF_WIDTH, BKGND_CLR);
          AI_pos.y = AI_pos.y - AI_speed;
        }
      } else if (ball_pos.y > AI_pos.y) {
        if ((AI_pos.y + AI_speed) > bottom_limit) {
        } else{
          game_tft->drawLine(AI_pos.x, AI_pos.y + HALF_WIDTH, AI_pos.x, AI_pos.y - HALF_WIDTH, BKGND_CLR);
          AI_pos.y = AI_pos.y + AI_speed;
        }
      }
      game_tft->drawLine(AI_pos.x, AI_pos.y + HALF_WIDTH, AI_pos.x, AI_pos.y - HALF_WIDTH, PAD_CLR);
    }
    
    int player_score_detect() {
      if (ball_pos.x > (AI_pos.x - ball_rad)) return getscore()+1;
      else return getscore();
    }

    int lose_life_detect() {
      if (ball_pos.x < (pad_pos.x +ball_rad)) return lives-1;
      else return lives;
    }
};
