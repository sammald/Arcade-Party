#!/usr/bin/env python3
# -*- coding: utf-8 -*-



import sqlite3
import datetime
import random
import requests
import json

questions_db = '/var/jail/home/team67/game_files/trivia/questions.db'
players_db = '/var/jail/home/team67/players.db'
games_db  = '/var/jail/home/team67/games.db'
game_starting_html  = """
<!DOCTYPE html>
<html>
<body>
<p> You are playing {game} </p>
<p>Instructions = {instruct}</p>

<form action="/sandbox/sc/team67/login.py" method="get">
<input type="submit" value="Back to Login">
</form>
                    
</body>
</html>
"""
game_inst = {"Joe": "Use the Joe-stick to move around the screen. Your goal is to eat food and grow longer. You lose if you eat yourself or leave the field of play (LCD screen boundaries)",
             "Joe-It": "Follow the commands as they appear on your LCD (Twist, Shake, Yell, Up, Down, Left, Right).", 
             "Pong":"Use the Joe-stick to move your paddle about the screen. Everytime the ball makes contact with your paddle you get a point. If you miss the ball you lose a life. Good Luck", 
             "Flappy-Joe": "Use the Joe-stick to move the circle to avoid the obstacles. Every obstacle you pass is a point. You lose by running into an obstacle or leaving the field of play",
             "Trivia": "Move left to answer TRUE and right to answer FALSE. You are timed so get it right as fast as possible.",
             "Guess-Joe": "A song will play from your computer, input the correct answer on your controlled to score points",
             "Doodle-Joe": "Tilt ESP to go left or right and jump on the platforms. Try to get as high as possible without falling."}

def gen_room_id():
    word = [["red", "orange", "yellow", "green", "blue", "purple"],
            ["apple", "bean", "carrot", "donut", "eclair", "flan"],
            ["baldwin", "pope", "walker", "wells", "sinclair", "clancy"]]
    return word[random.randint(0, 2)][random.randint(0, 5)] + "-" + word[random.randint(0, 2)][random.randint(0, 5)] + "-" + word[random.randint(0, 2)][random.randint(0, 5)] + "-" + str(random.randint(0, 1000))


def request_handler(request):
    if request['method'] == 'GET':
        #return str(request)
        if 'game' in request['values']:
            user = request['values']['player']
            game = request['values']['game']
            conn = sqlite3.connect(games_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            
            c.execute('''CREATE TABLE IF NOT EXISTS game_table (user text, game text, timing timestamp);''') 
            c.execute('''INSERT into game_table VALUES (?,?,?);''', (user, game, datetime.datetime.now()))
            conn.commit() # commit commands
            conn.close() # close connection to database

        if request['values'].get("mode") is not None: 
            return game_starting_html.format(game = game, instruct = game_inst[game])

        code = gen_room_id()
        player = request['values']['player']
        
        ## generate trivia questions for this party
        for i in range(10):
            r = requests.get("https://opentdb.com/api.php?amount=1&difficulty=easy&type=boolean")
            response = json.loads(r.text)
            question = response['results'][0]['question'].replace("&quot;", "'")
            question = question.replace("&#039;", "'")
            question = question.replace("\n", " ")
            question = question + "*" + response['results'][0]['correct_answer']
            
            conn = sqlite3.connect(questions_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS questions_table (partyid text, number int, question_and_answer text, timing timestamp);''') # run a CREATE TABLE command
            c.execute('''INSERT into questions_table VALUES (?,?,?,?);''', (code, i+1, question, datetime.datetime.now()))
            conn.commit() # commit commands
            conn.close() # close connection to database
            
        return """
                    <!DOCTYPE html>
                    <html>
                    <body>
                    <p>Room Code: {code}</p>
                    
                    <!--
                    <h1>PARTY Mode!</h1>
                    --->

                    <p>Select the Games</p>

                    <form action="/sandbox/sc/team67/rooms.py" method="post" onsubmit="return validateForm(event)">
                        <input type="hidden" id="mode" name="mode" value="party">
                        <input type="hidden" id="roomid" name="roomid" value="{code}">
                        <input type="hidden" id="player" name="player" value="{player}">
                        <input type="hidden" id="gen" name="gen" value="yes">

                        <div id="gOptions"></div>

                        <input type="submit" value="Submit">
                    </form>
                    
                    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>
                    <script src="http://608dev-2.net/sandbox/sc/team67/partySelect.js"></script>

                    </body>
                    </html>
                    """.format(code=code,player=player)
