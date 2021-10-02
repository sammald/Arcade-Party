import sqlite3
import datetime
game_scores = "/var/jail/home/team67/Leaderboard/game_scores.db"
conn = sqlite3.connect(game_scores)
c = conn.cursor()
show_top = 10
leaderboard_html= """<!DOCTYPE html>
<html>
<body style="background-color:powderblue;text-align:center;">

<h1 style="font-size:450%;">{Game}</h1>
<form action="http://608dev-2.net/sandbox/sc/team67/Leaderboard/leaderboard.py" method="get">
  <div id="game">
      <div class ="row">
          <div class = "name" style="font-size:400%;"><b>Player: Score</b></div>
      </div>
      <div class ="row">
          <div class = "name" style = "color:red;font-size:300;">{Player1}: {Score1}</div>
      </div>
       <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player2}: {Score2}</div>
      </div>
       <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player3}: {Score3}</div>
      </div>
       <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player4}: {Score4}</div>
      </div>
      <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player5}: {Score5}</div>
      </div>
      <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player6}: {Score6}</div>
      </div>
      <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player7}: {Score7}</div>
      </div>
      <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player8}: {Score8}</div>
      </div>
      <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player9}: {Score9}</div>
      </div>
      <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player10}: {Score10}</div>
      </div>

</body>
</html>"""

def request_handler(request):
    if request['method'] == 'POST':
        c.execute('''CREATE TABLE IF NOT EXISTS game_scores(player text,game text,score int, time date_time);''')
        try:
            score = int(request['form']['score'])
        except ValueError:
            print("Score is not a valid number")
            raise
        c.execute('''INSERT into game_scores VALUES (?,?,?,?);''', (request['form']['player'], request['form']['game'], score, datetime.datetime.now()))
        conn.commit()
        conn.close()
    elif request['method'] == 'GET':
        game = request['values']['game']
        player_score = c.execute('''SELECT player,score FROM game_scores WHERE game = ? ORDER BY score DESC LIMIT ?;''',(game,show_top)).fetchall()
        if len(player_score) < show_top:
            return str(show_top - len(player_score)) + " player(s) need to play before leaderboard will be visiable"
        current_leaders = leaderboard_html.format(Game = game, Player1 = player_score[0][0], Score1 = player_score[0][1],\
                                                  Player2 = player_score[1][0],Score2 = player_score[1][1],\
                                                  Player3 = player_score[2][0], Score3 = player_score[2][1],\
                                                  Player4 = player_score[3][0], Score4 = player_score[3][1],\
                                                  Player5 = player_score[4][0], Score5 = player_score[4][1],\
                                                  Player6 = player_score[5][0], Score6 = player_score[5][1],\
                                                  Player7 = player_score[6][0], Score7 = player_score[6][1],\
                                                  Player8 = player_score[7][0], Score8 = player_score[7][1],\
                                                  Player9 = player_score[8][0], Score9 = player_score[8][1],\
                                                  Player10 = player_score[9][0], Score10 = player_score[9][1])
        return current_leaders
