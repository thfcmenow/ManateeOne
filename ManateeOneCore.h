// game engine variables (not meant to be changed)
boolean block =       false; // initial boolean block for collision detection
boolean noback =      false; // no back - after a collision do not blank the space behind you
String blank =        " "; // blank field for movement

// change the following as approriate ----------------------------------------------------
boolean useLCD =      true;   // use an LCD shield
boolean debug =       false;   // serial write various stuffs

// used for calcuating boundaries on a 16x2 screen
int boundaryLeft =    1;
int boundaryRight =   15;
int boundaryUp =      0;
int boundaryDown =    1;

// track score
int score =           0;

// define mac number of sprite and objects
const int manSmax = 4;
const int manPmax = 6;

// struts used for game engine (not meant to be changed) -----------------------
// characters display as sprites with more advanced characteristics
struct CHARACTER {
   String sName;      // name of character
   String charUsed;  // actual character used for sprite
   int xpos;        // xpos
   int ypos;        // ypos
   int charHealth;  // health of character
};

// usually scenery like a tree - block the path
struct POBJECT {
   String objectName;      // name of character
   String charUsed; // actual character used
   int block;       // do you run over this or are you blocked?
   int xpos;        // xpos
   int ypos;        // ypos
   int collect;     // can you collect this?
   int score;       // if you can collect then the score it gives you
   int collected=0; // collected
};

// define number of sprites and objects
CHARACTER sprite[manSmax]; 
POBJECT plainObject[manPmax];

// bytes for custom characters 
byte def[8] = {B00100,B01110,B10101,B00100,B01110,B10101,B00100,B00100}; // default byte as I don't know how to pass bytes :/
byte tree[8] = {B00100,B01110,B10101,B00100,B01110,B10101,B00100,B00100}; // a tree
byte dollar[8] = {B00100,B11111,B10100,B11111,B00101,B11111,B00100,B00100}; // $ symbol

// use your own output library - default is lcd
void render(int xpos, int ypos, String charUsed, int bytenum = -1){
  if (useLCD==true)
  {
    // lcd ---
     lcd.setCursor(xpos,ypos);
     // check for custom character
     if (bytenum > -1)
  {
      lcd.write(byte(bytenum));
  } else
     {lcd.print(charUsed); }
  }
}

// slot us a serial printer and only displays if debug is true
void slog(int variable, boolean bvariable, String message)
{
   Serial.println(variable + String("---") + bvariable + String("----") + message);
}

// setup and display a sprite
void setupSprite(int slot,String charUsed, String spriteName, int x = 0, int y = 0, int cHealth = 10)
{
  sprite[slot].charUsed = charUsed; sprite[slot].sName = spriteName; sprite[slot].xpos = x;
  sprite[slot].ypos = y; sprite[slot].charHealth = cHealth; render(x,y,charUsed);
}

// setup an object that is typically a non moveable object
void setupPlainObject(int slot, String charUsed, int blockme, String objectName, int x = 0, int y = 0, int bnum = -1, byte data[8] = def,int collect = 0, int lscore = 0)
{
  plainObject[slot].charUsed = charUsed;
  plainObject[slot].block = blockme;
  plainObject[slot].objectName = objectName;
  plainObject[slot].xpos = x;
  plainObject[slot].ypos = y;
  plainObject[slot].collect = collect;
  plainObject[slot].score = lscore;
  plainObject[slot].collected = 0;

  // if this is a custom character
  if (bnum>-1)
  {
    lcd.createChar(bnum, data);
    render(x,y,blank,bnum);
  } else
  {
    render(x,y,charUsed);
  }
}

// support for moveSprite
int processMovement(int n, int a, int slot, int way,int direct)
  {
    switch (direct)
      {
        case 0:   if (plainObject[n].ypos == sprite[slot].ypos) {
        if ((a+1 == plainObject[n].xpos) && (plainObject[n].collected==0)){noback=true;}}
        break;
        case 1:   if (plainObject[n].ypos == sprite[slot].ypos) {   
        if ((a-1 == plainObject[n].xpos) && (plainObject[n].collected==0)){noback=true;}} 
        break;
        case 2:   if (plainObject[n].xpos == sprite[slot].xpos) {      
        if ((a+1 == plainObject[n].ypos) && (plainObject[n].collected==0)){noback=true;}}
        break;
        case 3:   if (plainObject[n].xpos == sprite[slot].xpos) {
        if ((a-1 == plainObject[n].ypos) && (plainObject[n].collected==0)){noback=true;}}
        break;          
      }
  
    // process x (left/right)- our a matches left or right
    if ((direct==0) || (direct==1))
    {
      if ((a == plainObject[n].xpos) && (sprite[slot].ypos == plainObject[n].ypos) && (plainObject[n].objectName!=""))           // if we run over an object on x (and the same y axis)
      {
        if (plainObject[n].collect == 1 && plainObject[n].collected == 0) // if this is a collectable and hasnt been collected increment score
        {
          block = false;
          score = plainObject[n].score + score;
          plainObject[n].collected = 1;
        } 
        if (plainObject[n].collect == 0)
        {
          block = true;   // if this is not a collectable then its a roadblock
        }
      }
    }

    // process y (up/down)- our a matches up and down
    if ((direct==2) || (direct==3))
    { 
    if ((a == plainObject[n].ypos) && (sprite[slot].xpos == plainObject[n].xpos) && (plainObject[n].objectName!=""))           // if we run over an object on y (and the same x axis)
      {
        if (plainObject[n].collect == 1 && plainObject[n].collected == 0) // if this is a collectable and hasnt been collected increment score
        {
          block = false;
          score = plainObject[n].score + score;
          plainObject[n].collected = 1;
        } 
        if (plainObject[n].collect == 0)
        {
          block = true;   // if this is not a collectable then its a roadblock
        }
      }
    }
 }

