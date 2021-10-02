import sqlite3
import datetime
import time
import random
import json
import ast
songs_db = "/var/jail/home/team67/Party/guess_songs.db"
songs_scores_db = "/var/jail/home/team67/Party/songs_scores.db"
song_round_db = "/var/jail/home/team67/Party/song_rounds.db"

song_ops = ["Airplanes",
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
                      "Like A G6",
                      "OMG",
                      "Alejandro",
                      "Your Love Is My Drug",
                      "Meet Me Halfway",
                      "Whataya Want from Me",
                      "Take It Off",
                      "Misery",
                      "All The Right Moves",
                      "Animal",
                      "Naturally",
                      "I Like It",
                      "Teenage Dream",
                      "California Gurls",
                      "My First Kiss",
                      "Blah Blah Blah",
                      "Imma Be",
                      "Try Sleeping with a Broken Heart",
                      "If I Had You",
                      "Rock That Body",
                      "Dog Days Are Over",
                      "Not Myself Tonight",
                      "DJ Got Us Fallin In Love",
                      "Castle Walls",
                      "Break Your Heart",
                      "Hello",
                      "A Thousand Years",
                      "Someone Like You",
                      "Give Me Everything",
                      "Just the Way You Are",
                      "Rolling in the Deep",
                      "Run the World (Girls)",
                      "Moves Like Jagger",
                      "Love On Top",
                      "Grenade",
                      "Tonight Tonight",
                      "What the Hell",
                      "Born This Way",
                      "Monster",
                      "Cheers (Drink To That)",
                      "S&M Remix",
                      "Written in the Stars",
                      "Jar of Hearts",
                      "Super Bass",
                      "Raise Your Glass",
                      "Invading My Mind",
                      "Moment 4 Life",
                      "Last Friday Night (T.G.I.F.)",
                      "Firework",
                      "Titanium",
                      "Locked Out of Heaven",
                      "Paradise",
                      "Payphone",
                      "What Makes You Beautiful",
                      "I Knew You Were Trouble",
                      "Call Me Maybe",
                      "Love You Like A Love Song",
                      "Set Fire to the Rain",
                      "We Are Never Ever Getting Back Together",
                      "Stronger",
                      "Starships",
                      "One More Night",
                      "Good Time",
                      "Glad You Came",
                      "Beauty And A Beat",
                      "International Love",
                      "Some Nights",
                      "Boyfriend",
                      "Part Of Me",
                      "Domino",
                      "Where Have You Been",
                      "Wide Awake",
                      "The One That Got Away",
                      "Dance Again",
                      "Turn Up the Music",
                      "We Are Young",
                      "Diamonds",
                      "Stereo Hearts",
                      "It Will Rain",
                      "Blow Me (One Last Kiss)",
                      "Underneath the Tree",
                      "Wake Me Up",
                      "Story of My Life",
                      "Just Give Me a Reason",
                      "Hall of Fame",
                      "Roar",
                      "We Cant Stop",
                      "Dont You Worry Child",
                      "Get Lucky",
                      "Wrecking Ball",
                      "Impossible",
                      "Blurred Lines",
                      "Heart Attack",
                      "Die Young",
                      "Clarity",
                      "Summertime Sadness",
                      "Everybody Talks",
                      "Hold On, Were Going Home",
                      "Best Song Ever",
                      "Kiss You",
                      "Sweet Nothing",
                      "Lose Yourself to Dance",
                      "Work Bitch",
                      "Cant Hold Us",
                      "Feel This Moment",
                      "Beneath Your Beautiful",
                      "Let Me Love You",
                      "Thrift Shop",
                      "Suit & Tie",
                      "I Love It",
                      "Daylight",
                      "Love Somebody",
                      "Walks Like Rihanna",
                      "Heartbreaker",
                      "Mirrors",
                      "Next To Me",
                      "Made In The USA",
                      "Girl On Fire",
                      "TKO",
                      "Come & Get It",
                      "Live It Up",
                      "We Own The Night",
                      "Applause",
                      "Anything Could Happen",
                      "Finally Found You",
                      "Pom Poms",
                      "Beautiful",
                      "Crazy Kids",
                      "People Like Us",
                      "Overdose",
                      "Give It 2 U",
                      "Foolish Games",
                      "Freak",
                      "All of Me",
                      "Stay With Me",
                      "Summer",
                      "Happy",
                      "Rude",
                      "Shake It Off",
                      "Dark Horse",
                      "Hey Brother",
                      "Maps",
                      "Treasure",
                      "Let Her Go",
                      "Problem",
                      "Pompeii",
                      "Team",
                      "Love Me Again",
                      "Latch"]

