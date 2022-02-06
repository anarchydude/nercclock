#include <Conceptinetics.h>
#include <Rdm_Defines.h>
#include <Rdm_Uid.h>
#include <JC_Button.h>
#include <Adafruit_NeoPixel.h>

// Colors vars are in decimal format for the NeoPixel
#define PIN 22
#define NUMPIXELS 168
#define WHITE 16777215
#define GREEN 65280
#define BLUE 255
#define RED 16711680
#define YELLOW 16776960
#define ORANGE 16544258
#define LED_R 40
#define LED_B 42
#define TAP_R 44
#define TAP_B 46
#define min3b 36
#define min5b 34
#define min2b 38
#define min3l 37
#define min5l 35
#define min2l 39
#define pausel 31
#define startl 33
#define stopl 29
#define pauseb 30
#define startb 32
#define stopb 26


//DMX
#define DMX_MASTER_CHANNELS   24
#define RXEN_PIN                2
DMX_Master        dmx_master ( DMX_MASTER_CHANNELS, RXEN_PIN );

//Sound
#define NEXT_SONG 0X01  
#define PREV_SONG 0X02  
#define CMD_PLAY_W_INDEX 0X03 //DATA IS REQUIRED (number of song) 
#define VOLUME_UP_ONE 0X04 
#define VOLUME_DOWN_ONE 0X05 
#define CMD_SET_VOLUME 0X06//DATA IS REQUIRED (number of volume from 0 up to 30(0x1E)) 
#define SET_DAC 0X17 
#define CMD_PLAY_WITHVOLUME 0X22 //data is needed  0x7E 06 22 00 xx yy EF;(xx volume)(yy number of song) 
#define CMD_SEL_DEV 0X09 //SELECT STORAGE DEVICE, DATA IS REQUIRED 
#define DEV_TF 0X02 //HELLO,IM THE DATA REQUIRED 
#define SLEEP_MODE_START 0X0A 
#define SLEEP_MODE_WAKEUP 0X0B 
#define CMD_RESET 0X0C//CHIP RESET 
#define CMD_PLAY 0X0D //RESUME PLAYBACK 
#define CMD_PAUSE 0X0E //PLAYBACK IS PAUSED 
#define CMD_PLAY_WITHFOLDER 0X0F//DATA IS NEEDED, 0x7E 06 0F 00 01 02 EF;(play the song with the directory \01\002xxxxxx.mp3 ex. 0x0102
#define STOP_PLAY 0X16 
#define PLAY_FOLDER 0X17// data is needed 0x7E 06 17 00 01 XX EF;(play the 01 folder)(value xx we dont care) 
#define SET_CYCLEPLAY 0X19//data is needed 00 start; 01 close 
#define SET_DAC 0X17//data is needed 00 start DAC OUTPUT;01 DAC no output 

static int8_t Send_buf[8] = {0} ;//The MP3 player understands orders in a 8 int string 


Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
 int clk_time = 180;
 int c1, c2, c3, dig, num, baseled, set_clk_tim = 180;
 long color;
 bool count_pulse, run_clock, match_active, ready_b, ready_r, active, waiting_for_players, flash, paused, red_tap_out, blue_tap_out;

 Button pause_db(pauseb);
 Button start_db(startb);

