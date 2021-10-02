#include <WiFi.h> //Connect to WiFi Network
#include <string.h>  //used for some string handling and processing.
#include <ArduinoJson.h>




char network[] = "MIT";
char password[] = "";

const int SCREEN_HEIGHT = 160;
const int SCREEN_WIDTH = 128;
const int LOOP_PERIOD = 40; //For consistent looping time
const int WAITING_PERIOD = 10000; // 5 seconds for input to go thru on web client
uint32_t primary_timer; // used for timing
unsigned long update_timer;  //used for storing millis() readings.
char old_input[500]; // to compare if input has changed for printing
char input[500]; // to hold username under construction (being inputted)


char game_mode[50]; // to hold gamemode
int num_games;
int game_count;
char game0[50];
char game1[50];
char game2[50];
char game3[50];
char game4[50];
bool starting=true;
bool drawn=false;
char start[10];


const uint8_t BUTTON_UP = 25; // Buttons to pins for inputs
const uint8_t BUTTON_DOWN = 19;
const uint8_t BUTTON_LEFT = 3;
const uint8_t BUTTON_RIGHT = 5;



//used to get x,y values from IMU accelerometer!
void get_angle(float* x, float* y) {
  imu.readAccelData(imu.accelCount);
  *x = imu.accelCount[0] * imu.aRes;
  *y = imu.accelCount[1] * imu.aRes;
}


//function that makes get request to server to know what game mode is being played
void get_mode() {
  memset(response_buffer, 0, sizeof(response_buffer));
  memset(request_buffer, 0, sizeof(request_buffer));
  int offset = 0;
  offset += sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/ESP_game_mode.py?user=%s HTTP/1.1\r\n", user);
  offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  sprintf(game_mode, "%s", response_buffer);
  game_mode[strlen(game_mode)-1] = '\0';
}


//function that makes get request to store games (in order) to be played
void get_games() {
  memset(response_buffer, 0, sizeof(response_buffer));
  memset(request_buffer, 0, sizeof(request_buffer));
  int offset = 0;
  offset += sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/ESP_games.py?user=%s&mode=%s&roomid=%s HTTP/1.1\r\n", user, game_mode, roomid);
  offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  StaticJsonDocument<300> doc;
  char* str = strchr(response_buffer, '{');
  Serial.println(user);
  Serial.println(game_mode);
  Serial.println(roomid);
  Serial.println(response_buffer);
  DeserializationError error = deserializeJson(doc, str);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  int game_num = doc["num_games"];
  num_games = doc["num_games"];
  while (game_num != 0) {
    if (game_num == 5) {
      const char* st4 = doc["game4"];
      memcpy(game4, st4, strlen(st4));
    } else if (game_num == 4) {
      const char* st3 = doc["game3"];
      memcpy(game3, st3, strlen(st3));
    } else if (game_num == 3) {
      const char* st2 = doc["game2"];
      memcpy(game2, st2, strlen(st2));
    } else if (game_num == 2) {
      const char* st1 = doc["game1"];
      memcpy(game1, st1, strlen(st1));
    } else if (game_num == 1) {
      const char* st0 = doc["game0"];
      memcpy(game0, st0, strlen(st0));
    }
  game_num--;
  }
  sprintf(current_game, "%s", game0);
}


//function to make POST request to server to connect ESP to web client
void attempt_pairing() {
  char body[100]; //for post request body
  sprintf(body, "mode=controller&user=%s", user);
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(request_buffer, "POST http://608dev-2.net/sandbox/sc/team67/login.py HTTP/1.1\r\n", user);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len);
  strcat(request_buffer, "\r\n"); 
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); 
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
}

