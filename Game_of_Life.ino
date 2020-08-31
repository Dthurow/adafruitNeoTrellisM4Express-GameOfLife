#include "Adafruit_NeoTrellisM4.h"

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

boolean *lit_keys;
boolean *new_lit_keys;
boolean game_started;

void setup(){
  Serial.begin(115200);
    
  trellis.begin();
  trellis.setBrightness(80);

  game_started = false;

  lit_keys = new boolean[trellis.num_keys()];
  new_lit_keys = new boolean[trellis.num_keys()];
  
  setInitialGameState();
  
}
  
void loop() {

  trellis.tick();
  
  if (!game_started){
      handleGameSetup();
  }
  else{
    gameOfLife();
    checkGameRestart();
  }
  
  delay(10);
}

//if a key is pressed it'll light up, pressed again, turned off
//lets you setup initial state for game of life
void handleGameSetup(){
  
  while (trellis.available()){
    keypadEvent e = trellis.read();
        
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      
      int key = e.bit.KEY;  // shorthand for what was pressed
      Serial.print(key); Serial.println(" pressed");

      if (key == trellis.num_keys()-1){
        Serial.println("starting the game");
        trellis.setPixelColor(key, 0);
        lit_keys[key] = false;
        game_started = true;
        return;
      }
      else{
        lit_keys[key] = !lit_keys[key];
        if (lit_keys[key]) {
          trellis.setPixelColor(key, Wheel(random(255)));
        } else {
          trellis.setPixelColor(key, 0);
        }  
      }
      

    }
  }
}

//For a space that is 'populated':
      //Each cell with one or no neighbors dies, as if by solitude.
      //Each cell with four or more neighbors dies, as if by overpopulation.
      //Each cell with two or three neighbors survives.
//For a space that is 'empty' or 'unpopulated'
      //Each cell with three neighbors becomes populated.
void gameOfLife(){
  int totalAlive = 0;

  //loop thru and find all changes, record the new version of lit keys
  //in the new_lit_keys array. Can't update as we go because then the changes
  //we make for one key will affect the others. We have to figure out all updates
  //THEN apply the changes
  for (int i=0; i<trellis.num_keys(); i++) {

    
    //to get the light directly above/below, +/-8
    //also need to the left and right of directly above/below, so +/-7 and +/-9
    //to get the light directly left/right, +/-1
      int numNeighbors = ( lit_keys[getNeighborIndex(i,8)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,9)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,7)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,-8)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,-9)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,-7)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,1)] ? 1 : 0)
                          + ( lit_keys[getNeighborIndex(i,-1)]? 1 : 0); 
                          
      bool key = lit_keys[i];

      Serial.printf("Math for neighbors is: %d + %d + %d + %d + %d + %d + %d + %d\n", ( lit_keys[getNeighborIndex(i,8)] ? 1 : 0),
                           ( lit_keys[getNeighborIndex(i,9)] ? 1 : 0),
                           ( lit_keys[getNeighborIndex(i,7)] ? 1 : 0),
                           ( lit_keys[getNeighborIndex(i,-8)] ? 1 : 0),
                           ( lit_keys[getNeighborIndex(i,-9)] ? 1 : 0),
                           ( lit_keys[getNeighborIndex(i,-7)] ? 1 : 0),
                           ( lit_keys[getNeighborIndex(i,1)] ? 1 : 0),
                           (lit_keys[getNeighborIndex(i,-1)]? 1 : 0));

      
      if (numNeighbors > 0){
        Serial.print("Have neighbors: "); Serial.println(numNeighbors);  
      }
      
      if (lit_keys[i]){
        //'populated'
        if (numNeighbors < 2 || numNeighbors > 3){
          //dies from lonliness or overpopulation
          Serial.print("Key ");Serial.print(i);Serial.println(" dies");
          key = false;
        }
      }
      else{
        //unpopulated
        if (numNeighbors == 3){
          //becomes populated
          Serial.print("Key ");Serial.print(i);Serial.println(" became populated");
          key = true;
        }
      }
      new_lit_keys[i] = key;
  }

  for (int i = 0; i < trellis.num_keys(); i++){
    
    if (new_lit_keys[i] && !lit_keys[i]){
      //should be turned on
      trellis.setPixelColor(i, Wheel(random(255)));
    }
    else if (!new_lit_keys[i]) {
      //should be turned off
      trellis.setPixelColor(i, 0);
    }
    lit_keys[i] = new_lit_keys[i];
    
    if (lit_keys[i]){
      totalAlive++;
    }
  }
  
  
  Serial.print("total alive: ");Serial.println(totalAlive);
  if (totalAlive == 0){
    //game is ended, start over
    game_started = false;
    setInitialGameState();
  }
  
  delay(500);
  
}

//if user presses the last button, reset game of life so they can
//set new initial conditions
void checkGameRestart(){
  keypadEvent e = trellis.read();
        
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      int key = e.bit.KEY;  // shorthand for what was pressed
      Serial.print(key); Serial.println(" pressed");

      if (key == trellis.num_keys()-1){
        //restart the game
        game_started = false;
        setInitialGameState();
        
      }
    }
}

//All lights should be off except one, which will start the game
void setInitialGameState(){
  
  for (int i=0; i<trellis.num_keys()-1; i++) {
    lit_keys[i] = false;
    trellis.setPixelColor(i, 0);
  }
  
  lit_keys[trellis.num_keys()-1] = true;
  trellis.setPixelColor(trellis.num_keys()-1, Wheel(75));
}


int getNeighborIndex(int i, int neighbor_offset){
  //to make it loop around top/bottom, mod by 32 (32 total lights, indexed 0-31)
   //if it's a negative number, add 32 to it
  
  int neighbor = (i+neighbor_offset) % trellis.num_keys();
  if (neighbor < 0){
    neighbor += trellis.num_keys();
  }
  Serial.print("Neighbor of ");Serial.print(i);Serial.print(" is ");Serial.println(neighbor);
  return neighbor;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return trellis.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return trellis.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return trellis.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