void setup() {
  // put your setup code here, to run once:

  pause_db.begin();
  start_db.begin();


// Start Wireless Serial port to OSB connection - set bus speed
Serial1.begin(38400);

// Start Serial port to MP3 board - set bus speed
Serial3.begin(9600);
delay(500);//Wait chip initialization is complete 
sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card   
delay(200);//wait for 200ms 
sendCommand(CMD_SET_VOLUME, 0x19); //set the volume now so there's less latency down the line

//Clear Clock
strip.begin();
strip.setBrightness(255);

pinMode(TAP_R,INPUT_PULLUP);
pinMode(TAP_B,INPUT_PULLUP);
pinMode(LED_R,OUTPUT);
pinMode(LED_B,OUTPUT);
digitalWrite(LED_R,LOW);
digitalWrite(LED_B,LOW);
waiting_for_players = true;

pinMode(min3b,INPUT_PULLUP);
pinMode(min5b,INPUT_PULLUP);
pinMode(min2b,INPUT_PULLUP);
pinMode(pauseb,INPUT_PULLUP);
pinMode(startb,INPUT_PULLUP);
pinMode(stopb,INPUT_PULLUP);

pinMode(min3l,OUTPUT);
pinMode(min5l,OUTPUT);
pinMode(min2l,OUTPUT);
pinMode(pausel,OUTPUT);
pinMode(startl,OUTPUT);
pinMode(stopl,OUTPUT);

// configure interrupt for 1 second pulse
 noInterrupts();
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // 1 Hz (16000000/((15624+1)*1024))
  OCR1A = 15624;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
  interrupts();

  //DMX
    dmx_master.enable (); 
      dmx_master.setChannelRange ( 1, 24, 0 ); // ( begin_channel, end_channel, byte_value ) This sets all channels between begin and end to the value specified 
      dmx_master.setChannelValue (1,255); // ( channel, byte_value ) By default, we are setting the master dimmer of the Amazon light to full brightness.
      dmx_master.setChannelValue (12,255); // Setting Red square Rockville bar to full brightness dimmer.
      dmx_master.setChannelValue (19,255); // Setting Blue square Rockville bar to full brightness dimmer.

  //rainbow(0);
  strip.clear();
  strip.show();

    set_clk_tim = 180;
  digitalWrite(min3l, true);
  digitalWrite(min2l, false);
  digitalWrite(min5l, false); 
}

// ## DMX Light from Ed ##
// 1: Master Dimmer(0~255) RGBW-Dark to Full Brightness; 
// 2. Strobe(0~255) from slow to fast; 
// 3. 0-17: Red; 18-35: Green; 36-53: Blue; 54--71: White; 72-88: Yellow; 89-107: Light Blue; 108-125: Purple; 126-127 Full Light; 128-169: Red & Blue Pulse Change; 170-210: 6 Color Pulse Change; 211-255: Gradient Change 
// 4. Function Speed - Slow to Fast; 
// 5. Red(O~255) - Dark to Full Brightness; 
// 6. Green (0~255) - Dark to Full Brightness; 
// 7. Blue (0~255) - Dark to Full Brightness;
// 8, White (0~255) - Dark to Full Brightness.

// ## DMX Rockville Bars
// 1 - Red (0-255)
// 2 - Green (0-255)
// 3 - Blue (0-255)
// 4 - Master Dimmer (0-255)
// 5 - If ch 7 set to 0, Strobe speed (off 0-255 full)
//   - If ch 7 set to 1-255, Running speed (0-255)
// 6 - Chase mode (0-25) Mode 0, cycles through chases 1-8
//   - Chase mode (26-255) Modes 1-9
// 7 - Dimmer mode 0
//   - Chase mode 1-13
//   - Sound Mode 14-27 A0 (runs the entire A cycle all modes)
//   - Sound mode 28-139 A1-A8
//   - Sound mode 140-153 B0 (runs the entire B cycle all modes)
//   - Sound mode 154-255 B1-B8

//void loop() {
//  Serial1.print(digitalRead(min2b));
//   Serial1.print(digitalRead(min3b));
//    Serial1.print(digitalRead(min5b));
//     Serial1.print(digitalRead(startb));
//      Serial1.print(digitalRead(stopb));
//       Serial1.println(digitalRead(pauseb));
//       delay(50);

//}



