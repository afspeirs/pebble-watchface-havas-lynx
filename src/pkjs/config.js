module.exports = [
	{ 
    	"type": "heading", 
    	"defaultValue": "Havas Lynx",
		"size": 1,
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Colour Selection"
			},
			{
				"type": "color",
				"messageKey": "COLOUR_BACKGROUND",
				"label": "Background",
				"defaultValue": "000000",
				"sunlight": true
			},
			{
				"type": "color",
				"messageKey": "COLOUR_HOUR",
				"label": "Hour",
				"defaultValue": "FFAA00",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
			{
				"type": "color",
				"messageKey": "COLOUR_MINUTE",
				"label": "Minute",
				"defaultValue": "FFFFFF",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			}
		]
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Bluetooth"
			},
			{
				"type": "select",
				"messageKey": "SELECT_BLUETOOTH",
				"defaultValue": "2",
				"label": "Vibration on disconnection",
				"options": [
					{ 
						"label": "None",
						"value": "0" 
					},
					{ 
						"label": "Short",
						"value": "1" 
					},
					{ 
						"label": "Long",
						"value": "2" 
					},
					{ 
						"label": "Double",
						"value": "3" 
					}
				]
			},
			{
				"type": "toggle",
				"messageKey": "TOGGLE_BLUETOOTH_QUIET_TIME",
				"label": "Vibrate during Quiet Time",
				"defaultValue": false,
			},
		]
 	},
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Battery"
			},
			{
				"type": "select",
				"messageKey": "SELECT_BATTERY_PERCENT",
				"defaultValue": "0",
				"label": "Show battery icon",
				"options": [
					{
						"label": "Never",
						"value": "0" 
					},
					{
						"label": "10%",
						"value": "10" 
					},
					{
						"label": "20%",
						"value": "20"
					},
					{ 
						"label": "30%",
						"value": "30"
					},
					{
						"label": "40%",
						"value": "40"
					}
				]
			},
		]
 	},
	{
		"type": "submit",
		"defaultValue": "Save"
	},
	{
		"type": "text",
		"defaultValue": " Version 0.2",
	},
];

