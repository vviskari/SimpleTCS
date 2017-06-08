module.exports = function(minified) {
    var clayConfig = this;
    //  var _ = minified._;
    //  var $ = minified.$;
    //  var HTML = minified.HTML;

    function toggleConfigs() {
        if (this.get() === 'y') {
            clayConfig.getItemById('idApiKey').hide();
            clayConfig.getItemById('idApiDescription').hide();
            clayConfig.getItemById('idForecast').hide();
            clayConfig.getItemById('idForecastDescription').hide();
        } else {
            clayConfig.getItemById('idApiKey').show();
            clayConfig.getItemById('idApiDescription').show();
            if (this.get() === 'w') {
                clayConfig.getItemById('idForecast').show();
                clayConfig.getItemById('idForecastDescription').show();
            } else {
                clayConfig.getItemById('idForecast').hide();
                clayConfig.getItemById('idForecastDescription').hide();

            }
        }
    }

    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
        var weatherRadio = clayConfig.getItemById('idWeatherProvider');
        toggleConfigs.call(weatherRadio);
        weatherRadio.on('change', toggleConfigs);
    });
};