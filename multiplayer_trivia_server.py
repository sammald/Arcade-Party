#!/usr/bin/env python3
# -*- coding: utf-8 -*-



import sqlite3
import datetime
import requests
import json
questions_db = '/var/jail/home/team67/game_files/trivia/questions.db'


def request_handler(request):
    if request["method"] == "GET":
        # gets questions
        partyid = ""
        mode = ""
        number = 0
        try:
            partyid = request['values']['partyid']
            mode = request['values']['mode']
            number = int(request['values']['number'])
        except Exception as e:
            return "Error: party/mode is missing"
        
        if mode == "question-getter":
            r = requests.get("https://opentdb.com/api.php?amount=1&difficulty=easy&type=boolean")
            response = json.loads(r.text)
            question = response['results'][0]['question'].replace("&quot;", "'")
            question = question.replace("&#039;", "'")
            question = question.replace("\n", " ")
            questions = question + "*" + response['results'][0]['correct_answer']
            return questions
        elif mode == "question":
            conn = sqlite3.connect(questions_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            outs = ""
            c.execute('''CREATE TABLE IF NOT EXISTS questions_table (partyid text, number int, question_and_answer text, timing timestamp);''') # run a CREATE TABLE command
            question_and_answer = c.execute('''SELECT * FROM questions_table WHERE partyid = ? AND number = ? ORDER BY timing DESC;''',(partyid, number,)).fetchone()
            if question_and_answer is None:
                outs = "error*True"
            else: 
                outs = str(question_and_answer[2])
            conn.commit() # commit commands
            conn.close() # close connection to database
            return outs
    elif request["method"] == "POST":
        return "no posts here"
        
    else:
        return 'Other not supported. You need to change that.'

