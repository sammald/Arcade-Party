import sqlite3
import datetime
import json
import requests

rooms_db = '/var/jail/home/team67/rooms.db'
games_db  = '/var/jail/home/team67/games.db'

def request_handler(request):
    if request['method'] == 'GET': #get a list of players based on room id
        mode = request['values']['mode']
        roomid = request['values']['roomid']

        conn = sqlite3.connect(rooms_db)
        c = conn.cursor()
        
        c.execute('''CREATE TABLE IF NOT EXISTS rooms_table (roomid text, mode text, player text, timing timestamp);''')
        room = c.execute('''SELECT DISTINCT player FROM rooms_table WHERE roomid = ? AND mode = ? ORDER BY timing DESC;''',(roomid, mode,)).fetchall()
        
        conn.commit() # commit commands
        conn.close() # close connection to database
        return json.dumps([i[0] for i in room])
    else:
        mode = request['form']['mode']
        roomid = request['form']['roomid']
        player = request['form']['player']
        gen = request['form']['gen']

        selectorList = ["game1","game2","game3","game4","game5"]
        games = [request['form'].get(i) for i in ["game1","game2","game3","game4","game5"] if request['form'].get(i) is not None and request['form'].get(i) != "x"]

        conn = sqlite3.connect(games_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
        
        c.execute('''CREATE TABLE IF NOT EXISTS mult_table (roomid text, game text, timing timestamp);''')
        for g in games:
            c.execute('''INSERT into mult_table VALUES (?,?,?);''', (roomid, g, datetime.datetime.now()))
        conn.commit() # commit commands
        conn.close() # close connection to database

        ### makes GET so all nonhosts can have games ###
        if len(games)==0:
            r = requests.get(f"http://608dev-2.net/sandbox/sc/team67/ESP_games.py?user={player}&mode={mode}&roomid={roomid}")
            games = list(eval(r.text).values())[:-1]
        ################################################

        conn = sqlite3.connect(rooms_db)
        c = conn.cursor()
        
        c.execute('''CREATE TABLE IF NOT EXISTS rooms_table (roomid text, mode text, player text, timing timestamp);''')
        room = c.execute('''SELECT DISTINCT player FROM rooms_table WHERE roomid = ? AND mode = ? ORDER BY timing DESC;''',(roomid, mode,)).fetchall()
        
        if (gen == "yes" and len(room) == 0) or (gen == "no" and mode == "party" and 0 < len(room) <= 5) or (gen == "no" and mode == "duo" and 0 < len(room) <= 2): # the player is generating the room
            c.execute('''INSERT into rooms_table VALUES (?,?,?,?);''', (roomid,mode,player,datetime.datetime.now()))
        else:
            return "can't join a non-existant room :("
        
        conn.commit() # commit commands
        conn.close() # close connection to database

        out = ["<li>" + i[0] + "</li>" for i in room]
        out.append("<li>" + player + "</li>")
        list_html = " ".join(out)

        return """
                    <!DOCTYPE html>
                    <html>
                    <body>
                    <p id="code">Room Code: {code}</p>

                    
                    <h1 id="mode">{mode} Mode!</h1>
                    
                    <p>Players in the lobby:</p>

                    <ul id="plist">
                    {lobby}
                    </ul>

                    <p>(Please wait until everyone is in the lobby before hitting start)</p>

                    <form action="/sandbox/sc/team67/instructions_page.py" method="get">
                        <input type="hidden" id="mode" name="mode" value="{mode}">
                        <input type="hidden" id="roomid" name="roomid" value="{code}">
                        <input type="hidden" id="games" name="games" value="{games}">
                        <input type="hidden" id="player" name="player" value="{player}">

                        <input type="submit" value="Start">
                    </form>

                    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>
                    <script src="http://608dev-2.net/sandbox/sc/team67/roomscript.js"></script>
                    </body>

                    </html>
                    """.format(code=roomid,mode=mode.upper(),lobby=list_html,games=games,player=player)
