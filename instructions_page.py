import json
import ast
import sqlite3
import datetime
import requests

games_db  = '/var/jail/home/team67/games.db'

html = """
<!DOCTYPE html>
<html>
<head>
<link rel="stylesheet" href="style.css">
</head>

<body>
<p id="welcome">Welcome to {game} !</p>

<p id="code">Room Code: {code}</p>
<h1>Instructions:</h1>

<p>{text}</p>

<ul id="plist">
</ul>

<div id="guess">
    <p>round 1</p>
    <div id="media">
        <audio id="myAudio" type="audio/mpeg" controls controlsList="nodownload">
            <source id="audioSource" src="http://608dev-2.net/sandbox/sc/team67/music/Kanye%20West%20-%20Mercy.mp3"></source>
            Your browser does not support the audio format.
        </audio>
    </div>

    <ol type="A" id="options">
    </ol>
</div>

<form action="/sandbox/sc/team67/instructions_page.py" method="get" id="scoreboard">
    <input type="hidden" id="mode" name="mode" value="{mode}">
    <input type="hidden" id="roomid" name="roomid" value="{code}">
    <input type="hidden" id="games" name="games" value="{games}">
    <input type="hidden" id="player" name="player" value="{player}">

    <input type="submit" value="Next Game">
</form>

<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>
<script src="http://608dev-2.net/sandbox/sc/team67/transition.js"></script>
</body>

</html>
"""



def request_handler(request):
    if request['method'] == 'GET': 
        mode = request['values']['mode']
        roomid = request['values']['roomid']
        user = request['values']['player']
        games = ast.literal_eval(str(request['values']['games']))
        if len(games) == 0:
            r = requests.get(f"http://608dev-2.net/sandbox/sc/team67/Party/party_leaderboard.py?room_id={roomid}")
            return r.text
        game = games.pop(0)

        ### db to know when game is started ###
        conn = sqlite3.connect(games_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
            
        c.execute('''CREATE TABLE IF NOT EXISTS game_start (roomid text, game text, timing timestamp);''') 
        c.execute('''INSERT into game_start VALUES (?,?,?);''', (roomid, game, datetime.datetime.now()))
        conn.commit() # commit commands
        conn.close() # close connection to database
        ########################################

        instr = ""
        
        if game == "Joe":
            instr = "Use the Joe-stick to move around the screen. Your goal is to eat food and grow longer. You lose if you eat yourself or leave the field of play (LCD screen boundaries)"
        elif game == "Joe-It":
            instr = "Follow the commands as they appear on your LCD (Twist, Shake, Yell, Up, Down, Left, Right)."
        elif game == "Pong":
            instr = "Use the Joe-stick to move your paddle about the screen. Everytime the ball makes contact with your paddle you get a point. If you miss the ball you lose a life. Good Luck"
        elif game == "Flappy-Joe":
            instr = "Use the Joe-stick to move the circle to avoid the obstacles. Every obstacle you pass is a point. You lose by running into an obstacle or leaving the field of play"
        elif game == "Trivia":
            instr = "Move left to answer TRUE and right to answer FALSE. You are timed so get it right as fast as possible."
        elif game == "Guess-Joe":
            instr = "A song will play from your computer, input the correct answer on your controlled to score points"
        elif game == "Doodle-Joe":
            instr = "Tilt ESP to go left or right and jump on the platforms. Try to get as high as possible without falling."
        else:
            return "not a valid game"

        return html.format(game=game,text=instr,games=games,code=roomid,mode=mode,player=user)
    if request['method'] == 'POST':
        pass
    return "can only do get requests"
