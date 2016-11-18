#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static GFont s_time_font;
static GFont s_date_font;
static BitmapLayer *s_background_layer, *s_bt_icon_layer, *s_t_l;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap, *s_t_lbm;
static int s_battery_level;
static Layer *s_battery_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00.00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00.00"), "%H.%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00.00"), "%I.%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void update_date() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  static char buffer[] = "Mon";
  
  strftime(buffer, sizeof("Mon"), "%a", tick_time);
  

  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
  update_date();
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 40.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void bluetooth_callback(bool connected) {
  // Hide icon if connected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MAIN_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(64, -15, 96, 42));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(45, -15, 40, 45));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorClear);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_STAR_WRECK_42));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_STAR_WRECK_42));
  
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Improve the layout to be more like a watchface
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCON);
  
  // Create the BitmapLayer to display the GBitmap
  s_bt_icon_layer = bitmap_layer_create(GRect(5, 5, 16, 45));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(bluetooth_connection_service_peek());
  
  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(64, 35, 40, 15));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create GBitmap, then set to created BitmapLayer
  s_t_lbm = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_OVERLAY);
  s_t_l = bitmap_layer_create(GRect(80, 44, 23, 6));
  bitmap_layer_set_bitmap(s_t_l, s_t_lbm);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_t_l));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_date_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
  
  gbitmap_destroy(s_t_lbm);
  bitmap_layer_destroy(s_t_l);
  
  layer_destroy(s_battery_layer);
}

static void init(){
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Set original time
  update_time();
  update_date();
  
  // Register for Bluetooth connection updates
  bluetooth_connection_service_subscribe(bluetooth_callback);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit(){
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}