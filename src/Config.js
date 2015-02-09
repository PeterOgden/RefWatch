Pebble.addEventListener("ready", 
  function(e) {
    console.log('Hello World!');
  });

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL('http://cas.ee.ic.ac.uk/~pko11/RefWatch.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  var decoded = JSON.parse(decodeURIComponent(e.response));
  console.log("Config returned: " + e.response);
  Pebble.sendAppMessage(decoded);
});