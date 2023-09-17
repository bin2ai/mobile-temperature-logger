#include "serial_handler.h"

String inputString = ""; // Initialize a string to hold incoming data

String check_string()
{
  if (Serial.available() > 0)
  {
    Serial.println("Data available");
    char c = Serial.read();
    Serial.print("Char: ");
    Serial.println(c);
    Serial.println(inputString);

    if (c == '\r' || c == '\n')
    {
      Serial.print("Newline received");
      // If a newline character is received, process the complete inputString
      String receivedString = inputString; // Create a copy of inputString to return
      inputString = "";                    // Reset the inputString for the next input
      return receivedString;
    }
    else
    {
      // Otherwise, append the character to inputString
      inputString += c;
    }
  }
  else
  {
    // Serial.println("No data");
    //
    return ""; // If no complete line has been received, return an empty string
  }
  return "";
}
