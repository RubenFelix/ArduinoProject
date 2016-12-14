/*
Barcode Scanner                                                        
  This code reads the input from a ps/2 keyboard or keyboard-like        
  device (e.g. a barcode scanner), translates the scan-codes into        
  numbers (only numbers from 0 to 9 can be used at the moment)           
  It is nowhere near a complete implementation of the ps/2 protocol,     
  but it should give you a starting point.                               
  mys .// Benjamin Maus ( benjamin.maus <at> allesblinkt.com )          
  2007                                                                   
*/

const int GreenLedPin = 6;  // green led pin number - green tells the user that the barcode will scan and increment that product on the database
const int RedLedPin = 7;  // red led pin number - on the other hand, red means that product read by the barcode will be taken from the database
const int YellowLedPin = 9; // yellow pin for error warning
boolean inputFridge = true;  // variable that will control if the barcodes will subtract or increment things on the fridge [false - subtract | true - increment] 
int cenas = 0;

int clockPin = 8;
int dataPin = 4;
int SCAN_ENTER = 0x5a;
int SCAN_BREAK = 0xf0;
int breakActive = 0;
byte scanCodes[10] = {0x45,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46}; 
char characters[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
char buffer[64] = {}; // This saves the characters (for now only numbers) 
int bufferPos = 0;

boolean logging = true;

void setup() {

  //output mode for the LED pins
  pinMode(GreenLedPin, OUTPUT);
  pinMode(RedLedPin, OUTPUT);
  pinMode(YellowLedPin, OUTPUT);
  //input mode for data and clock pin of the barcode scanner
  pinMode(dataPin, INPUT_PULLUP);                                               
  pinMode(clockPin, INPUT_PULLUP);

  //Red light on, green light off indicating that products will be subtracted by default
  digitalWrite(RedLedPin, HIGH);
  digitalWrite(GreenLedPin, LOW);
  digitalWrite(YellowLedPin, LOW);

  // Attach an interrupt to the second ISR vector, which corresponds to the PIN number 3 - switch button
  attachInterrupt(1, InterruptChangeState, RISING);

  //begin serial interface
  Serial.begin(9600);

}







void loop() {
  if(logging) Serial.println("-= loop() - before dataRead()");
  dataRead();
  if(logging) Serial.println("-= loop() - after dataRead()");
  if(logging) Serial.println("-= loop() - before printBuffer()");
  printBuffer();
  if(logging) Serial.println("-= loop() - after printBuffer()");
  if(logging) Serial.println("-= loop() - before cleanBuffer()");
  cleanBuffer();
  if(logging) Serial.println("-= loop() - after cleanBuffer()");
}



/**
 * Interrupt function for when the user triggers the switch button
 */
void InterruptChangeState(){
  //Flips leds and boolean for inputFridge value
  digitalWrite(RedLedPin, !digitalRead(RedLedPin));
  digitalWrite(GreenLedPin, !digitalRead(GreenLedPin));
  inputFridge^=true;
  Serial.println("");
  Serial.println(cenas);
}


void printBuffer(){
  
  for (int i = 0; i < sizeof(buffer); i++) {                                                                 
    if(buffer[i] != 0)   Serial.print( buffer[i] );
  }
  Serial.println("");
  Serial.println(cenas);
  cenas++;
}

void cleanBuffer(){
  for (int i = 0; i < sizeof(buffer); i++) {                                                                 
    buffer[i] = 0;
  }     
}

/**
 * Function that will return the value read by the barcode scanner
 */
void dataRead() {
  byte val;
  if(logging) Serial.println("-= inside dataRead() - Before WHILE");
  while(val != SCAN_ENTER){
    val = 0;
    while (digitalRead(clockPin));  // Wait for LOW - Clock is high when barcode is idle.                                                                                 
    // Skip start state and start bit                                      
    while (!digitalRead(clockPin)); // Wait for HIGH.                
    while (digitalRead(clockPin));  // Wait for LOW.       
 
    for (int offset = 0; offset < 8; offset++) {                           
      while (digitalRead(clockPin));         // Wait for LOW               
      val |= digitalRead(dataPin) << offset; // Add to byte                
      while (!digitalRead(clockPin));        // Wait for HIGH              
    }                
                                                          
    // Skipping parity and stop bits down here.                            
    while (digitalRead(clockPin));           // Wait for LOW.         
    while (!digitalRead(clockPin));          // Wait for HIGH.        
    while (digitalRead(clockPin));           // Wait for LOW.        
    while (!digitalRead(clockPin));          // Wait for HIGH.
    
    if (val != SCAN_BREAK && val != SCAN_ENTER) {
      if(breakActive == 1){
        breakActive = 0;
      }
      else{
        for (int i = 0; i < sizeof(characters); i++) {                                                                 
        if(scanCodes[i] == val){                                                                                          
          buffer[bufferPos] = characters[i];
          bufferPos++;
          break;                                                       
          }                                                                    
        }  
      }
    }          
    else if (val == SCAN_BREAK){
      breakActive = 1;                             
    }
    else if (val == SCAN_ENTER){
      breakActive = 0;                             
    }  
  }
  if(logging) Serial.println("-= loop() - ENTER detected. Leaving dataRead()...");  
  breakActive = 0;  
  bufferPos = 0;                      
}
