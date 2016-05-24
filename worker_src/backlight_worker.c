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

static int16_t stand_rotate[22][3] = {
{1024,536,-168},
{1096,816,-224},
{1128,976,-192},
{1160,1096,-200},
{1232,1288,-48},
{1288,1312,-264},
{1280,1208,-216},
{1224,1120,-336},
{1048,936,-344},
{888,816,-344},
{688,608,-424},
{536,432,-408},
{392,216,-288},
{264,-8,-376},
{192,-184,-240},
{120,-408,-312},
{96,-512,-344},
{80,-672,-360},
{72,-824,-504},
{80,-864,-448},
{64,-896,-488},
{64,-888,-496}};

static int16_t wrist_rotate[21][3] = {
{16,-648,-752},
{16,-600,-800},
{24,-520,-840},
{-8,-320,-288},
{24,216,-176},
{120,952,-104},
{88,1152,-224},
{48,1048,-352},
{24,1040,-176},
{24,1048,-240},
{24,1056,-168},
{24,1152,-184},
{8,1152,-320},
{8,1072,-544},
{-8,864,-656},
{-48,448,-464},
{-56,-272,-544},
{-64,-496,-496},
{40,-456,-872},
{16,-592,-800},
{8,-672,-656}}; 

static int16_t sit_rotate[23][3] = {
{-208,664,-840},
{-208,664,-720},
{-416,568,-912},
{-424,656,-1000},
{-504,648,-904},
{-472,704,-864},
{-408,632,-896},
{-360,512,-856},
{-296,424,-856},
{-232,224,-768},
{-200,-16,-728},
{-160,-72,-704},
{-136,-224,-680},
{-112,-344,-632},
{-88,-448,-600},
{-72,-544,-600},
{-64,-592,-560},
{-72,-664,-520},
{-48,-728,-520},
{-8,-744,-584},
{16,-776,-544},
{40,-808,-568},
{56,-816,-608}};



//bool away_value_added = false; // if true, then we have already incremented by 4

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

int light_timeout = 0;
 
