var GenericWeather = require('pebble-generic-weather');
var genericWeather = new GenericWeather();

Pebble.addEventListener('appmessage', function(e) {
  genericWeather.appMessageHandler(e);
});
