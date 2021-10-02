#!/usr/bin/env python3
# -*- coding: utf-8 -*-



import sqlite3
import datetime
players_db = '/var/jail/home/team67/players.db'



def request_handler(request):
    if request["method"] == "GET":
        return """
                <!DOCTYPE html>
                <html>
                <body>
                
                <h1>Welcome to Joe's Arcade Party!</h1>
                <h2>Controller Pairing</h2>
                <form action="/sandbox/sc/team67/login.py" method="post">
                  <label for="user">Enter name (Only capital letters and numbers) of controller to pair (make sure your controller is on and ready for pairing):</label>
                  <input type="text" id="user" name="user">
                  <br><br>
                  <p>After a joe-stick long press to begin inputting your username, please tilt your controller left and right to scroll through letters and numbers.</p>
                  <p>Your username must start with a letter. A short press will lock in the selected character. A long press will complete the username input of the already selected characters.</p>
                  <br>
                  <input type="submit">
                </form>
                </body>
                </html>
                """
    elif request["method"] == "POST":
        user = ""
        mode = ""
        if 'mode' in request['form']:
            mode = request['form']['mode']
        
        try:
            user = request['form']['user']
        except:
            return "Must include a user."
        
        ## for ESP to post its presence available for pairing
        if mode == "controller":
        
            conn = sqlite3.connect(players_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            
            c.execute('''CREATE TABLE IF NOT EXISTS controller_table (user text, timing timestamp);''') 
            c.execute('''INSERT into controller_table VALUES (?,?);''', (user, datetime.datetime.now()))
            
            thirty_seconds_ago = datetime.datetime.now()- datetime.timedelta(seconds = 30)
            c.execute('''CREATE TABLE IF NOT EXISTS active_players (user text, timing timestamp);''')
            paired = c.execute('''SELECT * FROM active_players WHERE user = ? AND timing > ? ORDER BY timing DESC;''',(user, thirty_seconds_ago,)).fetchone()
            
            conn.commit() # commit commands
            conn.close() # close connection to database
            
            # controller is not yet paired
            if paired is None:
                return "Pairing..."
            
            # controller has been paired to a browser and is now active
            return "Pairing Successful!"
                
        
        
        ## for webpage to pair to controller
        else:
            
            conn = sqlite3.connect(players_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS controller_table (user text, timing timestamp);''')
            thirty_seconds_ago = datetime.datetime.now()- datetime.timedelta(seconds = 30) # create time for 30 seconds ago
            available_controller = c.execute('''SELECT * FROM controller_table WHERE user = ? AND timing > ? ORDER BY timing DESC;''',(user, thirty_seconds_ago,)).fetchone()
            
            # controller not found
            if available_controller is None:
                conn.commit()
                conn.close()
                return """
                    <!DOCTYPE html>
                    <html>
                    <body>
                    
                    <h1>Welcome to Joe's Arcade Party!</h1>
                    <h2>Controller Pairing</h2>
                    <form action="/sandbox/sc/team67/login.py" method="post">
                      <label for="user">Enter name (only capital letters) of controller to pair and make sure your controller is on and ready for pairing:</label>
                      <input type="text" id="user" name="user">
                      <br>
                      <input type="submit">
                      <br><br>
                      <p>After a joe-stick long press to begin inputting your username, please tilt your controller left and right to scroll through letters.</p>
                      <p>Your username must start with a letter. A short press will lock in the selected character. A long press will complete the username input of the already selected characters.</p>
                      <br>
                      <p>Controller not found. Make sure it is turned on and connected to the internet.</p>
                    </form>
                    
                    </body>
                    </html>
                    """
            
            # controller found, add to active players
            c.execute('''CREATE TABLE IF NOT EXISTS active_players (user text, timing timestamp);''')
            c.execute('''INSERT into active_players VALUES (?,?);''', (user, datetime.datetime.now()))
            conn.commit()
            conn.close() 
            
            return """
                        <!DOCTYPE html>
                        <html>
                        <body>
                        
                        <h1>Game Mode Selection</h1>
                        <p>Paired Successfully! Now choose a game mode:</p>
                        <form action="/sandbox/sc/team67/game_mode_selection.py" method="get">
                        
                          <input type="hidden" name="user" value={user} /> 
                        
                          <input type="radio" id="single" name="game_mode" value="single">
                          <label for="single">Single Player</label><br>
                          
                          <input type="radio" id="party" name="game_mode" value="party">
                          <label for="party">PARTY!!</label>
                          
                          <br>
                          <input type="submit">
                        </form>
                        
                        
                        </body>
                        </html>
                    """.format(user=user)
        
    else:
        return 'Other not supported.'

