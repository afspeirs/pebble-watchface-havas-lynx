#include <pebble.h>

#define SETTINGS_KEY 1				// Persistent storage key

static Window *s_window;
static TextLayer *s_hour_layer, *s_minute_layer;
static GFont s_time_font;
static BitmapLayer *s_layer_battery, *s_layer_havas;
static GBitmap *s_bitmap_battery, *s_bitmap_havas;
static bool appStarted = false;

typedef struct ClaySettings {
	GColor ColourBackground;
	GColor ColourHour;
	GColor ColourMinute;
	bool ToggleBluetoothQuietTime;
	int SelectBluetooth;
	int SelectBatteryPercent;
} ClaySettings;						// Define our settings struct

static ClaySettings settings;		// An instance of the struct

static void config_default() {
	settings.ColourBackground	= GColorBlack;
	settings.ColourHour			= PBL_IF_BW_ELSE(GColorWhite,GColorChromeYellow);
	settings.ColourMinute		= GColorWhite;
	settings.ToggleBluetoothQuietTime = false;
	settings.SelectBluetooth	  = 2;
	settings.SelectBatteryPercent = 0;
}

static void config_save() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));		// Write settings to persistent storage
}

static void config_load() {
	config_default();													// Load the default settings
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));		// Read settings from persistent storage, if they exist
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Methods /////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void getBatteryIcon() {
	gbitmap_destroy(s_bitmap_battery);
	if (gcolor_equal(gcolor_legible_over(settings.ColourBackground), GColorBlack)) {
		s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_BLACK);
	} else {
		s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_WHITE);
	}
	bitmap_layer_set_bitmap(s_layer_battery, s_bitmap_battery);
}

void getHavasLogo() {
	gbitmap_destroy(s_bitmap_havas);
	if (gcolor_equal(gcolor_legible_over(settings.ColourBackground), GColorBlack)) {
		s_bitmap_havas = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAVAS_BLACK);
	} else {
		s_bitmap_havas = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAVAS_WHITE);
	}
	bitmap_layer_set_bitmap(s_layer_havas, s_bitmap_havas);
}

void setColours() {
	window_set_background_color(s_window, settings.ColourBackground);																	// Set Background Colour
	text_layer_set_text_color(s_hour_layer, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourHour));		// Set Hour Colour
	text_layer_set_text_color(s_minute_layer, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourMinute));	// Set Minute Colour
}

