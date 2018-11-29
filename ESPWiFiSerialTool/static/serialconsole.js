/* serialconsole.js
 * Right now, this Javascript is being served from a String function in the .ino file
 * I just put this here for easier reading
 *
 */


 //function to add an HTML <p> element to the serial_display div with the DOM
 function addConsoleEvent(mssg) {
   var text = document.createTextNode(mssg);
   var pg = document.createElement('p');
   pg.appendChild(text);
   var display = document.getElementById('serial_display');
   display.appendChild(pg);
   display.scrollTop = display.scrollHeight;
 }

//function to HTTP Post
function postSerial() {
  var textBox = document.getElementById('serial_input');
  var mssg = textBox.value;

  var xhttp = new XMLHttpRequest();
  xhttp.open('POST', '/serial', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('serial_input=' + mssg);

  textBox.value = '';
  xhttp.onload = function() {
    //Only add the mssg to the console if the ESP8266 received it and sent a response back
    console.log('Serial Post: ' + xhttp.responseText);
    addConsoleEvent('> ' + xhttp.responseText);
  }
}

// Response when user presses "Enter" in the input text box
var textBox = document.getElementById('serial_input');
textBox.addEventListener('keyup', function(e) {
  e.preventDefault();
  if (e.keyCode === 13) { postSerial(); }
});

//Initialize an EventSource to listen for server sent events at /serialupdate
var source = new EventSource('/serialupdate');
if(typeof(EventSource)!=='undefined') {
  console.log('EventSource initialized');

  /* // Server sent event listener version 1: this wasn't working for me, not sure why
  source.onmessage = function(e) {
      console.log('Serial Console Event onmessage' + e.data);
  }; */

  // Server sent event listener version 2: this works for me
  source.addEventListener('serialupdate', function(e) {
      //var data = JSON.parse(e.data);    //json parsing example
      console.log('Serial Console Event: ' + e.data);
      addConsoleEvent('< ' + e.data);
  }, false);

  console.log('EventSource Listener Initialized');
}
else {
  console.log('EventSource Listener could not be initialized');
  addConsoleEvent('Serial Input Console could not be initialized');
  //TODO use polling if EventSource is not supported by browser
}
