# SimpleTCS
Watchface for Pebble Time. Based on "Simple Time and Calendar" by Michael S. Thanks Michael!

Works best in Time series watches and above (Basalt->). PTR is not tested and propably will not show correctly.

### Layout changes
BT connection and battery charge moved to top. Smaller seconds font. Seconds displayed for 1 minute.

### Current step count
Added current step count and compared that to daily average. Orange count = current steps under 60% of average 
White count = 60-95% of average. Green count = more than 95% of average.

### Battery estimation bar
Added a battery estimation bar in the calendar. The bar is visible after first full charge. The estimation is constantly adjusted and averaged over several charge cycles. Removing the watchface will delete the stored history from watch and the estimation starts from beginning.

### Weather
App uses 'pebble-generic-weather' to fetch weather data from Yahoo! Weather. Weather condition font is from
http://www.alessioatzeni.com/meteocons/
