function getArgs(){
    var rm = $("#code").text().split(" ")[2];
    var gm = $("#welcome").text().split(" ")[2];
    console.log({room_id : rm, game : gm});
    //console.log($("#code").text().split(" "));
    return {room_id : rm, game : gm}
}
function getArgsLeaderboard(){
    var rm = $("#code").text().split(" ")[2];
    var gm = $("#welcome").text().split(" ")[2];
    console.log({room_id : rm, game : gm});
    //console.log($("#code").text().split(" "));
    return {room_id : rm, game : gm, leaderboard: ""}
}

function getArgsNames(){
    var rm = $("#code").text().split(" ")[2];
    var md = $("#mode").val().toLowerCase();
    console.log({roomid : rm, mode : md});
    //console.log($("#code").text().split(" "));
    return {roomid : rm, mode : md}
}

var playerNames = [];

function fetchNames(){
    $.ajax({
     url: 'http://608dev-2.net/sandbox/sc/team67/rooms.py',
     type: 'get',
     data: getArgsNames(),
     success: function(data){
      // Perform operation on return value
      console.log(data);
      var names = JSON.parse(data);
      playerNames = names;
     } 
    });
}

function startGame(){
    /*$.ajax({
     url: 'http://608dev-2.net/sandbox/sc/team67/rooms.py',
     type: 'get',
     data: getArgsNames(),
     success: function(data){
      // Perform operation on return value
      console.log(data);
      var names = JSON.parse(data);
      playerNames = names;
     } 
    });*/
    alert("starting game!");
}
function fetchLeaderboard(){
    $.ajax({
        url: 'http://608dev-2.net/sandbox/sc/team67/Party/party_databases.py',
        type: 'get',
        data: getArgsLeaderboard(),
        success: function(data){
            console.log(data);
        }

    });
};
function fetchdata(){
    $.ajax({
        url: 'http://608dev-2.net/sandbox/sc/team67/Party/party_databases.py',
        type: 'get',
        data: getArgs(),
        success: function(data){
            // Perform operation on return value
            console.log(data);
            //console.log(typeof data);

            var scores = JSON.parse(data.replace(/'/g,'"'));
            if (scores.length == playerNames.length){
                fetchLeaderboard();
                $('#scoreboard').show();
                $('#gInstr').show();
                scores.forEach(function(g){ // make the game selected no longer an option for other selects
                    $("ul").append('<li>' + g[0] + ': ' + g[1].toString() + '</li>');
                });
                $("#start").hide();
            }
        },
        complete:function(data){
            if ($('li').length != playerNames.length){
                setTimeout(fetchdata,5000);
            }
        }
    });
};



// everyone has 30 seconds
// 15 seconds after the page loads the song starts automatically
// have to guess within the 30 seconds
// next song in 10 seconds
var vid = document.getElementById("myAudio");

var round = 1;
var playStartTime = 0;
var playersPosted = 0;
var lastCallNextVid = 0;
var results = {};
var correctSongs = [];
var playerInfo = [];
var gotAns = false;
var songs = ["Airplanes",
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
            "See You Again"]

vid.ontimeupdate = function() {vidStop()};
function vidStop(){
    //console.log('playing song');
    if (vid.currentTime >= 30){
        vid.pause();
        fetchSongScores(); 
    }
}

function nextVid(){
        vid.currentTime = 0;
        //round++;
        if (round >= 6){
            $("#guess p").text("Game Over!");
        } else {
            //while( Math.floor((Date.now()-lastCallNextVid) / 1000) < 25) console.log('waiting');
            $("#guess p").text("round " + round.toString());
            console.log(round);
            //var abba = songs[correctSongs[round-1]];
            //console.log(abba);
            $("#audioSource").attr("src", "http://608dev-2.net/sandbox/sc/team67/music/" + songs[correctSongs[round-1]].replace(/\s/g, '%20') + ".mp3"); // set the new song to play
            vid.load();
            fetchNextRound();
            gotAns = false;
            setTimeout(playSong(),5000); //wait 5 seconds for new song
        }
    
}

function playSong(){
    vid.currentTime = 0;
    playStartTime = Date.now();
    vid.oncanplay = function() {
        console.log('can play');
    };
    vid.play();
}

function fetchSongScores(){
    $.ajax({
        url: 'http://608dev-2.net/sandbox/sc/team67/Party/song_select.py', // players post to db their gameid, playeid, round, bool correct, and time
        type: 'get',
        data: {roomid:$("#code").text().split(" ")[2], "fetch":round}, //pass in roomid
        success: function(data){
            // Perform operation on return value
            console.log(data); // should be the list of list of [[player id, bool correct, time], ....]
            //console.log(typeof data);

            playerInfo = JSON.parse(data);
            
            if(playerInfo.length == playerNames.length){
                //if don't have all the player info then just skip
                gotAns = true;
                round++;
                /*for(var i = 0; i < playerInfo.length; i++){
                    if (!(playerInfo[i][0] in results)){
                        results[playerInfo[i][0]] = 0;
                    } else {
                        if (playerInfo[i][1] == 1){ // update when correct
                            var points = Date.parse(playerInfo[i][2]) - playStartTime;
                            results[playerInfo[i][0]] += (points < 0)? 30 : points;
                        }
                    }
                }*/
            }
        },
        complete:function(data){
            if (!gotAns){ //  && Math.abs((Date.now() -  playStartTime)/1000) < 60
                console.log('still fetching scores');
                setTimeout(fetchSongScores,5000);
            } else {
                //gotAns = false;
                lastCallNextVid  = Date.now();
                nextVid();
            }
        }
    });
};


function fetchNextRound(){ // to start/transition rounds should post to db, the esp will continually fetch
    $.ajax({
        url: 'http://608dev-2.net/sandbox/sc/team67/Party/song_select.py', // post gameid, round
        type: 'post',
        data: {roomid:$("#code").text().split(" ")[2], "nextRound":round} //pass in roomid
    });
};


function fetchCorrectSongs(){
    $.ajax({
        url: 'http://608dev-2.net/sandbox/sc/team67/Party/song_select.py',
        type: 'get',
        data: {roomid:$("#code").text().split(" ")[2]}, //pass in roomid
        success: function(data){
            console.log(data); // should be the list of list of [[player id, int, time], ....]
            correctSongs = JSON.parse(data).right;
            $("#audioSource").attr("src", "http://608dev-2.net/sandbox/sc/team67/music/" + songs[correctSongs[round-1]].replace(/\s/g, '%20') + ".mp3"); // set the new song to play
            console.log("http://608dev-2.net/sandbox/sc/team67/music/" + songs[correctSongs[round-1]].replace(/\s/g, '%20') + ".mp3")
            vid.load();
            //vid.play();
        }
    });
};

function postGuessStart(){
    $.ajax({
        url: 'http://608dev-2.net/sandbox/sc/team67/Party/song_select.py',
        type: 'post',
        data: {roomid:$("#code").text().split(" ")[2]},
        complete:function (data) { fetchCorrectSongs(); }
    });
};

$(document).ready(function(){
    $('#scoreboard').hide();
    fetchNames();
    if ($("#welcome").text().split(" ")[2] == "Guess-Joe"){
        $('#myAudio').click(false);
        $('#guess').css("display", "initial");
        postGuessStart();
        //fetchCorrectSongs();
        fetchNextRound();
        setTimeout(playSong, 10000);
    }
    setTimeout(fetchdata,5000);
});