bool vibrateBool() {
	if(quiet_time_is_active() && !settings.ToggleBluetoothQuietTime) {			// True False
		return false;
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// TIME ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void update_time() {
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);
	static char h_buffer[3], m_buffer[3];
	
	strftime(h_buffer, sizeof(h_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);	//%M %I
	strftime(m_buffer, sizeof(m_buffer), "%M", tick_time);		//%M
	text_layer_set_text(s_hour_layer, h_buffer);
	text_layer_set_text(s_minute_layer, m_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Callbacks ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void battery_callback(BatteryChargeState state) {
	if(state.charge_percent <= settings.SelectBatteryPercent) {
		getBatteryIcon();
		layer_set_hidden(bitmap_layer_get_layer(s_layer_battery), false); // Visible
	} else {
		layer_set_hidden(bitmap_layer_get_layer(s_layer_battery), true);  // Hidden
	}
}

static void bluetooth_callback(bool connected) {
	if(!connected) {		// Disconected
		if(appStarted && vibrateBool()) {	
			if(settings.SelectBluetooth == 0) { }								// No vibration 
			else if(settings.SelectBluetooth == 1) { vibes_short_pulse(); }		// Short vibration
			else if(settings.SelectBluetooth == 2) { vibes_long_pulse(); }		// Long vibration
			else if(settings.SelectBluetooth == 3) { vibes_double_pulse(); }	// Double vibration
			else { vibes_long_pulse(); }					 // Default // Long Vibration
		}
	}
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
	// Colours
	Tuple *bg_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_BACKGROUND);
	if(bg_colour_t) { settings.ColourBackground = GColorFromHEX(bg_colour_t->value->int32); }
	Tuple *hr_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_HOUR);
	if(hr_colour_t) { settings.ColourHour = GColorFromHEX(hr_colour_t->value->int32); }
	Tuple *mn_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_MINUTE);
	if(mn_colour_t) { settings.ColourMinute = GColorFromHEX(mn_colour_t->value->int32); }
// Bluetooth
	Tuple *bq_toggle_t = dict_find(iter, MESSAGE_KEY_TOGGLE_BLUETOOTH_QUIET_TIME);
	if(bq_toggle_t) { settings.ToggleBluetoothQuietTime = bq_toggle_t->value->int32 == 1; }
	Tuple *bt_select_t = dict_find(iter, MESSAGE_KEY_SELECT_BLUETOOTH);
	if(bt_select_t) { settings.SelectBluetooth = atoi(bt_select_t->value->cstring); }			// Pretty sure this shouldnt need to convert to a string
// Battery
	Tuple *bp_select_t = dict_find(iter, MESSAGE_KEY_SELECT_BATTERY_PERCENT);
	if(bp_select_t) { settings.SelectBatteryPercent = atoi(bp_select_t->value->cstring); }
	
	config_save();
	
	battery_callback(battery_state_service_peek());
	getHavasLogo();
	appStarted = false;
	bluetooth_callback(connection_service_peek_pebble_app_connection());		// Sets date colours (and detects if a phone is connected)
	appStarted = true;
	setColours();
	update_time();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Window //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void window_load(Window *window) {
	GRect bounds = layer_get_bounds(window_get_root_layer(window));
	
// Fonts
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_BOLD_54));
	
// Locations
	s_hour_layer	= text_layer_create(GRect(			   15, bounds.size.h - 60, bounds.size.w/2 - 15, 75));
	s_minute_layer	= text_layer_create(GRect(bounds.size.w/2, bounds.size.h - 60, bounds.size.w/2 - 15, 75));
	s_layer_havas	= bitmap_layer_create(GRect(bounds.size.w/2-114/2, 0, 114, 118)); // battery
	s_layer_battery	= bitmap_layer_create(GRect(4, 2, 12, 12)); // battery

// Battery Image
	layer_mark_dirty(bitmap_layer_get_layer(s_layer_battery));
	#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(s_layer_battery, GCompOpSet);	
	#endif
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_layer_battery));
	
// Havas
	getHavasLogo();
	layer_mark_dirty(bitmap_layer_get_layer(s_layer_havas));
	#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(s_layer_havas, GCompOpSet);	
	#endif
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_layer_havas));

// Hour
	text_layer_set_font(s_hour_layer, s_time_font);
	text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_hour_layer, GColorClear);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));
	
// Minutes
	text_layer_set_font(s_minute_layer, s_time_font);
	text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_minute_layer, GColorClear);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_minute_layer));

	battery_callback(battery_state_service_peek());
	appStarted = false;
	bluetooth_callback(connection_service_peek_pebble_app_connection());		// Sets date colours (and detects if a phone is connected)
	appStarted = true;
	setColours();
	update_time();
}

static void window_unload(Window *window) {
	text_layer_destroy(s_hour_layer);
	text_layer_destroy(s_minute_layer);
	fonts_unload_custom_font(s_time_font);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Other ///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void init() {
	config_load();

	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
	window_stack_push(s_window, true);
	
	connection_service_subscribe((ConnectionHandlers) {
  		.pebble_app_connection_handler = bluetooth_callback
	});

	app_message_register_inbox_received(inbox_received_handler);
	app_message_open(256, 256);
	
	battery_state_service_subscribe(battery_callback);
	battery_callback(battery_state_service_peek());

	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
	gbitmap_destroy(s_bitmap_battery);
	gbitmap_destroy(s_bitmap_havas);
	bitmap_layer_destroy(s_layer_battery);
	bitmap_layer_destroy(s_layer_havas);
	window_destroy(s_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}