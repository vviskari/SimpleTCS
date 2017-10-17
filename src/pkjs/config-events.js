module.exports = function(minified) {
  var clayConfig = this;
  //  var _ = minified._;
  //  var $ = minified.$;
  //  var HTML = minified.HTML;

  function toggleForecastConfig() {
    if (this.get() === 'y') {
      clayConfig.getItemById('idApiKey').hide();
      clayConfig.getItemById('idApiDescription').hide();
      clayConfig.getItemById('idForecast').hide();
      clayConfig.getItemById('idTimeInterval').hide();
      clayConfig.getItemById('idForecastDescription').hide();
    } else {
      clayConfig.getItemById('idApiKey').show();
      clayConfig.getItemById('idApiDescription').show();
      if (this.get() === 'w') {
        clayConfig.getItemById('idForecast').show();
        clayConfig.getItemById('idForecastDescription').show();
        clayConfig.getItemById('idTimeInterval').show();
      } else {
        clayConfig.getItemById('idForecast').hide();
        clayConfig.getItemById('idForecastDescription').hide();
        clayConfig.getItemById('idTimeInterval').hide();
      }
    }
  }

  function toggleTranslationConfig() {
    if (this.get()) {
      clayConfig.getItemById('idMonday').show();
      clayConfig.getItemById('idTuesday').show();
      clayConfig.getItemById('idWednesday').show();
      clayConfig.getItemById('idThursday').show();
      clayConfig.getItemById('idFriday').show();
      clayConfig.getItemById('idSaturday').show();
      clayConfig.getItemById('idSunday').show();
    } else {
      clayConfig.getItemById('idMonday').hide();
      clayConfig.getItemById('idTuesday').hide();
      clayConfig.getItemById('idWednesday').hide();
      clayConfig.getItemById('idThursday').hide();
      clayConfig.getItemById('idFriday').hide();
      clayConfig.getItemById('idSaturday').hide();
      clayConfig.getItemById('idSunday').hide();
    }
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var weatherRadio = clayConfig.getItemById('idWeatherProvider');
    toggleForecastConfig.call(weatherRadio);
    weatherRadio.on('change', toggleForecastConfig);

    var translateRadio = clayConfig.getItemById('idTranslateWeekdays');
    toggleTranslationConfig.call(translateRadio);
    translateRadio.on('change', toggleTranslationConfig);
  });
};