void loop() {

  // Returns the current debounced button state, true for pressed,
  // false for released. Call this function frequently to ensure
  // the sketch is responsive to user input.
  pause_db.read();
  start_db.read();

  // If both player ready buttons are pressed, fight is active, but clock is not running, flash the start button 
  if ((waiting_for_players == false) and (active == true) and (run_clock == false)) {
      digitalWrite(startl, flash);
  }

  // If the fight is not active, and you're waiting for players..
  // AKA Pre-Fight
  if ((active == false) and (waiting_for_players == true)){

    // If the Red Tap button is pressed
    if (digitalRead(TAP_R) == false){
      red_ready(); // Plays sound, sets red ready, notifies on clock.
    }

    // If Red Tap is not set to ready, and you're waiting for players, flash the Red Tap LED
    if ((ready_r == false) and (waiting_for_players == true)){
      digitalWrite(LED_R, flash);
    }

    // If the Blue Tap button is pressed
    if (digitalRead(TAP_B) == false){
      blue_ready();
    }

    // If Blue Tap is not set to ready, and you're waiting for players, flash the Blue Tap LED
    if ((ready_b == false) and (waiting_for_players == true)){
      digitalWrite(LED_B, flash);
    }

    // Start override. If the start button is pressed for 2 seconds, set both players ready, which kicks off judge start.. shouldn't kick it off TBH
    if(start_db.pressedFor(2000)) {
      blue_ready();
      red_ready();
    }

    // If both tap buttons are pressed (set to ready), you're now not waiting for players, and the fight is now active.
    if ((ready_r == true) and (ready_b == true)) {
      //run_clock = true;
      waiting_for_players = false;
      //tree_start();
      active = true; //move to judge start
    }

    //Judge 2 minute set
    // If the 2 min button is pressed, set clk time int to 120, light up 2 min, turn off 3 and 5 min lights

    if (digitalRead(min2b) == false){
      set_clk_tim = 120;
      digitalWrite(min2l, true);
      digitalWrite(min3l, false);
      digitalWrite(min5l, false);  
    }

    //Judge 3 minute set
    // If the 3 min button is pressed, set clk time int to 180, light up 3 min, turn off 2 and 5 min lights

    if (digitalRead(min3b) == false){
      set_clk_tim = 180;
      digitalWrite(min3l, true);
      digitalWrite(min2l, false);
      digitalWrite(min5l, false);  
    }

    //Judge 5 minute set
    // If the 5 min button is pressed, set clk time int to 300, light up 5 min, turn off 2 and 3 min lights

    if (digitalRead(min5b) == false){
      set_clk_tim = 300;
      digitalWrite(min5l, true);
      digitalWrite(min3l, false);
      digitalWrite(min2l, false);  
    }

  }

  // If you are not waiting for players, and the fight is active..
  if ((waiting_for_players == false) and (active == true)){

    //Judge Pause
    // If the pause button was pressed, reads current state. Changes state if it was not present. Also, if the clock is running..
    if (pause_db.wasPressed() and (run_clock == true)) {
      paused = !paused; // Paused = "true" becomes "not true". See below in clock IF statement. Technically paused starts false?
      // Insert sound here for pause - endCommand(CMD_PLAY_WITHFOLDER, 0x0106);
      digitalWrite(pausel, paused); // Pause light becomes true?
      //dmx_master.setChannelValue( 5, 255);
      //dmx_master.setChannelValue( 6, 140);
    }

    // Judge Stop
    // if the stop button reads false (pressed), and run_clock is true (bool)
    // Set run_clock false, clk_time resets to the set_clk_tim, set fight as not active, both player buttons not ready, set the ready button lights to low, reset to waiting for players.

    if ((digitalRead(stopb) == false) and (run_clock)){
      run_clock = false;
      clk_time = set_clk_tim;
      active = false;
      ready_b = false;
      ready_r = false;
      delay(10);
      digitalWrite(LED_R,LOW);
      delay(10);
      digitalWrite(LED_B,LOW);
      delay(10);
      waiting_for_players = true;
      tree_stop();
    }

    //Judge Start
    //Will need to look into moving the run_clock = true inside of tree_start so that the christmas tree can stay lit while the clock starts.
    if ((digitalRead(startb) == false) and (run_clock == false)){
      clk_time = set_clk_tim;
      digitalWrite(startl, true);
      tree_start();
      run_clock = true;
    
    }

    //Blue Tap Out
    
    if ((digitalRead(TAP_B) == false) and (run_clock)){
      tap_out_blue();
    }

    //Red Tap Out
    
    if ((digitalRead(TAP_R) == false) and (run_clock)){
      tap_out_red();
    }

    // Clock run Segment
    // run timer if run_clock = true
    // Essentially, if the clock is running, and paused is "false" (Active)
    if (count_pulse & run_clock & !paused) {
      clk_time--;
      count_pulse = false; // this stops the count pulse, right?
      c1 = clk_time / 60;
      c2 = (clk_time % 60)/10;
      c3 = (clk_time % 60) % 10;

      // output clock to OBS

      Serial1.print(c1, DEC);
      Serial1.print(":");
      Serial1.print(c2, DEC);
      Serial1.println(c3, DEC);

      // ##Clock warnings##
      // Two min warning
      if (clk_time == 120) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0107);
      }

      // One min warning
      if (clk_time == 60) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0108);
      }

      // 10 seconds countdown
      if (clk_time == 10) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }
      
      // 10 seconds countdown
      if (clk_time == 9) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }
      
      // 10 seconds countdown
      if (clk_time == 8) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 7) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 6) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 5) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 4) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 3) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 2) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // 10 seconds countdown
      if (clk_time == 1) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0109);
      }

      // Color changes to LED clock - time milestones
      if (clk_time > 60) {color = GREEN;}
      if ((clk_time <=60) && (clk_time >=11)) {color = YELLOW;}
      if (clk_time <=10) {color = RED;}

      // Clear the panel, set each of the three digits with the clock time above, and what color.
      strip.clear();
      led_num(0, c1, color);
      led_num(1, c2, color);
      led_num(2, c3, color);
      strip.show();

    }

    //Times up
    if (clk_time == 0) {
      sendCommand(CMD_PLAY_WITHFOLDER, 0x0105);
      run_clock = false;
      clk_time = set_clk_tim;
      active = false;
      ready_b = false;
      ready_r = false;
      delay(10);
      digitalWrite(LED_R,LOW);
      delay(10);
      digitalWrite(LED_B,LOW);
      delay(10);
      waiting_for_players = true;
      //strip.clear();
      //strip.show();
      tree_stop();}
  }
}

