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
boolean inputFridge = false;  // variable that will control if the barcodes will subtract or increment things on the fridge [false - subtract | true - increment] 


int clockPin = 8;
int dataPin = 4;
int SCAN_ENTER = 0x5a;
int SCAN_BREAK = 0xf0;
int breakActive = 0;
byte dataValue;
byte scanCodes[10] = {0x45,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46}; 
char characters[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
char buffer[64] = {}; // This saves the characters (for now only numbers) 
int bufferPos = 0;

boolean logging = true;

void setup() {
  if(logging) Serial.println("-= Starting setup() function...");
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
  if(logging) Serial.println("-= End of setup() function.");
}







void loop() {
  if(logging) Serial.println("-= loop() initial...");
  if(logging) Serial.println("-= loop() - GLOBAL VARIABLES -");
  if(logging) Serial.println("-= loop() - BreakActive: ");
  if(logging) Serial.print(breakActive);
  if(logging) Serial.println("-= loop() - dataValue: ");
  if(logging) Serial.print(dataValue);
  if(logging) Serial.println("-= loop() - bufferPos: ");
  if(logging) Serial.print(bufferPos);
  if(logging) Serial.println("-= loop() - ---------------- -");
  
  if(logging) Serial.println("-= loop() - before dataRead()");
  dataValue = dataRead();
  if(logging) Serial.println("-= loop() - after dataRead()");
  if(logging) Serial.println("-= dataValue: ");
  if(logging) Serial.print(dataValue);
                                             
  // If there is a break code, skip the next byte                        
  if (dataValue != SCAN_BREAK && dataValue != SCAN_ENTER) {
    if(logging) Serial.println("-= loop() - dataValue appears to be a normal number...");
    if(breakActive == 1){
      if(logging) Serial.println("-= loop() - ...but it's not - BREAK char before this one. Ignoring...");
      breakActive = 0;
    }
    else{
      if(logging) Serial.println("-= loop() - ... and it really is! Converting HEX to DEC...");
      for (int i = 0; i < sizeof(characters); i++) {                                                                 
      if(scanCodes[i] == dataValue){                                                                                          
        buffer[bufferPos] = characters[i];
        if(logging) Serial.println("-= loop() - New char saved on buffer position ");
        if(logging) Serial.print(bufferPos);
        if(logging) Serial.print(". Char saved: ");
        if(logging) Serial.print(characters[i]);
        bufferPos++;
        if(logging) Serial.println("-= loop() - bufferPos incremented. New value: ");
        if(logging) Serial.print(bufferPos);
        break;                                                       
        }                                                                    
      }  
    }
  }
  else if (dataValue == SCAN_ENTER){
    if(logging) Serial.println("-= loop() - ENTER detected.");  
    breakActive = 0;
    if(logging) Serial.println("-= loop() - Lets print the buffer as the scan finished its job.");
    Serial.print("\nbuffer: ");          
    // Read the buffer                                                   
    int i=0;                                                             
    if (buffer[i] != 0) {                                                
      while(buffer[i] != 0) {                                            
        Serial.print( buffer[i] );                                       
        buffer[i] = 0;                                                   
        i++;                                                             
      }                                                                  
    }                        
    bufferPos = 0;                      
  }            
  else if (dataValue == SCAN_BREAK){
    if(logging) Serial.println("-= loop() - BREAK detected. Flag will be set to true.");
    breakActive = 1;                             
  }
  dataValue = 0;    
}



/**
 * Interrupt function for when the user triggers the switch button
 */
void InterruptChangeState(){
  //Flips leds and boolean for inputFridge value
  digitalWrite(RedLedPin, !digitalRead(RedLedPin));
  digitalWrite(GreenLedPin, !digitalRead(GreenLedPin));
  inputFridge^=true;
}


/**
 * Function that will return the value read by the barcode scanner
 */
int dataRead() {
  if(logging) Serial.println("-= inside dataRead()");
  byte val = 0;
  
  while (digitalRead(clockPin));  // Wait for LOW - Clock is high when barcode is idle.                                                                                 
  // Skip start state and start bit                                      
  if(logging) Serial.println("-= inside dataRead() - LOW");
  
  while (!digitalRead(clockPin)); // Wait for HIGH.
  if(logging) Serial.println("-= inside dataRead() - HIGH");
                        
  while (digitalRead(clockPin));  // Wait for LOW.       
  if(logging) Serial.println("-= inside dataRead() - LOW");

  
  for (int offset = 0; offset < 8; offset++) {                           
    while (digitalRead(clockPin));         // Wait for LOW               
    val |= digitalRead(dataPin) << offset; // Add to byte                
    while (!digitalRead(clockPin));        // Wait for HIGH              
  }                                                                      
  // Skipping parity and stop bits down here.                            
  while (digitalRead(clockPin));           // Wait for LOW.
  if(logging) Serial.println("-= inside dataRead() - LOW");
                
  while (!digitalRead(clockPin));          // Wait for HIGH.
  if(logging) Serial.println("-= inside dataRead() - HIGH");
               
  while (digitalRead(clockPin));           // Wait for LOW.
  if(logging) Serial.println("-= inside dataRead() - LOW");
                
  while (!digitalRead(clockPin));          // Wait for HIGH.
  if(logging) Serial.println("-= inside dataRead() - HIGH");

  if(logging) Serial.println("-= inside dataRead() - value to be returned: ");
  if(logging) Serial.print(val);   
  return val;
}