def request_handler(request):
    if request['method'] == 'POST':
        roomid = request['form']['roomid']
        if ('playerid' in request['form']):
            playerid = request['form']['playerid']
            correct = int(request['form']['correct'])
            round_ = int(request['form']['round'])
            #epoch = int(round(time.time() * 1000))

            conn = sqlite3.connect(songs_scores_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS song_score(room_id text, playerid text, correct int, round int, timing datetime);''')
            c.execute('''INSERT into song_score VALUES (?,?,?,?,?);''', (roomid, playerid, correct, round_, datetime.datetime.now()))
            conn.commit() # commit commands
            conn.close() # close connection to database
            #return epoch
        elif "nextRound" in request['form']:
            conn = sqlite3.connect(song_round_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS roundss(room_id text, round int, timing datetime);''')
            c.execute('''INSERT into roundss VALUES (?,?,?);''', (roomid, int(request['form']['nextRound']), datetime.datetime.now()))
            conn.commit() # commit commands
            conn.close() # close connection to database
        else:
            conn = sqlite3.connect(songs_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS songs(room_id text, s1 int, s2 int,s3 int,s4 int,s5 int,timing datetime);''')
            c.execute('''INSERT into songs VALUES (?,?,?,?,?,?,?);''', (roomid, random.randint(0, 4), random.randint(5, 9), random.randint(10, 14), random.randint(15, 19), random.randint(20, 24), datetime.datetime.now()))
            conn.commit() # commit commands
            conn.close() # close connection to database
            return
    if request['method'] == 'GET':
        roomid = request['values']['roomid']
        if ('fetch' in request['values']):
            conn = sqlite3.connect(songs_scores_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS song_score(room_id text, playerid text, correct int, round int, timing datetime);''')
            one_minute_ago = datetime.datetime.now()- datetime.timedelta(seconds = 30)
            answers = c.execute('''SELECT playerid, correct FROM song_score WHERE room_id = ? AND round = ? AND timing > ? ORDER BY timing DESC;''',(roomid, request['values']['fetch'], one_minute_ago)).fetchall()
            conn.commit() # commit commands
            conn.close() # close connection to database
            return json.dumps(answers)
        elif "nextRound" in request['values']:
            conn = sqlite3.connect(song_round_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS roundss(room_id text, round int, timing datetime);''')
            one_minute_ago = datetime.datetime.now()- datetime.timedelta(seconds = 60)
            curr_round = c.execute('''SELECT round FROM roundss WHERE room_id = ? AND timing > ? ORDER BY timing DESC;''',(roomid, one_minute_ago)).fetchone()
            conn.commit() # commit commands
            conn.close() # close connection to database
            return json.dumps(curr_round)
        elif "select" in request['values']:
            songs_arr = ast.literal_eval(request['values']['select'])
            return json.dumps([[song_ops[j] for j in i] for i in songs_arr])
        else:
            conn = sqlite3.connect(songs_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            c.execute('''CREATE TABLE IF NOT EXISTS songs(room_id text, s1 int, s2 int,s3 int,s4 int,s5 int,timing datetime);''')
            one_minute_ago = datetime.datetime.now()- datetime.timedelta(seconds = 3600)
            songs = c.execute('''SELECT s1,s2,s3,s4,s5 FROM songs WHERE room_id = ? AND timing > ? ORDER BY timing DESC;''',(roomid, one_minute_ago)).fetchone()
            wrong = [random.randint(0,154) for i in range(15)]
            conn.commit() # commit commands
            conn.close() # close connection to database

        return json.dumps({"right":songs,"wrong":wrong})
