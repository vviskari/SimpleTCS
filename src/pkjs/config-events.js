module.exports = function(minified) {
  var clayConfig = this;
//  var _ = minified._;
//  var $ = minified.$;
//  var HTML = minified.HTML;

  function checkApiKey() {
    if (this.get() === 'y') {
      clayConfig.getItemById('idApiKey').hide();
      clayConfig.getItemById('idApiDescription').hide();
    } else {
      clayConfig.getItemById('idApiKey').show();
      clayConfig.getItemById('idApiDescription').show();
    }
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var weatherRadio = clayConfig.getItemById('idWeatherProvider');
    checkApiKey.call(weatherRadio);
    weatherRadio.on('change', checkApiKey);
  });
};
