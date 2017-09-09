module.exports = [{
        "type": "heading",
        "defaultValue": "Simple TCS: Configuration"
    },
    {
        "type": "text",
        "defaultValue": "You can configure the watch face below."
    },
    {
        "type": "section",
        "items": [{
                "type": "select",
                "messageKey": "wsd",
                "defaultValue": "m",
                "label": "Week starts on:",
                "options": [{
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
                "options": [{
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
                ]
            }
        ]
    },
    {
        "type": "section",
        "items": [{
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
        "items": [{
                "type": "heading",
                "defaultValue": "Weather"
            },
            {
                "type": "select",
                "messageKey": "wt",
                "defaultValue": "C",
                "label": "Show temperature in:",
                "options": [{
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
                "options": [{
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