//Button class to use for user input
class Button {
  public:
  uint32_t state_2_start_time;
  uint32_t button_change_time;    
  uint32_t debounce_duration;
  uint32_t long_press_duration;
  uint8_t pin;
  uint8_t flag;
  bool button_pressed;
  uint8_t state; // This is public for the sake of convenience
  Button(int p) {
  flag = 0;  
    state = 0;
    pin = p;
    state_2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
  }
  void read() {
    uint8_t button_state = digitalRead(pin);  
    button_pressed = !button_state;
  }
  int update() {
  read();
  flag = 0;
  if (state==0) {
    if (button_pressed) {
      state = 1;
      button_change_time = millis();
    }
  } else if (state==1) {
    if (!button_pressed) {
      state = 0;
      button_change_time = millis();
    } else if (millis()-button_change_time >= debounce_duration) {
      state = 2;
      state_2_start_time = millis();
    }
  } else if (state==2) {
    if (!button_pressed) {
      state = 4;
      button_change_time = millis();
    } else if (millis()-state_2_start_time >= long_press_duration) {
      state = 3;
    }
  } else if (state==3) {
    if (!button_pressed) {
      state = 4;
      button_change_time = millis();
    }
  } else if (state==4) {
    if (!button_pressed && millis()-button_change_time >= debounce_duration) {
      state = 0;
      if (millis()-state_2_start_time < long_press_duration) {
        flag = 1;
      } else flag = 2;
    } else if (button_pressed && millis() - state_2_start_time < long_press_duration) {
      state = 2;
      button_change_time = millis();
    } else if (button_pressed && millis() - state_2_start_time >= long_press_duration) {
      state = 3;
      button_change_time = millis();
    }
  }
  return flag;
}
};

//Class used to scroll through alphabet and numbers to set username (wikipedia getter structure)
class Username_input {
    char alphabet[50] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char msg[400] = {0}; //contains previous query response
    char query_string[50] = {0};
    int char_index;
    int state;
    uint32_t scrolling_timer;
    const int scrolling_threshold = 150;
    const float angle_threshold = 0.3;
  public:

    Username_input() {
      state = 0;
      memset(msg, 0, sizeof(msg));
      strcat(msg, "Long Press \nJoe-stick to input \nusername!");
      char_index = 0;
      scrolling_timer = millis();
    }
    void update(float angle, int button, char* output) {
      if (state == 0) { //state 0
          sprintf(output, "%s", msg);
          if (button == 2) {
            state = 1;
            scrolling_timer = millis();
            char_index = 0;
            memset(msg, 0, sizeof(msg));
          } 
      } else if (state == 1) { //state 1
        memset(output, 0, sizeof(output));
        int q_len = strlen(query_string);
        if (button == 1) {
          query_string[q_len] = alphabet[char_index];
          q_len = strlen(query_string);
          char_index = 0;
        } else if (abs(angle) >= angle_threshold) {
          if (millis()-scrolling_timer >= scrolling_threshold) {
            if (angle < 0) {
              if (char_index == 0) {
                char_index = 36;
              } else {
                char_index--;
              }
            } else {
              if (char_index == 36) {
                char_index = 0;
              } else {
                char_index++;
              }
            }
            scrolling_timer = millis();
          }
        }
        sprintf(output, "%s%c", query_string, '\0');
        output[q_len] = alphabet[char_index];
        if (button == 2) {
          state = 2;
          memset(output, 0, sizeof(output));
          sprintf(output, " ");
        }
      } else if (state = 2) {
        sprintf(user, "%s", query_string);
        sprintf(output, "%s", "Username set!");
      }
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


Pong pong;
SnakeG snake;
Flappy_Joe flappy;
Joe_It joeit;
Trivia trivia;
Doodle_Jump doodle; 
Song guess;
int party_game_counter=0;
Username_input username; //Username input object
Button button(BUTTON_UP); //button object!


void pair_ESP(){
  unsigned long user_timer = millis();//Used for loop timing
  
  //Must set username before anything else
  while (strlen(user) == 0) {
    float x, y;
    get_angle(&x, &y); //reads IMU
    int bv = button.update(); //reads button input
    username.update(-y, bv, input); //Updates username input (user constructing username)
    if (strcmp(input, old_input) != 0) {//only draw if changed!
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println(input);
    }
    memset(old_input, 0, sizeof(old_input));//clears old input
    strcat(old_input, input); //sets old input to current input
//    Serial.println(input);
    while (millis() - user_timer < LOOP_PERIOD); //wait for primary timer to increment
    user_timer = millis();
  }

  unsigned long pair_timer = 0;
  //Must pair before continuing
  while (strcmp(response_buffer, "Pairing Successful!\n") != 0) {
    if (millis() - pair_timer > WAITING_PERIOD/2) {
      attempt_pairing();
      Serial.println("attempting to pair");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,2);
      tft.println("Pairing... input     username in website");
      pair_timer = millis();
    }
  }
  Serial.println("paired");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,2);
  tft.println("Pairing Successful!");
  
}

