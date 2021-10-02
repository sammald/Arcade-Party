/*function getRequest() {
    let xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://608dev-2.net/sandbox/sc/team67/rooms.py');
    xhr.responseType = 'text';
    xhr.send();
    console.log(xhr.response);
    document.getElementById("txt").innerHTML = xhr.response;
    console.log('requested');
};

window.onload = (event) => {
    console.log('page is fully loaded');
    setTimeout(getRequest(), 5000);
};*/

function getArgs(){
    var rm = $("#code").text().split(" ")[2];
    var md = $("#mode").text().split(" ")[0].toLowerCase();
    console.log({roomid : rm, mode : md});
    //console.log($("#code").text().split(" "));
    return {roomid : rm, mode : md}
}

function fetchdata(){
 $.ajax({
  url: 'http://608dev-2.net/sandbox/sc/team67/rooms.py',
  type: 'get',
  data: getArgs(),
  success: function(data){
   // Perform operation on return value
   console.log(data);
   var names = JSON.parse(data);
   var linames = $.map( $('li'), function (element) { return $(element).text() });
   for(i = 0; i < names.length; i++){
    if (!linames.includes(names[i])){
        $("ul").append('<li>' + names[i] + '</li>');
    }
   };
  },
  complete:function(data){
   setTimeout(fetchdata,5000);
  }
 });
}

$(document).ready(function(){
 setTimeout(fetchdata,5000);
}); 
