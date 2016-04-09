#include <pebble_worker.h>

#define DURATION	6               /* copied from backlight.c */
#define SAMPLES		7               /* copied from backlight.c */
#define CHARGING	8
#define PLUGGED		9
#define AMBIENT		10

void light_enable_interaction(void);
void light_enable(bool val);

bool auto_backlight = false;
bool light_on = false;
time_t watch_level_start = 0;
uint32_t time_duration=15;               /* default */
uint32_t samples=1;               /* default */
bool charging=false;
bool light_charging = false;            /* current have light on while charging */
bool plugged=false;
bool light_plugged = false;            /* current have light on while powered */
bool ambient=false;
bool away_value_added = false; // if true, then we have already incremented by 4

int16_t lastx = 0;
int16_t lasty = 0;


/*
 * Trying out turning on the backlight via the function which understands
 * ambient light.
 */
void
backlight_enable (bool val) 
{

    if (val)
        light_enable_interaction();
}



void
light_callback (void *data) 
{ 

    light_on = false;
    light_enable(false);
}


#define X_RANGE_LOW -250
#define X_RANGE_HIGH 250
#define Y_RANGE_LOW -1000
#define Y_RANGE_HIGH -300

int16_t X_SIDE_LOW = -200;
int16_t X_SIDE_HIGH = 200;
int16_t Y_SIDE_LOW = -1200;
int16_t Y_SIDE_HIGH = -200;

int watch_away = 100;
bool watch_looking = false;
bool moved_away = false;
int count = 0;
int look_first_count = 0;
int light_timeout = 0;
//int look_second_count = 0;
//AJB we are going to store x/y values and compare the change, if too much change we don't turn on

void
handle_accel(AccelData *data, uint32_t num_samples)
{
   // static bool watch_outside_range = true;

    uint i;

    if (light_charging == true || light_plugged == true) {
        /* Don't bother, but leave the light on while charging or powered */
       return;
    }

    for (i = 0 ; i < num_samples ; i++) {
      
      if (data[i].x < X_SIDE_HIGH && data[i].x > X_SIDE_LOW &&
            data[i].y > Y_SIDE_LOW && data[i].y < Y_SIDE_HIGH) { //you are looking at the watch - 10 ms of looking and it will turn on
          
          if(abs(data[i].x - lastx) < 30 || abs(data[i].y - lasty) < 80){ //if watch hasn't moved much increment closer to turning on
            //watch_looking = true;
            count++;
            } 
          else{count = 0;} //if too much movement, reset to 0
        
          if(count == 0 || count == 3 || count == 7){
            lastx = data[i].x;
            lasty = data[i].y;
          }
          if (light_on == false && watch_level_start == 0 && count >= 10 ){ //&& look_second_count >= 2) {   
                    watch_level_start = time(0L); // record when level started
                    break;
            } 
            //look_first_count++; 
      } 
   
      
      if(data[i].x > 900 && data[i].y > 0) { //watch has been put down to your side
            watch_away = 0;
            away_value_added = false;
      }
      if(data[i].x < 800) {watch_away++;};  
      
      if(count >= 11){count = 0;} 
      
      if(watch_away < 10){ //watch is away so now we have 10 tries to look at watch
            if (data[i].x < X_SIDE_HIGH && data[i].x > X_SIDE_LOW &&
            data[i].y > Y_SIDE_LOW && data[i].y < Y_SIDE_HIGH && away_value_added == false) { 
              count = count + 9;
              //force it to have close data
              lastx = data[i].x;
              lasty = data[i].y;
              away_value_added = true;
            }
      } 
        if(light_on == true){
          
             if (data[i].y > 75 || data[i].x < -600) { //was 100 and 100   
                watch_level_start = 0;
               // watch_outside_range = true;
                app_timer_register(0, light_callback, NULL); //wait some milliseconds before turning off light
                watch_away = 100;
                count = 0;
                light_timeout = 0;
                break;
            }
          
            light_timeout = light_timeout + 1; // starting counting for when to turn off light
            if(light_timeout >= 350){
                light_enable(false);
                break;
            }
            else{
              light_enable(true); //force the light on even when screens change
            }
          }
    }
    
    if (watch_level_start != 0 && (time(0L) - watch_level_start) > 0) {
        if (ambient) {
            backlight_enable(true);
        } else {
            light_enable(true);
        }
	  light_on = true;
	  //app_timer_register(time_duration * 1000, light_callback, NULL); //light will still timeout after certain period, but I will set it long
	  watch_level_start = 0;
    }  
}


void
battery_handler (BatteryChargeState charge) 
{

    //if (charge.is_charging && charging) {
    if (charge.is_charging) {
        /*light_enable(true);
        light_on = true;*/
        light_charging = true;
        light_enable(false);
        light_on = false;
    } else if (charge.is_plugged) {
        light_enable(true);
        light_plugged = true;
        light_on = true;
    } else {
        light_charging = false;
        light_plugged = false;
        if (light_on == true) {
            light_enable(false);
            light_on = false;
        }
    }
}



int main(void) {
    uint32_t	val;

    val = persist_read_int(DURATION);
    if (val) {
	time_duration = (int)val;
    } else {
        time_duration = 5;              /* default */
    }
    APP_LOG(APP_LOG_LEVEL_WARNING, "time_duration=%u", (uint)val);

    val = persist_read_int(SAMPLES);
    if (val) {
	samples = (int)val;
    } else {
        samples = 1;              /* default */
    }
    APP_LOG(APP_LOG_LEVEL_WARNING, "samples=%u", (uint)val);

    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    accel_data_service_subscribe(samples, handle_accel);
    auto_backlight = true;

    val = persist_read_bool(CHARGING);
    if (val) {
	charging = (bool)val;
    } else {
        charging = false;              /* default */
    }
    APP_LOG(APP_LOG_LEVEL_WARNING, "light when charging = %u",
            (uint)val);

    val = persist_read_bool(PLUGGED);
    if (val) {
	plugged = (bool)val;
    } else {
        plugged = false;              /* default */
    }
    APP_LOG(APP_LOG_LEVEL_WARNING, "light when plugged = %u",
            (uint)val);

    val = persist_read_bool(AMBIENT);
    if (val) {
	ambient = (bool)val;
    } else {
        ambient = false;              /* default */
    }
    APP_LOG(APP_LOG_LEVEL_WARNING, "ambient=%u", (uint)val);

    if (charging || plugged) {
        battery_state_service_subscribe(battery_handler);
    }

    worker_event_loop();
}
