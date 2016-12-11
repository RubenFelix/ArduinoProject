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
int clockValue = 0; 
byte dataValue;
byte scanCodes[10] = {0x45,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46}; 
char characters[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int quantityCodes = 10;
char buffer[64] = {}; // This saves the characters (for now only numbers) 
int bufferPos = 0; 
int bufferLength = 64;


void setup() {
  //output mode for the LED pins
  pinMode(GreenLedPin, OUTPUT);
  pinMode(RedLedPin, OUTPUT);
  pinMode(YellowLedPin, OUTPUT);
  //input mode for data and clock pin of the barcode scanner
  pinMode(dataPin, INPUT);                                               
  pinMode(clockPin, INPUT);

  //Red light on, green light off indicating that products will be subtracted by default
  digitalWrite(RedLedPin, HIGH);
  digitalWrite(GreenLedPin, LOW);
  digitalWrite(YellowLedPin, LOW);

  // Attach an interrupt to the second ISR vector, which corresponds to the PIN number 3 - switch button
  attachInterrupt(1, ChangeState, RISING);

  //begin serial interface
  Serial.begin(9600);                                                    
}

void loop() {
  
  dataValue = dataRead();                                                
  // If there is a break code, skip the next byte                        
  if (dataValue == SCAN_BREAK) {                                         
    breakActive = 1;                                                     
  }                                                                      
  // Translate the scan codes to numbers                                 
  // If there is a match, store it to the buffer                         
  for (int i = 0; i < quantityCodes; i++) {                              
    byte temp = scanCodes[i];                                            
    if(temp == dataValue){                                               
      if(!breakActive == 1){                                             
        buffer[bufferPos] = characters[i];                               
        bufferPos++;                                                     
      }                                                                  
    }                                                                    
  }                                                                      
  //Serial.print('*'); // Output an asterix for every byte               
  // Print the buffer if SCAN_ENTER is pressed.                          
  if(dataValue == SCAN_ENTER){                                           
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
    Serial.println(" [Enter]");                                          
    bufferPos = 0;                                                        
  }                                                                      
  // Reset the SCAN_BREAK state if the byte was a normal one             
  if(dataValue != SCAN_BREAK){                                           
    breakActive = 0;                                                     
  }                                                                      
  dataValue = 0;    
                                                     
}


/**
 * Interrupt function for when the user triggers the switch button
 */
void ChangeState(){
  //Flips leds and boolean for inputFridge value
  digitalWrite(RedLedPin, !digitalRead(RedLedPin));
  digitalWrite(GreenLedPin, !digitalRead(GreenLedPin));
  inputFridge^=true;
}


/**
 * Function that will return the value read by the barcode scanner
 */
int dataRead() {
  byte val = 0;                                                          
  while (digitalRead(clockPin));  // Wait for LOW - Clocl is high when barcode is idle.                       
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
  return val;                                                            
}