# SimpleTCS
Watchface for Pebble Time. Based on "Simple Time and Calendar" by Michael S. Thanks Michael!

Works best in Time series watches and above (Basalt->). PTR is not tested and propably will not show correctly.

## New release available
Pebble services went down for good. Use rebble.io instead. It's not possible to update the apps in rebble store, so in the meantime you can download the new releases [here](https://github.com/vviskari/SimpleTCS/releases).

## Features

### Calendar and time
Shows current time, date and a calendar for previous, current and next week. Week start day, day names and date format can be customized in settings.

### Current step count
Current step count is compared to your daily average (needs Pebble Health). Orange = current steps under 60% of average. White = 60-95% of average. Green = more than 95% of average.

### Battery estimation bar
A battery estimation bar is rendered in the calendar. The bar is visible after first full charge. The estimation is constantly adjusted and averaged over several charge cycles. Removing the watchface will delete the stored history from watch and the estimation starts from beginning.

### Weather forecast
Shows current weather and forecast.

The app uses 'pebble-generic-weather' to fetch weather data. Default weather provider is Yahoo! Weather. It shows only simple weather status data. For a detailed graphical forecast on the watchface, a Weather Underground or OpenWeatherMap API key is required. The API key can be configured in the watchface settings.

Weather Underground has decided to shut down the free API for developers by the end of 2018. For that reason, it is recommended to use OpenWeatherMap API (https://openweathermap.org/api).

Weather condition font is from
http://www.alessioatzeni.com/meteocons/