void led_num(int dig, int num, long color) {
  baseled = dig * 56;
  

    switch (num){
    
    case 0: 
      strip.fill(color, baseled, 48);
      break;

    case 1:
      strip.fill(color, baseled + 16, 16);
      break;

    case 2:
      strip.fill(color, baseled + 8, 16);
      strip.fill(color, baseled + 32, 24);
      break;

    case 3:
      strip.fill(color, baseled + 8, 32);
      strip.fill(color, baseled + 48, 8);
      break;

    case 4:
      strip.fill(color, baseled, 8);
      strip.fill(color, baseled + 16, 16);
      strip.fill(color, baseled + 48, 8);
      break;

   case 5:
      strip.fill(color, baseled, 16);
      strip.fill(color, baseled + 24, 16);
      strip.fill(color, baseled + 48, 8);
      break;

   case 6:
      strip.fill(color, baseled, 16);
      strip.fill(color, baseled + 24, 32);
      break;

   case 7:
      strip.fill(color, baseled + 8, 24);
      break;

   case 8:
      strip.fill(color, baseled, 56);
      break;

   case 9:
      strip.fill(color, baseled, 40);
      strip.fill(color, baseled + 48, 8);
      break;
  }

}

ISR(TIMER1_COMPA_vect) {
 count_pulse = true;
 flash = !flash;

  
}

void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}


