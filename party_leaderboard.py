import sqlite3
party_leaderboard = "/var/jail/home/team67/Party/party_leaderboard.db"
conn = sqlite3.connect(party_leaderboard)
c = conn.cursor()
leaderboard_html= """<!DOCTYPE html>
<html>
<body style="background-color:powderblue;text-align:center;">

<h1 style="font-size:450%;">Leaderboard for {Room_id}</h1>
<form action="http://608dev-2.net/sandbox/sc/team67/Party/party_leaderboard.py" method="get">
  <div id="game">
      <div class ="row">
          <div class = "name" style="font-size:400%;"><b>Top three players</b></div>
      </div>
      <div class ="row">
          <div class = "name" style = "color:red;font-size:300;">{Player1}</div>
      </div>
       <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player2}</div>
      </div>
       <div class ="row">
          <div class = "name" style="color:grey;font-size:300;">{Player3}</div>
      </div>
   

</body>
</html>"""

def request_handler (request): 
    if request['method'] == 'GET':
        players = c.execute('''SELECT DISTINCT player,score FROM party_leaderboard WHERE room_id = ? ORDER BY timing DESC, score ASC;''',(request['values']['room_id'],)).fetchall()
        conn.commit()
        conn.close()
        players_seen = []
        new_players = []
        for i in range(len(players)):
            if players[i][0] not in players_seen: 
                new_players.append(players[i])
                players_seen.append(players[i][0])
        new_players = new_players[::-1]
        if len(new_players) == 0:
            return "That's all folks. Party is officially over..."
        if len(new_players) == 1:
            return "Go play single player mode you weirdo."
        elif len(new_players) == 2:
            return leaderboard_html.format(Room_id = request['values']['room_id'], Player1 = new_players[0][0], Player2 = new_players[1][0], Player3 = new_players[1][0])
        return leaderboard_html.format(Room_id = request['values']['room_id'],Player1 = new_players[0][0], Player2 = new_players[1][0], Player3 = new_players[2][0])
        

