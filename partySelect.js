var gameOptions = ["x","Joe", "Joe-It", "Pong", "Flappy-Joe", "Trivia", "Guess-Joe", "Doodle-Joe"];
var selectorMap = {"game1":"x", "game2":"x", "game3":"x", "game4":"x", "game5":"x"}
var selectorList = ["game1","game2","game3","game4","game5"]

function validateForm() {
    var count = 0;
    for(i = 0; i < 5; i++){
        if (selectorMap[selectList[i]] == "x"){
            count++;
        }
    }

    if (count == 5){
        alert("must select games");
        return false;
    }
}


function selectors(selectObject){
    // removes option from others once selected
    var game = selectObject.value;
    var selectorName = selectObject.name;
    console.log(game);
    if (game != "x"){
        //console.log(selectorList.indexOf(selectorName) != 0);
        //console.log(selectorMap[selectorList[selectorList.indexOf(selectorName)-1]]);
        if (selectorList.indexOf(selectorName) != 0 && selectorMap[selectorList[selectorList.indexOf(selectorName)-1]] == "x") { // have to select in order
            alert("must select games in order");
            $('[name="' + selectorName + '"] option[value="x"]').prop('selected', 'selected');
        }
        else {
            selectorList.forEach(function(g){ // make the game selected no longer an option for other selects
                if (g != selectorName){
                    $('[name="' + g + '"] option[value="' + game +'"]').hide();
                }
            });
        }
    }
    else  { // make the game available for other selects
        selectorList.forEach(function(g){
            if (g != selectorName && selectorMap[selectorName] != "x"){
                $('[name="' + g + '"] option[value="' + selectorMap[selectorName] +'"]').show();
            }
        });
    }

    selectorMap[selectorName] = game;
}


window.onload = (event) => {
    console.log('page is fully loaded');
    for(i = 1; i <= 5; i++){
        var selectList = "<p id='" + i.toString() + "'> Game Select " + i.toString()  + "</p>";
        selectList += "<select name='game" + i.toString() +"' onchange='selectors(this)'>";
        for (var x = 0; x < gameOptions.length; x++) {
            selectList += "<option value='"+ gameOptions[x] +"'>" + gameOptions[x] + "</option>";
        }
        selectList += "</select>";
        $('#gOptions').append(selectList);
    }
}



