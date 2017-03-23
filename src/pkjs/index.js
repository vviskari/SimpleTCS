var GenericWeather = require('pebble-generic-weather');
var genericWeather = new GenericWeather();
Pebble.addEventListener('appmessage', function(e) {
  genericWeather.appMessageHandler(e);
});

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var configEvents = require('./config-events');
var clay = new Clay(clayConfig, configEvents);