//Variables for standing arm by side
uint storei = 0;
int counti = 0;
//variables for wrist rotate
uint storew = 0;
int countw = 0;
//variables for sitting arm at waist
uint stores = 0;
int counts = 0;
int rotateActivate = 0;
int delayActivate = 0;
int countViewArea = 0;

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
      if(light_on == false){
  /////// ACTIVATE WATCH WHEN STANDING ARM DOWN BY SIDE    
      if(abs(data[i].x - stand_rotate[storei+0][0]) <= 150 || abs(data[i].x - stand_rotate[storei+1][0]) <= 150 || abs(data[i].x - stand_rotate[storei+2][0]) <= 150 ||
         abs(data[i].x - stand_rotate[storei+3][0]) <= 150 || abs(data[i].x - stand_rotate[storei+4][0]) <= 150 || abs(data[i].x - stand_rotate[storei+5][0]) <= 150){
      //    if(abs(data[i].y - stand_rotate[storei+0][1]) <= 200 || abs(data[i].y - stand_rotate[storei+1][1]) <= 200 || abs(data[i].y - stand_rotate[storei+2][1]) <= 200 ||
      //     abs(data[i].y - stand_rotate[storei+3][1]) <= 200 || abs(data[i].y - stand_rotate[storei+4][1]) <= 200 || abs(data[i].y - stand_rotate[storei+5][1]) <= 200){ 
    				if(abs(data[i].z - stand_rotate[storei+0][2]) <= 200 || abs(data[i].z - stand_rotate[storei+1][2]) <= 200 || abs(data[i].z - stand_rotate[storei+2][2]) <= 200 ||
    				abs(data[i].z - stand_rotate[storei+3][2]) <= 200 || abs(data[i].z - stand_rotate[storei+4][2]) <= 200 || abs(data[i].z - stand_rotate[storei+5][2]) <= 200){
              counti++;
              storei = storei + 1;
				    }
  //       }                                                                                                                                                               
      }
       if(storei > 22){
        storei = 0;
        counti = 0;
      }
        
  //////// ACTIVATE WATCH FROM SITTING POSITION
     if(abs(data[i].x - sit_rotate[stores+0][0]) <= 100 || abs(data[i].x - sit_rotate[stores+1][0]) <= 100 || abs(data[i].x - sit_rotate[stores+2][0]) <= 100 ||
        abs(data[i].x - sit_rotate[stores+3][0]) <= 100 || abs(data[i].x - sit_rotate[stores+4][0]) <= 100 || abs(data[i].x - sit_rotate[stores+5][0]) <= 100){
         if(abs(data[i].y - sit_rotate[stores+0][1]) <= 300 || abs(data[i].y - sit_rotate[stores+1][1]) <= 300 || abs(data[i].y - sit_rotate[stores+2][1]) <= 300 ||
          abs(data[i].y - sit_rotate[stores+3][1]) <= 300 || abs(data[i].y - sit_rotate[stores+4][1]) <= 300 || abs(data[i].y - sit_rotate[stores+5][1]) <= 300){
  				  if(abs(data[i].z - sit_rotate[stores+0][2]) <= 250 || abs(data[i].z - sit_rotate[stores+1][2]) <= 250 || abs(data[i].z - sit_rotate[stores+2][2]) <= 250 ||
  				  abs(data[i].z - sit_rotate[stores+3][2]) <= 250 || abs(data[i].z - sit_rotate[stores+4][2]) <= 250 || abs(data[i].z - sit_rotate[stores+5][2]) <= 250){
              counts++;
              stores = stores + 1;
  				  }
           }
        }    
      if(stores > 23){  
        stores = 0;
        counts = 0;
      }
  ////// ACTIVATE WATCH WHEN LOOKING AT WATCH BUT DIDNT ACTIVATE. ROTATE AWAY AND THEN BACK
        
      if(data[i].x < 200 && data[i].x > -200 && data[i].y > Y_SIDE_LOW && data[i].y < Y_SIDE_HIGH){ //looking at watch
        rotateActivate = 20;
      }else{rotateActivate = rotateActivate - 1;} //once outside of this range starts counting down
        
      if(rotateActivate > 0){
        if(abs(data[i].x - wrist_rotate[storew+0][0]) <= 100 || abs(data[i].x - wrist_rotate[storew+1][0]) <=100 || abs(data[i].x - wrist_rotate[storew+2][0]) <= 100 ||
           abs(data[i].x - wrist_rotate[storew+3][0]) <= 100 || abs(data[i].x - wrist_rotate[storew+4][0]) <= 100 || abs(data[i].x - wrist_rotate[storew+5][0]) <= 100){
            if(abs(data[i].y - wrist_rotate[storew+0][1]) <= 150 || abs(data[i].y - wrist_rotate[storew+1][1]) <= 150 || abs(data[i].y - wrist_rotate[storew+2][1]) <= 150 ||
             abs(data[i].y - wrist_rotate[storew+3][1]) <= 150 || abs(data[i].y - wrist_rotate[storew+4][1]) <= 150 || abs(data[i].y - wrist_rotate[storew+5][1]) <= 150){
				//      if(abs(data[i].z - wrist_rotate[storew+0][2]) <= 150 || abs(data[i].z - wrist_rotate[storew+1][2]) <= 150 || abs(data[i].z - wrist_rotate[storew+2][2]) <= 150 ||
				//	     abs(data[i].z - wrist_rotate[storew+3][2]) <= 150 || abs(data[i].z - wrist_rotate[storew+4][2]) <= 150 || abs(data[i].z - wrist_rotate[storew+5][2]) <= 150){
                countw++;
                storew = storew + 1;
					//    }
            }
          }
      }
        if(storew > 23 || rotateActivate <= 0){
          storew = 0;
          countw = 0;
        }
  ///////////////////////////////// check values for activation data 
        if(data[i].x < X_SIDE_HIGH && data[i].x > X_SIDE_LOW && data[i].z > 1000){ //lying down activation on back
              if(abs(data[i].x - lastx) < 50 || delayActivate == 0){  //lets verify that the watch isnt moving much though!
                delayActivate++;
                if(delayActivate > 4){ //now lets activate the light
                  light_enable(true);
                  watch_level_start = time(0L); // activates light
                  counti = 0; storei = 0;
                  countw = 0; storew = 0;
                  counts = 0; stores = 0;
                  rotateActivate = 0;
                  delayActivate = 0;
                  break; //exit for loop to turn on light
                }
              }
        }
     
        if(countw > 4 || counts > 8 || counti > 5 || delayActivate > 0) {
            if(data[i].x < X_SIDE_HIGH && data[i].x > X_SIDE_LOW && data[i].y > Y_SIDE_LOW && data[i].y < Y_SIDE_HIGH){ //watch is in vieable range
              if(abs(data[i].x - lastx) < 50 || delayActivate == 0){  //lets verify that the watch isnt moving much though!
                delayActivate++;
                if(delayActivate > 6){ //now lets activate the light
                  light_enable(true);
                  watch_level_start = time(0L); // activates light
                  counti = 0; storei = 0;
                  countw = 0; storew = 0;
                  counts = 0; stores = 0;
                  rotateActivate = 0;
                  delayActivate = 0;
                  break; //exit for loop to turn on light
                }        
              }
            }else{delayActivate = 0;}
        }
        lastx = data[i].x; // store x value so we can see how it changes next time
      }    
        if(light_on == true){
             if (data[i].y > 75 || data[i].x < -600) { //was 100 and 100   
                watch_level_start = 0;
               // watch_outside_range = true;
                app_timer_register(0, light_callback, NULL); //wait some milliseconds before turning off light
                watch_away = 100;
                light_timeout = 0;
                break;
            }
            light_enable(true);
            
            light_timeout = light_timeout + 1; // starting counting for when to turn off light
            if(light_timeout >= 1000){ // this is around 60 seconds
                watch_level_start = 0;
               // watch_outside_range = true;
                app_timer_register(0, light_callback, NULL); //wait some milliseconds before turning off light
                watch_away = 100;
                light_timeout = 0;
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
