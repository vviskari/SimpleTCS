module.exports = [
  {
    "type": "heading",
    "defaultValue": "Simple TCS: Configuration"
  },
  {
    "type": "text",
    "defaultValue": "You can configure the watch face below."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "select",
        "messageKey": "wsd",
        "defaultValue": "m",
        "label": "Week starts on:",
        "options": [
          {
            "label": "Monday",
            "value": "m"
          },
          {
            "label": "Sunday",
            "value": "s"
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "df",
        "defaultValue": "1",
        "label": "Date format:",
        "options": [
          {
            "label": "dd.MM.yyyy",
            "value": "1"
          },
          {
            "label": "MM/dd/yyyy",
            "value": "2"
          },
          {
            "label": "yyyy-MM-dd",
            "value": "3"
          },
          {
            "label": "dd.MM.",
            "value": "4"
          },
          {
            "label": "MM.dd.",
            "value": "5"
          }
        ]
      },
      {
        "type": "toggle",
        "id": "idTranslateWeekdays",
        "messageKey": "translate_days",
        "defaultValue": false,
        "label": "Translate weekdays:"
      },
      {
        "type": "text",
        "id": "idTranslateDescription",
        "defaultValue": "Use weekday names based on system locale or provide your own by enabling this setting and translating the days.",
      },
      {
        "type": "input",
        "id": "idMonday",
        "messageKey": "monday",
        "defaultValue": "Monday",
        "attributes": {
          "size": 14
        }
      },
      {
        "type": "input",
        "id": "idTuesday",
        "messageKey": "tuesday",
        "defaultValue": "Tuesday",
        "attributes": {
          "size": 14
        }
      },
      {
        "type": "input",
        "id": "idWednesday",
        "messageKey": "wednesday",
        "defaultValue": "Wednesday",
        "attributes": {
          "size": 14
        }
      },
      {                                    
        "type": "input",
        "id": "idThursday",
        "messageKey": "thursday",
        "defaultValue": "Thursday",
        "attributes": {
          "size": 14
        }
      },
      {
        "type": "input",
        "id": "idFriday",
        "messageKey": "friday",
        "defaultValue": "Friday",
        "attributes": {
          "size": 14
        }
      },
      {
        "type": "input",
        "id": "idSaturday",
        "messageKey": "saturday",
        "defaultValue": "Saturday",
        "attributes": {
          "size": 14
        }
      },
      {
        "type": "input",
        "id": "idSunday",
        "messageKey": "sunday",
        "defaultValue": "Sunday",
        "attributes": {
          "size": 14
        }
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "radiogroup",
        "messageKey": "vm",
        "defaultValue": "s",
        "label": "Watchface behaviour:",
        "options": [
          {
            "label": "Shake 3x to change",
            "value": "s"
          },
          {
            "label": "Shake 2x to change",
            "value": "t"
          },
          {
            "label": "Show calendar",
            "value": "c"
          },
          {
            "label": "Show forecast",
            "value": "f"
          }
        ]
      },
      {
        "type": "text",
        "id": "idBehaviourDescription",
        "defaultValue": "Select how the watchface should behave. Always display calendar or weather forecast, or switch between them with 3 shakes. Selecting '2 shakes' will first switch to seconds and then another 2 will change mode.",
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather"
      },
      {
        "type": "select",
        "messageKey": "wt",
        "defaultValue": "C",
        "label": "Show temperature in:",
        "options": [
          {
            "label": "Celsius",
            "value": "C"
          },
          {
            "label": "Fahrenheit",
            "value": "F"
          }
        ]
      },
      {
        "type": "radiogroup",
        "messageKey": "wp",
        "id": "idWeatherProvider",
        "defaultValue": "y",
        "label": "Weather provider:",
        "options": [
          {
            "label": "Yahoo!",
            "value": "y"
          },
          {
            "label": "OpenWeatherMap",
            "value": "o"
          },
          {
            "label": "Weather Underground",
            "value": "w"
          },
          {
            "label": "Forecast.io",
            "value": "f"
          }
        ]
      },
      {
        "type": "input",
        "id": "idApiKey",
        "messageKey": "wak",
        "defaultValue": "",
        "label": "Weather API key:",
        "attributes": {
          "placeholder": "<your API key>",
          "limit": 20
        }
      },
      {
        "type": "text",
        "id": "idApiDescription",
        "defaultValue": "Insert your weather API key above. If you selected Yahoo! above, the API key is NOT required.",
      },
      {
        "type": "toggle",
        "id": "idForecast",
        "messageKey": "temp_grid",
        "defaultValue": true,
        "label": "Render temperature grid:"
      },
      {
        "type": "text",
        "id": "idForecastDescription",
        "defaultValue": "Enable to render the time and temp grid in weather forecast chart. Works only with Weather Underground provider.",
      },
      {
        "type": "radiogroup",
        "messageKey": "time_interval",
        "id": "idTimeInterval",
        "defaultValue": "1",
        "label": "Forecast time interval:",
        "options": [
          {
            "label": "6h",
            "value": "1"
          },
          {
            "label": "12h",
            "value": "2"
          },
          {
            "label": "12h am/pm",
            "value": "3"
          }
        ]
      }
    ]
      },
      {
      "type": "submit",
      "defaultValue": "Save Settings"
      }
    ];