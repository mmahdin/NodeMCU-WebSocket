
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButton();
}

function getReadings(){
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        
        document.getElementById(key).innerHTML = myObj[key];
        if (key == "state"){
            if (myObj[key] == "1")
            {
                state = "OFF";
            }
            else{
                state = "ON";
            }
            document.getElementById('state').innerHTML = state;
        }   
        else if (key == "temperature" && myObj[key] != "null"){
            var x = (new Date()).getTime();
            var y = parseFloat(myObj[key]);
            if (chartT.series[0].data.length > 40) {
                chartT.series[0].addPoint([x, y], true, true, true);
              } else {
                chartT.series[0].addPoint([x, y], true, false, true);
              }
        }
        else if (key == "humidity" && myObj[key] != "null"){
            var x = (new Date()).getTime();
            var y = parseFloat(myObj[key]);
            if (chartH.series[0].data.length > 40) {
                chartH.series[0].addPoint([x, y], true, true, true);
              } else {
                chartH.series[0].addPoint([x, y], true, false, true);
              }
        }
    
    }
}

function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
}

function toggle(){
    websocket.send('toggle');
}