void red_ready() {
  sendCommand(CMD_PLAY_WITHFOLDER, 0x0103);
  ready_r = true;
  digitalWrite(LED_R,HIGH);
  led_num(0,0,RED); // Set left digit (0) to "0", color RED, then show it on the strip.
  strip.show();
  dmx_master.setChannelValue ( 9, 255);
  //dmx_master.setChannelValue ( 3, 17 );
}

void blue_ready() {
  sendCommand(CMD_PLAY_WITHFOLDER, 0x0102);
  ready_b = true;
  digitalWrite(LED_B,HIGH);
  led_num(2,0,BLUE); // Set right digit (2) to "0", color BLUE, then show it on the strip.
  strip.show();
  dmx_master.setChannelValue ( 18, 255);
  //dmx_master.setChannelValue ( 3, 53);
}

void tree_start() {
  dmx_master.setChannelValue ( 9, 0);
  dmx_master.setChannelValue ( 18, 0);
  delay (100);
    sendCommand(CMD_PLAY_WITHFOLDER, 0x0101); // Need to let sound play concurrently with the start of the clock.
  // Set the delays for a longer light time with a blip of dark, then extend green start for 5 secs.
  // Future change, a smooth pulse
  dmx_master.setChannelValue ( 4, 30);
  dmx_master.setChannelValue ( 3, 88);
  dmx_master.setChannelRange ( 9, 11, 255);
  dmx_master.setChannelRange ( 16, 18, 255);
  delay (750);
  dmx_master.setChannelValue ( 3, 0);
  dmx_master.setChannelRange ( 9, 11, 0);
  dmx_master.setChannelRange ( 16, 18, 0);
  delay (250);
  dmx_master.setChannelValue ( 3, 88);
  dmx_master.setChannelRange ( 9, 11, 255);
  dmx_master.setChannelRange ( 16, 18, 255);
  delay (750);
  dmx_master.setChannelValue ( 3, 0);
  dmx_master.setChannelRange ( 9, 11, 0);
  dmx_master.setChannelRange ( 16, 18, 0);
  delay (250);
  dmx_master.setChannelValue ( 3, 88);
  dmx_master.setChannelRange ( 9, 11, 255);
  dmx_master.setChannelRange ( 16, 18, 255);
  delay (750);
  dmx_master.setChannelValue ( 3, 0);
  dmx_master.setChannelRange ( 9, 11, 0);
  dmx_master.setChannelRange ( 16, 18, 0);
  delay (250);
  dmx_master.setChannelValue ( 3, 35);
  dmx_master.setChannelValue ( 10, 255);
  dmx_master.setChannelValue ( 17, 255);
  delay (1000); // The right amount of glow.. but the clock won't change over fast enough.
  dmx_master.setChannelRange ( 3, 4, 0);
  dmx_master.setChannelValue ( 10, 0);
  dmx_master.setChannelValue ( 17, 0);
}

void tree_stop() {
  digitalWrite(startl, false);
  digitalWrite(stopl, true);
  sendCommand(CMD_PLAY_WITHFOLDER, 0x0105);
  //dmx_master.setChannelValue ( 2, 255);
  dmx_master.setChannelValue ( 3, 17);
  dmx_master.setChannelValue ( 9, 255);
  dmx_master.setChannelValue ( 16, 255);
  delay (12000);
  dmx_master.setChannelValue ( 3, 0);
  dmx_master.setChannelValue ( 9, 0);
  dmx_master.setChannelValue ( 16, 0);
  //dmx_master.setChannelValue ( 2, 0);
  strip.clear();
  strip.show();
  run_clock = false;
  digitalWrite(stopl, false);
  paused = false;
  digitalWrite(pausel, false);
}

