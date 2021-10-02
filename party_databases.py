import sqlite3
import datetime
party_scores = "/var/jail/home/team67/Party/party_scores.db"
party_leaderboard = "/var/jail/home/team67/Party/party_leaderboard.db"
conn = sqlite3.connect(party_scores)
conn2 = sqlite3.connect(party_leaderboard)
c = conn.cursor()
c2 = conn2.cursor()

def request_handler(request):
    if request['method'] == 'POST':
        c.execute('''CREATE TABLE IF NOT EXISTS party_scores(room_id text, player text,game text, score int, timing datetime);''')
        try:
            score = int(request['form']['score'])
        except ValueError:
            print("Score is not a valid number")
            raise
        c.execute('''INSERT into party_scores VALUES (?,?,?,?,?);''', (request['form']['room_id'], request['form']['player'], request['form']['game'], score, datetime.datetime.now()))
        conn.commit()
        conn.close()
    elif request['method'] == 'GET':
        if 'leaderboard' not in request['values']:
            players = c.execute('''SELECT DISTINCT player,score FROM party_scores WHERE room_id = ? and game = ? ORDER BY score DESC;''', (request['values']['room_id'], request['values']['game'])).fetchall()
            conn.commit()
            conn.close()
            return [list(i) for i in players]
        else: 
            players = c.execute('''SELECT DISTINCT player,score FROM party_scores WHERE room_id = ? and game = ? ORDER BY score DESC;''', (request['values']['room_id'], request['values']['game'])).fetchall()
            c2.execute('''CREATE TABLE IF NOT EXISTS party_leaderboard (room_id text, player text, score int, timing datetime);''')
            leaderboard = c2.execute('''SELECT DISTINCT player, score FROM party_leaderboard WHERE room_id = ? ORDER BY timing DESC, score ASC;''', (request['values']['room_id'],)).fetchall() 
            if leaderboard == []: 
                for i in range(len(players)): 
                    c2.execute('''INSERT into party_leaderboard VALUES (?,?,?,?);''',(request['values']['room_id'], players[i][0],i, datetime.datetime.now()))
            else: 
                for i in range(len(leaderboard)):
                    player_score = c2.execute('''SELECT score FROM party_leaderboard WHERE room_id = ? and player = ? ORDER BY timing DESC;''',(request['values']['room_id'], leaderboard[i][0])).fetchone()
                    current_score = player_score[0] + i
                    c2.execute('''INSERT into party_leaderboard VALUES (?,?,?,?);''', (request['values']['room_id'], players[i][0], current_score, datetime.datetime.now()))
                    
            players = c2.execute('''SELECT DISTINCT player,score FROM party_leaderboard WHERE room_id = ? ORDER BY timing DESC;''',(request['values']['room_id'],)).fetchall()
            conn2.commit()
            conn2.close()
            players_seen = []
            new_players = []
            for i in range(len(players)):
                if players[i][0] not in players_seen: 
                    new_players.append(players[i])
                    players_seen.append(players[i][0])
            new_players = new_players[::-1]
            return [list(i) for i in new_players]
             