void restart(){
  if (strcmp(game_mode, "single") == 0) {
    memset(response_buffer, 0, sizeof(response_buffer));
    memset(game_mode, 0, sizeof(game_mode));
    memset(game0, 0, sizeof(game0));
    pair_ESP();
  }
  Serial.println("GAME ENDED");
  starting=true;
  drawn=false;
  
  sprintf(start, "%s", "False");
  if (strcmp(current_game, game0) == 0) {
    if (strlen(game1) > 0) {
      sprintf(current_game, "%s", game1);
    } else sprintf(start, "%s", "END");
  } else if (strcmp(current_game, game1) == 0) {
    if (strlen(game2) > 0) {
      sprintf(current_game, "%s", game2);
    } else sprintf(start, "%s", "END");
  } else if (strcmp(current_game, game2) == 0) {
    if (strlen(game3) > 0) {
      sprintf(current_game, "%s", game3);
    } else sprintf(start, "%s", "END");
  } else if (strcmp(current_game, game3) == 0) {
    if (strlen(game4) > 0) {
      sprintf(current_game, "%s", game4);
    } else sprintf(start, "%s", "END");
  }
  Serial.println(current_game);
}


void game_step(char* game) {
  Serial.print("CURRENT GAME: ");
  Serial.println(game);
  uint8_t up;
  uint8_t down;
  uint8_t left;
  uint8_t right;

      // read joystick
    int x_joy = analogRead(X_pin);
    int y_joy = analogRead(Y_pin);
    
    if (x_joy >3000) {
      up=1;
      down=1;
      left=1;
      right=0;
    } else if (x_joy < 800) {
      up=1;
      down=1;
      left=0;
      right=1;
    } else if (y_joy > 3000) {
      up=1;
      down=0;
      left=1;
      right=1;
    } else if (y_joy < 800) {
      up=0;
      down=1;
      left=1;
      right=1;
    } else {
      up=1;
      down=1;
      left=1;
      right=1;
    }

  if (starting==true) {
    if (drawn==false) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println("Move any direction to start");
      drawn=true;      
      Serial.println("START SCREEN");
    }
    if (up==0 || down==0 || left==0 || right==0 && millis()-primary_timer > 100) {
      starting=false;
      primary_timer = millis();
      tft.fillScreen(TFT_BLACK);
    }
    } else {
    if (strcmp(game, "Pong") == 0) {
      pong.step(up, down);
      if(pong.getgamestate()==false){ //REPLACE IF FIND WAY TO REPEATEDLY SELECT GAMES FROM WEBSITE
        restart();
      }
    } else if (strcmp(game, "Joe") == 0) {
      snake.step(up, down, left, right);
      if(snake.getgamestate()==false){
        restart();
      }
    } else if (strcmp(game, "Flappy-Joe") == 0) { //ADD OTHER GAMES Joe-It, Flappy Joe, Trivia
      flappy.step(up,down);
      if(flappy.getgamestate()==false){
        restart();
      }
    } else if (strcmp(game, "Joe-It") == 0) {
      joeit.step(up, down, left, right);
      if(joeit.getgamestate()==false){
        restart();
      }
    }else if (strcmp(game, "Trivia") == 0) {
      trivia.step(up, down, left, right);
      if(trivia.getgamestate()==false){
        restart();
      }
    } else if (strcmp(game, "Doodle-Joe") == 0) {
      imu.readAccelData(imu.accelCount);//read imu
      float x = -imu.accelCount[1] * imu.aRes;
      doodle.step(up, down, left, right,x);
      if(doodle.getgamestate()==false){
        restart();
      }
    }else if (strcmp(game, "Guess-Joe") == 0) {
      guess.step(up, down, left, right,x);
      if(guess.getgamestate()==false){
        guess.post_score(true);
        restart();
      }
    }
    }
}