void tap_out_red() {
  Serial1.print("redtapout"); // Do this first, so that it outputs to OBS ASAP
  sendCommand(CMD_PLAY_WITHFOLDER, 0x0104); // Play tapout sound.
  run_clock = false;
  clk_time = set_clk_tim;
  active = false;
  ready_b = false;
  ready_r = false;
  delay(10);
  digitalWrite(LED_R,LOW);
  delay(10);
  digitalWrite(LED_B,LOW);
  delay(10);
  waiting_for_players = true;
  strip.clear();
  strip.show();
    dmx_master.setChannelValue ( 2, 200);
    dmx_master.setChannelValue ( 3, 17);
    dmx_master.setChannelValue ( 9, 255);
    dmx_master.setChannelValue ( 13, 180);
    dmx_master.setChannelValue ( 16, 255);
    dmx_master.setChannelValue ( 20, 180);
  delay (4000);
    dmx_master.setChannelRange ( 2, 3, 0);
    dmx_master.setChannelValue ( 9, 0);
    dmx_master.setChannelValue ( 13, 0);
    dmx_master.setChannelValue ( 16, 0);
    dmx_master.setChannelValue ( 20, 0);

     digitalWrite(startl, false);
   digitalWrite(stopl, true);
   //dmx_master.setChannelValue ( 2, 255);
   dmx_master.setChannelValue ( 3, 17);
   dmx_master.setChannelValue ( 9, 255);
   delay (12000);
   dmx_master.setChannelValue ( 3, 0);
   dmx_master.setChannelValue ( 9, 0);
   //dmx_master.setChannelValue ( 2, 0);
   strip.clear();
   strip.show();
   run_clock = false;
   digitalWrite(stopl, false);
   paused = false;
   digitalWrite(pausel, false);
    //tree_stop();
}

void tap_out_blue() {
  Serial1.print("bluetapout"); // Do this first, so that it outputs to OBS ASAP
  sendCommand(CMD_PLAY_WITHFOLDER, 0x0106); // Play tapout sound
  run_clock = false;
  clk_time = set_clk_tim;
  active = false;
  ready_b = false;
  ready_r = false;
  delay(10);
  digitalWrite(LED_R,LOW);
  delay(10);
  digitalWrite(LED_B,LOW);
  delay(10);
  waiting_for_players = true;
  strip.clear();
  strip.show();
    dmx_master.setChannelValue ( 2, 200); // Set the strobe
    dmx_master.setChannelValue ( 3, 17);  // Flash red 
    dmx_master.setChannelValue ( 9, 255);
    dmx_master.setChannelValue ( 13, 180);
    dmx_master.setChannelValue ( 16, 255);
    dmx_master.setChannelValue ( 20, 180);
  delay (4000);
    dmx_master.setChannelRange ( 2, 3, 0);
    dmx_master.setChannelValue ( 9, 0);
    dmx_master.setChannelValue ( 13, 0);
    dmx_master.setChannelValue ( 16, 0);
    dmx_master.setChannelValue ( 20, 0);
  digitalWrite(startl, false);
  digitalWrite(stopl, true);
   //dmx_master.setChannelValue ( 1, 255);
   dmx_master.setChannelValue ( 3, 53);
   dmx_master.setChannelValue ( 18, 255);
   delay (12000);
   dmx_master.setChannelValue ( 3, 0);
   dmx_master.setChannelValue ( 18, 0);
   //dmx_master.setChannelValue ( 2, 0);
   strip.clear();
   strip.show();
   run_clock = false;
   digitalWrite(stopl, false);
   paused = false;
   digitalWrite(pausel, false);
    //tree_stop();
}


void sendCommand(int8_t command, int16_t dat) 
{ 
delay(20); 
Send_buf[0] = 0x7e; //starting byte 
Send_buf[1] = 0xff; //version 
Send_buf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte 
Send_buf[3] = command; // 
Send_buf[4] = 0x00;//0x00 = no feedback, 0x01 = feedback 
Send_buf[5] = (int8_t)(dat >> 8);//datah 
Send_buf[6] = (int8_t)(dat); //datal 
Send_buf[7] = 0xef; //ending byte 
for(uint8_t i=0; i<8; i++)// 
{ 
  Serial3.write(Send_buf[i]) ;
 
} 
} 