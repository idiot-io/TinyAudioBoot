
/*-----------------------------------------------------------------------------------------------------------------

   smooth light hart beat with software PWM
   
   Revision History:
   V1.0 2017 02 06 ChrisMicro, initial version

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   It is mandatory to keep the list of outhors
   
---------------------------------------------------------------------------------------------------------------*/
#define LEDPIN 1

void setup()
{
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  
  uint8_t n;
  uint8_t brightness;
  uint8_t count;
  int8_t  signum;
  
  while (1)
  {
    if (n == 0)
    {
      digitalWrite(LEDPIN, HIGH);   // LED on
      
      count--;
      // check delay counter
      if (count == 0)
      {
        brightness += signum;
        
        if (brightness == 255)
        {
          delay(500);
          signum *= -1;
        }
        
        if (brightness == 0)
        {
          digitalWrite(LEDPIN, LOW);
          delay(500);
          signum *= -1;
        }
        
        count = 10; // hart beat slowness
      }
    }
    
    if (n == brightness ) digitalWrite(LEDPIN, LOW); // LED off
    n++;

  }
}
