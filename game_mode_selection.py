#!/usr/bin/env python3
# -*- coding: utf-8 -*-



import sqlite3
import datetime
mode_db = '/var/jail/home/team67/mode.db'



def request_handler(request):
    if request["method"] == "GET":
        
        user = ""
        game_mode = ""
        
        try:
            user = request['values']['user']
            game_mode = request['values']['game_mode']
        except Exception as e:
            return "Error: user or game_mode is missing"

        conn = sqlite3.connect(mode_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
        
        c.execute('''CREATE TABLE IF NOT EXISTS mode_table (user text, mode text, timing timestamp);''') 
        c.execute('''INSERT into mode_table VALUES (?,?,?);''', (user, game_mode, datetime.datetime.now()))
        conn.commit() # commit commands
        conn.close() # close connection to database

        if game_mode == "single":
            return """
                    <!DOCTYPE html>
                    <html>
                    <body>
                    <p>Welcome {user}!</p>
                    
                    <h1>Single Player Mode!</h1>
                    
                    <p>Select a game...</p>
                    
                    <form action="/sandbox/sc/team67/game_room.py" method="get">
                        <input type="hidden" name="player" value={user} />
                        <input type="hidden" name="mode" value=single />

                        <input type="radio" id="game1" name="game" value="Joe">
                        <label for="game1"> Joe </label><br>

                        <input type="radio" id="game2" name="game" value="Pong">
                        <label for="game2"> Pong</label><br>

                        <input type="radio" id="game3" name="game" value="Flappy-Joe">
                        <label for="game3"> Flappy-Joe</label><br>

                        <input type="radio" id="game4" name="game" value="Joe-It">
                        <label for="game4"> Joe-It </label><br>

                        <input type="radio" id="game6" name="game" value="Guess-Joe">
                        <label for="game6"> Guess-Joe</label><br>

                        <input type="radio" id="game7" name="game" value="Doodle-Joe">
                        <label for="game7"> Doodle-Joe</label><br><br>

                        <input type="submit" value="Submit">
                    </form>
                    
                    </body>
                    </html>
                    """.format(user=user)
        
        elif game_mode == "party":
            return """
                    <!DOCTYPE html>
                    <html>
                    <body>
                    <p>Welcome {user}!</p>
                    
                    <h1>PARTY Mode!</h1>
                    
                    <p>Enter a party to create or join...</p>
                    
                    <form action="/sandbox/sc/team67/rooms.py" method="post">
                        <input type="hidden" id="mode" name="mode" value="party">
                        <input type="hidden" name="player" value={user} /> 
                        <input type="hidden" id="gen" name="gen" value="no">

                        <label for="roomid">Enter a room code</label>
                        <input type="text" id="roomid" name="roomid"><br><br>
                        <input type="submit" value="Submit">
                        
                    </form>

                    <form action="/sandbox/sc/team67/game_room.py" method="get">
                        <input type="hidden" name="player" value={user} /> 
                        <input type="submit" value="Generate a Room">
                    </form>
                    
                    </body>
                    </html>
                    """.format(user=user)
        
        else:
            return "Error: game_mode invalid"
        
        
    elif request["method"] == "POST":
        return 'POST not supported.'
        
    else:
        return 'Other not supported.'