void setup() {
  Serial.begin(115200); // Start serial monitor

  // Wifi connection
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n",WiFi.localIP()[3],WiFi.localIP()[2],
                                            WiFi.localIP()[1],WiFi.localIP()[0], 
                                          WiFi.macAddress().c_str() ,WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

  //Checks and sets up IMU
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  //Starts TFT with specified settings
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  tft.setCursor(0,0,2);

  //Sets up pin to read button inputs
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);

  
  // Set rotary encoder pins as inputs for joe-it
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  pair_ESP();
  
  randomSeed(analogRead(0));  //initialize random numbers
  Serial.println("all paired");
  
  update_timer = millis();
  sprintf(start, "%s", "False");
  
}

void loop() {
  Serial.println(current_game);
  Serial.println(game_mode);
  Serial.println(start);
  if (strcmp(game_mode, "single") != 0 && strcmp(game_mode, "party") != 0) {
    get_mode();
    update_timer = millis();
  } else if (strcmp(game0, "Pong") != 0 && strcmp(game0, "Joe") != 0 && strcmp(game0, "Flappy-Joe") != 0 && strcmp(game0, "Joe-It") != 0 && strcmp(game0, "Trivia") != 0 && strcmp(game0, "Guess-Joe") != 0 && strcmp(game0, "Doodle-Joe") != 0) { //
    if (strcmp(game_mode, "party") == 0 && strlen(roomid) < 10) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,2);
      tft.println("Getting Room ID");
      while (strlen(roomid) < 10) {
        get_roomid();
      }
      snake.setmode(true);
      pong.setmode(true);
      joeit.setmode(true);
      flappy.setmode(true);
      trivia.setmode(true);
      doodle.setmode(true);
      guess.setmode(true);
    } else {
      Serial.println(roomid);
      Serial.println("getting game");
      get_games();
      Serial.println(game0);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,2);
      tft.println("ESP is ready to \nstart");
      primary_timer = millis();
    }
  }else if (strcmp(game_mode, "single") == 0 && millis()-primary_timer>10) { //
    game_step(game0);
    primary_timer=millis();
  }else if (strcmp(game_mode, "party") == 0 && millis()-primary_timer>10){
    if (strcmp(start, "False") == 0) {
      Serial.println("getting start status");
      game_start();
      Serial.println("status got");
    } else if (strcmp(start, "True") == 0) {
      Serial.println("game running");
      Serial.println(current_game);
      game_step(current_game);
      primary_timer=millis();
    } else if (strcmp(start, "END") == 0) {
      if (drawn==false) {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,2);
        tft.println("PARTY OVER");
        drawn=true;        
      }
    }
  }
}

void get_roomid() {
  memset(response_buffer, 0, sizeof(response_buffer));
  memset(request_buffer, 0, sizeof(request_buffer));
  memset(roomid, 0, sizeof(roomid));
  int offset = 0;
  offset += sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/ESP_partyid.py?user=%s HTTP/1.1\r\n", user);
  offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  sprintf(roomid, "%s", response_buffer);
  roomid[strlen(roomid)-1] = '\0';
  update_timer = millis();
}

void game_start() {
  memset(response_buffer, 0, sizeof(response_buffer));
  memset(request_buffer, 0, sizeof(request_buffer));
  int offset = 0;
  offset += sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/ESP_game_start.py?game=%s&roomid=%s HTTP/1.1\r\n", current_game, roomid);
  offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  sprintf(start, "%s", response_buffer);
  start[strlen(start)-1] = '\0';
  Serial.println(start);
}

 
