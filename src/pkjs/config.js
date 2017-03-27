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
        "messageKey": "wf",
        "defaultValue": false,
        "label": "Fetch forecast:"
      },
      {
        "type": "text",
        "id": "idForecastDescription",
        "defaultValue": "Enable to fetch weather forecast data. Works only with Weather Underground provider.",
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];