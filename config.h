//Hardware Select
#define LILYGO_WATCH_2020_V3             //To use T-Watch2020 V3, please uncomment this line

// NOT SUPPORT ...
//// #define LILYGO_WATCH_BLOCK
// NOT SUPPORT ...

// => Function select
#define LILYGO_WATCH_LVGL                   //To use LVGL, you need to enable the macro LVGL

#include <LilyGoWatch.h>
//ER: 11:The smartwatch application shall time stamp the session with a timing delay of no less than 100 milliseconds.
//ER: 14: The smartwatch application should allow the therapist to easily identify the most-relevant behavior of the patient based upon the color and description in the interface.
//ER: 15: The smartwatch application shall allow therapists to interact with up to four icons associated with specific behaviors for timestamping during a session.




/*
  
  ECE Capstone 490 - 491 
  Year 2023 Spring/Fall Semester
  University of Kentucky
  Student: Iran Rodriguez F
  Dual Major: Electrical and Computer Engineer
  Sponsor: Dr. Cheung Sen-Cing
  Project: Iran Rodriguez (Thera-log App),Dylan Bragg (Camera System), GraceAnn Rimel (WebServer) Keagan Hill (File Conversion)

  Description: Theralogv7_1_5CamControlUSB_FINAL package contains 3 files. 
  1st file is a folder in which contains the Software Application. The Software Application
  called Thera-log App contains the functionalities of start a session, 
  timestamp key behevarials, end the session, control camera system and send the saved data 
  thru a USB from the T-Watch-2020 V3.
  2nd file is a server .py python script in which contains the script to control the camera system as well
  to received the timestamp .txt file from the smartwatch.
  Engineer Consideration: 
  
  Integrated Development  Environment (IDE) is Arduino 3, TTGO-TWatch Board (View ArduinoIDE Folder).
  
  -- Possible Modification Considerations --
  setup(): Modify/View Serial Rate Frequency --> 115200
  Update Key Behavioral where it starts
  //********************MODIFY Start**************************
  //********************MODIFY End****************************

  WiFi SSID, Password, ServerPort and IP Address (NOTE: WiFi functionalities disable due to camera system and smartwatch Compatabilities)
  //********************MODIFY Start**************************
  //********************MODIFY End**************************

  Comments: The Thera-Log App was developed within a 3 months period. The code will require a many improvements as well additional
  functionalities. For a 1st time demostration, the software has been executed and tested. Individual functionalities worked and
  no errors and issues were find. For future improvements, I will suggest to review the code and separete function into different
  pages.
*/