// provide movement to a sprite
void moveSprite(int slot, int direct = 0, int distance = 1, int speeds = 0, int trail = 0)
{
  int newx = sprite[slot].xpos;
  int newy = sprite[slot].ypos;

  // left ------------------------------------------------------------------------------------------------------
  if (direct==0 && sprite[slot].xpos != boundaryLeft){
   for( int a = sprite[slot].xpos; (a > (sprite[slot].xpos-distance) && (a > boundaryLeft-1)); a = a - 1 ) { 
      // cant move to the following area - object in way
        for (int n=0; n < manPmax; n++){
         processMovement(n,a,slot,0,direct); // process movement left
         }
     if (block==true){break;} // if object in way then break out of loop cant move there
     if (noback == false) { render(a+1, sprite[slot].ypos,blank); } // as long as noback=false, clear block along trail
     noback = false;
     render(a, sprite[slot].ypos, sprite[slot].charUsed); // generate new position sprite
     delay(50);
     newx = a;
   }}

  // right ------------------------------------------------------------------------------------------------------
  // slog(sprite[slot].xpos,0,"what is players x pos");
  if (direct==1 && sprite[slot].xpos != boundaryRight){
   for( int a = sprite[slot].xpos; (a < (sprite[slot].xpos+distance) && (a < boundaryRight+1)); a = a + 1 ) {
      for (int n=0; n < manPmax; n++){
          processMovement(n,a,slot,1,direct); // process movement right
        }                                                                     
     
     if (block==true){break;}
     if (noback == false){render(a-1, sprite[slot].ypos,blank);}
     noback = false;
     render(a, sprite[slot].ypos,sprite[slot].charUsed);
     delay(50);
     newx = a;
  }}

  // up ------------------------------------------------------------------------------------------------------
  if (direct==2 && sprite[slot].ypos != boundaryUp){
   for( int a = sprite[slot].ypos; (a < (sprite[slot].ypos+distance) && (a > boundaryUp-1)); a = a - 1 ) {
      for (int n=0; n < manPmax; n++){
       processMovement(n,a,slot,0,direct); // process movement up
        }
     if (block==true){break;}
     if (noback == false){render(sprite[slot].xpos,a+1,blank);}
     noback = false;
     render(sprite[slot].xpos,a,sprite[slot].charUsed);
     delay(50);
     newy = a;
   }}

  // down ------------------------------------------------------------------------------------------------------
  if (direct==3 && sprite[slot].ypos != boundaryDown ){
   for( int a = sprite[slot].ypos; (a > (sprite[slot].ypos-distance) && (a < boundaryDown+1)); a = a + 1 ) {
      for (int n=0; n < manPmax; n++){
       processMovement(n,a,slot,1,direct); // process movement down
        }
      if (block==true){break;}
      if (noback == false){render(sprite[slot].xpos,a-1,blank);}
      noback = false;
      render(sprite[slot].xpos,a,sprite[slot].charUsed);
      delay(50);
      newy = a;
   }}


  // assign new position of sprite to array struct
  sprite[slot].xpos = newx;
  sprite[slot].ypos = newy;

  // reset block
  block = false;
 }
 
 // USAGE EXAMPLE -------------------------
 /*
 void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // obj num,char,block u?[0-1],obj name,x,y,custom byte [0-1-2--],byte id,collect?[0-1],score amt[0-1-2--]
  setupPlainObject(0,"^",1,"Tree",3,0,0,tree);
  setupPlainObject(1,"^",1,"Tree2",12,1,0,tree);
  setupPlainObject(2,"-",0,"Dollar",14,1,1,dollar,1,2);
  setupPlainObject(3,"-",0,"Dollar2",5,0,1,dollar,1,2);
  
  // sprite num, char, sprite name,x,y
  setupSprite(0,"*","Snowflake",7,0);

  // setup scoreboard to the left
  setupPlainObject(4,"0",1,"score0",0,0);
  setupPlainObject(5,"0",1,"score1",0,1);
}

void loop() {
    if ((digitalRead(8) == HIGH) && (digitalRead(9) == HIGH)){  
    } else 
    {
      // left
      if ((digitalRead(8) != HIGH) && (digitalRead(9) == HIGH)){ 
      delay(100);   
      // move sprite0, left, 4 spaces, 0 speed, 0 trail
      // direction 0=left, 1=right
        moveSprite(0,0,2,0,0);
      };

      // right
       if ((digitalRead(9) != HIGH) && (digitalRead(8) == HIGH)){   
      delay(100);     
      moveSprite(0,1,2,0,0);
      };
    };

      //up
     if (digitalRead(A4) != HIGH) {
     delay(100);
     moveSprite(0,2,2,0,0);
     }

     //down
     if (digitalRead(A1) != HIGH) {
     delay(100);
     moveSprite(0,3,2,0,0);
     }     

    // update score
    render(0,1,String(score));
}
*/
  
 

