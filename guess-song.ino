
class Song: public Game { // Song Game
    bool start=true;
    bool game_over = false;
    bool got_songs=false;
    bool gen_round = false;
    bool pressed = false;
    bool print_once = true;
    bool new_round = false;
    bool print_game_over = false;
    bool party_mode=false;

    uint32_t elapse = 0;
    
    uint8_t round_ = 0;
    uint8_t selector = 0;
    
    int right_songs[5] = {1,2,3,4,5};
    int songs_menu[5][4];

    char song_arr[38][50] = { "Airplanes",
                      "Positions",
                      "Talking to the Moon",
                      "Kiss Me More",
                      "Broccoli",
                      "Levitating",
                      "American Boy",
                      "Pumped Up Kicks",
                      "Mask Off",
                      "Somebody That I Used To Know",
                      "Watermelon Sugar",
                      "Mercy",
                      "MONTERO",
                      "Truth Hurts",
                      "Rude",
                      "Going Bad",
                      "Hayloft",
                      "Promiscuous",
                      "Drivers License",
                      "White Iverson",
                      "Black Beatles",
                      "The Less I Know the Better",
                      "Sweater Weather",
                      "SICKO MODE",
                      "See You Again",
                      "Hey, Soul Sister",
                      "Love The Way You Lie",
                      "TiK ToK",
                      "Bad Romance",
                      "Just the Way You Are",
                      "Baby",
                      "Dynamite",
                      "Secrets",
                      "Only Girl (In The World)",
                      "Marry You",
                      "Cooler Than Me",
                      "Telephone",
                      "Like A G6"};
    
  public:
 
    Song(){
      
    }

    void setmode(bool party){
      party_mode=party;
    }

    bool getgamestate(){
      return !game_over;
    }
    
    void step(uint8_t up_inp, uint8_t down_inp, uint8_t c_inp, uint8_t right=0,float x=0) {
      if(start==true){
        tft.setCursor(0, 0, 2);
        if(!got_songs){
          get_songs();
          tft.println("Fetching games...");
          if (got_songs) tft.fillScreen(TFT_BLACK);
        } else{
          if (print_once) {
            tft.println("Listen Up!");
            print_once = false;
          } else if (new_round) {
            start = false;
            new_round = false;
          } else {
            get_round();
          }
        }
      }
      else if (game_over == true && !print_game_over) { // Stays displaying game over scree
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 2);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("GAME OVER");
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("\n\nScore: ");
        tft.print(getscore());
        post_score(party_mode);
        print_game_over = true;
      } else {
        if(!gen_round){ // haven't yet made the round appear on screen
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 2);
          for(int i = 0; i < 4; i++){
            if (i == selector){ // selector set to first line
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.println(song_arr[songs_menu[round_-1][i]]);
            } else {
              tft.setTextColor(TFT_WHITE, TFT_BLACK);
              tft.println(song_arr[songs_menu[round_-1][i]]);
            }
          }
          gen_round = true;
        } else {
           display_text((up_inp !=0)? true: false, (down_inp != 0)? true: false);
           if (millis() - elapse > 45000 && !pressed){
              tft.setCursor(0, 95, 2);

              if (songs_menu[round_-1][selector] == right_songs[round_ - 1]){ //right answer
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.println("CORRECT");
                post_round(1);
                setscore(getscore()+10);
              } else { //wrong answer
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.println("INCORRECT");
                post_round(0);
              }
              pressed = true;
            }

            if (pressed){
              if(round_ == 5) game_over = true;
              else get_round(); 
            }
        }
      }
    }
    
    
  private:
    void get_songs() {
      memset(response_buffer, 0, sizeof(response_buffer));
      memset(request_buffer, 0, sizeof(request_buffer));
      int offset = 0;
      offset += sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/Party/song_select.py?roomid=%s HTTP/1.1\r\n", roomid);
      offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
      StaticJsonDocument<300> doc;
      deserializeJson(doc, response_buffer);
      if (doc.isNull()) return;
      
      for (int i = 0; i < 5; i++){
        right_songs[i] = doc["right"][i].as<int>();
        Serial.println(right_songs[i]);
      }

      for(int i = 0; i < 5; i++){ //populate possible answers per round
        int v = rand() % 4;
        for(int k = 0; k < 4; k++){
            if (k == v){
                songs_menu[i][k] = right_songs[i];
            } else {
                int u = rand() % 38;
                while (u == right_songs[i]) u = rand() % 38;
                songs_menu[i][k] = u;
            }
        }
      }
      got_songs = true;
    }
    
    void get_round() {
      memset(response_buffer, 0, sizeof(response_buffer));
      memset(request_buffer, 0, sizeof(request_buffer));
      int offset = 0;
      offset += sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team67/Party/song_select.py?roomid=%s&nextRound=yes HTTP/1.1\r\n", roomid);
      offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
      StaticJsonDocument<300> doc;
      deserializeJson(doc, response_buffer);
      Serial.print("response buffer round: ");
      Serial.println(response_buffer);
      if (doc.isNull()) return;
      
      if (doc[0].as<int>() != round_){ //round != deserialized
        round_++;
        new_round = true;
        pressed = false;
        gen_round = false;
        elapse = millis();
      }
    }

   void post_round(int correct){
   
      char body[100]; //for body
      sprintf(body,"roomid=%s&playerid=%s&correct=%d&round=%d",roomid,user,correct,round_);
        
      Serial.println("Ready to post");
      int body_len = strlen(body); //calculate body length (for header reporting)
  
      sprintf(request_buffer,"POST /sandbox/sc/team67/Party/song_select.py HTTP/1.1\r\n");
      strcat(request_buffer,"Host: 608dev-2.net\r\n");
      strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //new line
      Serial.println(request_buffer);
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
    }

    
   void display_text(bool up, bool down){
    int prev = selector;
    if(!down) selector =  (selector == 3)? 0 : selector + 1;
    else if(!up) selector = (selector == 0)? 3 : selector - 1;
    else return;

    tft.setCursor(0, 0, 2);
    for(int i = 0; i < 4; i++){
      if (i == selector){ // selector set to first line
        tft.setTextColor(TFT_PINK, TFT_BLACK);        
      } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      tft.println(song_arr[songs_menu[round_-1][i]]);
    }
   }
};
