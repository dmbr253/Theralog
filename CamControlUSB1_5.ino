//********************
//READ config.h 
//********************
//v6-Save file with correct structure in milliseconds for precision
#include "config.h" //Interaction with the Hardaware of the smartwach
#include <Ticker.h>//Call timer library
#include <FS.h>  //the SPIFFS library for file operations. Basically to save files.****
#include <WiFi.h> //WiFi library to connect to WiFi, final was demo done connecting to USB port 
#include <HTTPClient.h> //Create communication with Server clients
#include <TTGO.h> //TTGO library to use smartwatch buttons, and button actions, custome screen,create screens, etc..

//********************MODIFY Start**************************
//Note: WiFi functionalities when pressing does not work, you have to uncomment inside the 
//WiFi & server, currently wifi and server are not functionable due to smartwatch board and camera board bandwidth compatibilities.
const char *ssid = "THERALOG";   // Your WiFi SSID, MiniPC Hostpot
const char *password = "16rA9605";// Your WiFi password, MiniPC Hostpot
const char *serverAddress = "10.47.141.200";  //"192.168.1.240";  // Replace with your server's IP address
int serverPort = 8000;//Server port
//********************MODIFY End**************************
bool wifiConnected = false;
//Sever Info, connecting to WiFi and server are not functionable due to smartwatch board and Camera System board compatabilities. 
lv_obj_t *wifi_status_label;         // Label to display Wi-Fi status
lv_obj_t *TimestampingWarningLabel;  //define label to display if try to end session when a key behavior is still running
// Connect to the server
WiFiClient client;  //Socket server define global to start and make it stop anywhere

//GLOBAL variables are defines for BUTTONS
TTGOClass *ttgo; //Define ttgo hardware functionalities 
lv_obj_t *start_session_screen;  //session screen start_session_screen
lv_obj_t *key_behavior_screen;  //key behavior screen key_behavior_screen
lv_obj_t *control_session_screen; //control session screen control_session_screen
lv_obj_t *ssbtn;              //Start session button ssbtn
lv_obj_t *key_behavior_btn1;  //button 1 for behavior 1
lv_obj_t *key_behavior_btn2;  //button 2 for behavior 2
lv_obj_t *key_behavior_btn3;  //button 3 for behavior 3
lv_obj_t *key_behavior_btn4;  //button 4 for behavior 4
lv_obj_t *end_session_btn;    //end session button
lv_obj_t *wifi_btn;           //Start wifi button

//Variables are define for buttons and title labels
lv_obj_t *btn1_label;         //label for button 1
lv_obj_t *btn2_label;         //label for button 2
lv_obj_t *btn3_label;         //label for button 3
lv_obj_t *btn4_label;         //label for button 4
lv_obj_t *end_session_label;  //label for end session button

//********************MODIFY Start**************************
//Modified based on the Key-behavior for clinic
const char *label1_title = "Smiling"; //Modify inside double " " to prefered behavior
const char *label2_title = "Vocalization"; //Modify inside double " " to prefered behavior
const char *label3_title = "AB"; //Modify inside double " " to prefered behavior
const char *label4_title = "SIB"; //Modify inside double " " to prefered behavior
const char *end_session_title = "End Session"; //Modify inside double " " to prefered behavior
//********************MODIFY End**************************

//TIMER for timestamping behavior 
Ticker sessionTimer; //Define variable for timer
bool timerRunning = false; //Define variable to keep track of main timer running
unsigned long sessionStartTime = 0;  //Define veriable in which start time of session will be record here
unsigned long lastPrintTime = 0;  //Define veriable in which end time of session will be record here


//Define datatype variables to keep track of each key behavior timestamping time
bool recording = false; //Tracks if is or not recording for behavioral 1
unsigned long recordedStartTime = 0; //Record start time for behavior 1
unsigned long recordedEndTime = 0; //Record end time for behavior 1
//Button Behavior 2
bool recording2 = false; //Tracks if is or not recording for behavioral 2
unsigned long recordedStartTime2 = 0; //Record start time for behavior 2
unsigned long recordedEndTime2 = 0; //Record end time for behavior 2
//Button Behavior 3
bool recording3 = false; //Tracks if is or not recording for behavioral 3
unsigned long recordedStartTime3 = 0; //Record start time for behavior 3
unsigned long recordedEndTime3 = 0; //Record end time for behavior 3
//Button Behavior 4
bool recording4 = false; //Tracks if is or not recording for behavioral 4
unsigned long recordedStartTime4 = 0; //Record start time for behavior 4
unsigned long recordedEndTime4 = 0; //Record end time for behavior 4

//Define datatype variable to track file open and data gathering
fs::File dataFile;  //File datatype and save file to dataFile in smartwatch
bool isFileClose = false; //keeps track file open
bool isFileOpen = false; //keeps track file close
//bool isSessionTimerOrder = false; //?

/////////////////////////////////////////////////////
//UpdateWifiStatusText() is called to update the label 
//for connected or not connected WiFi. It checks in the 
//loop() function every 5 seconds. But also checks when
//pressing the connect to wifi button. Currently,
//the button is not executing the function that allows
//to read button pressed/action. 
/////////////////////////////////////////////////////
void updateWifiStatusText() {
  // Update the text based on the Wi-Fi connection status
  const char *wifiStatusText = (WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected";
  lv_label_set_text(wifi_status_label, wifiStatusText);
}
/////////////////////////////////////////////////////
//sendRequestToServer() function is call when 
//Save/Exit button is pressed.
//Wifi must be connected, and server running on from
//package version file. Update
//the server address and port to requirements.
//Once connected, a file will send, only when the
//Save/Exit button is pressed. Wi-Fi must be connected
//and be on the same server.
/////////////////////////////////////////////////////
void sendRequestToServer() {

  if (!client.connect(serverAddress, serverPort)) {
    Serial.println("Failed to connect to server");
    return;
  }

  // Send the file name to the server
  client.print("recorded_times.txt");

  // Open the file for reading
  //File file = SD.open("recorded_times.txt", FILE_READ);
  dataFile = SPIFFS.open("/recorded_times.txt", FILE_READ);  //remove fs::File since you declaring it as global, opens created file

  if (dataFile) {
    // Read and send the file data in chunks
    while (dataFile.available()) {
      uint8_t buffer[1024];
      size_t bytesRead = dataFile.read(buffer, sizeof(buffer));
      client.write(buffer, bytesRead);
    }
    dataFile.close();
    Serial.println("File sent successfully.");
  } else {
    Serial.println("Failed to open the file.");
  }

  // Close the connection
  client.stop();
}
/////////////////////////////////////////////////////
//sendSingleCharacter() function passes char datatype
//variable as a parameter. The function will send a 
//"Y" thru the USB serial port to start and "N" to
//stop the camera recording. Currently, this is the
//functionality been use for system demostration 
//integration for controlling the cameras.
/////////////////////////////////////////////////////
void sendSingleCharacter(char character) {
  Serial.write(character);
  Serial.println();  // Add a newline character at the end if needed
}
/////////////////////////////////////////////////////
//sendFileOverUSB() function send the data thru
//USB serial port by providing an aknoledgement to the
//server that was included in this file version
//package. It tell the server to start writing data
//in to a file on the server based on start and stop 
//transmission signal which is uses to tell data is
//over. This is guard to prevent junk data over serial
//to be writing in the file.
/////////////////////////////////////////////////////
void sendFileOverUSB() {

  // Open the file for reading
  dataFile = SPIFFS.open("/recorded_times.txt", FILE_READ);
  Serial.println("File opened successfully.");

  // Send a unique start signal
  Serial.println("START_OF_TRANSMISSION");

  if (dataFile) {
    // Read and send the file data in chunks
    while (dataFile.available()) {
      uint8_t buffer[1024];
      size_t bytesRead = dataFile.read(buffer, sizeof(buffer));
      Serial.write(buffer, bytesRead);  // Send raw binary data over Serial
      Serial.write("\n");
    }
    // Send a unique end signal
    Serial.println("END_OF_TRANSMISSION");

    dataFile.close();
    Serial.println("File sent successfully.");
  } else {
    Serial.println("Failed to open the file.");
  }
}
/////////////////////////////////////////////////////
//connectToWiFi() function is call when the connect to
//WiFi button is pressed. The function take the ssid
//and password to connect to the router. 
//Make sure the WiFi ssdi and password is upated
//correctly to connect succesffuly.
//Function connects to WiFi if not then will set 
//wifiConnected false not allowing to start the session
/////////////////////////////////////////////////////
void connectToWiFi() {
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  // Wait for connection
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 15) {  //loops 10xs to try to connect
    delay(1000);
    Serial.println(" Connecting to WiFi...");
    attemptCount++;  //increments by 1
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("Signal Strength (RSSI): ");
    Serial.println(WiFi.RSSI());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
    return;
  } else {
    Serial.println("Failed to connect to both Wi-Fi network");
    wifiConnected = false;
  }
}
/////////////////////////////////////////////////////
//disableButton() function is called when the 
//Wi-Fi is not connected. If Wi-Fi is not connnected
//disables the Start Session button.
/////////////////////////////////////////////////////
// Function to disable the button
void disableButton() {
  lv_btn_set_state(ssbtn, LV_BTN_STATE_DISABLED);
}
/////////////////////////////////////////////////////
//disableButton() function is called when the 
//Wi-Fi is not connected. If Wi-Fi is connnected
//enables the Start Session button.
/////////////////////////////////////////////////////
void enableButton() {
  lv_btn_set_state(ssbtn, LV_BTN_STATE_RELEASED);
}
/////////////////////////////////////////////////////
//printSessionTimer() tracks and prints the time of 
//the main session and not the key behavioral. 
//When the Start Session is pressed, this function
//timer will be on until the Save/Exit button is 
//pressed to stop the timer. Updating the global
//start and stop variable to record times of
//sessions.
/////////////////////////////////////////////////////
void printSessionTimer() {  //add it to print time
  if (timerRunning) {
    unsigned long currentTime = millis();
    if (currentTime - lastPrintTime >= 900) {
      // Print session timer every 900 milliseconds
      unsigned long sessionTimeInSeconds = (currentTime - sessionStartTime);

      //Serial.println("Session Timer: " + String(sessionTimeInSeconds) + " milliseconds");//un-comment to see data printing on serial IDE. !@# see symbol

      //Serial.println("Session Timer: " + String(currentTime - sessionStartTime) + " ms");
      lastPrintTime = currentTime;
    }
  }
}
/////////////////////////////////////////////////////
//recordTime#() corresponds to each key behavioral 
//and when key behavioral buttons are pressed, 
//the timer starts and stop recording. The recording
//time will be save into a temporary file called
//saveSessionData("B#"+ ...+ " ms") on the
//smartwatch until button is pressed again of that 
//initial pressed behavioral. The time send will be 
//formatted to milliseconds.
/////////////////////////////////////////////////////
void recordTime() {  //For Behaior 1
  if (timerRunning) {
    if (!recording) {
      recordedStartTime = (millis() - sessionStartTime);  //divide 1000 since time is in milliseconds
      recording = true;
      char labelText[32];
      sprintf(labelText, "Running\n %s", label1_title);  //calling sprintf convert it into a single string, <b></b> bold "R", %s is placeholder for label1_title, R is for Running for visual
      lv_label_set_text(btn1_label, labelText);
      saveSessionData("B1(" + String(label1_title) + "):" + String(recordedStartTime) + " ms ");  //writes behavior 1 start time to the file.
      Serial.println("B1:Recording start time: " + String(recordedStartTime) + " ms ");

    } else {
      unsigned long currentTime = millis() - sessionStartTime;
      recordedEndTime = currentTime;  // divide by 1000 since currentTime is in ms // = currentTime - recordedStartTime; //How long it took
      recording = false;
      lv_label_set_text(btn1_label, label1_title);
      saveSessionData("B1(" + String(label1_title) + "):" + String(currentTime) + " ms ");  //writes behavior 1 end time to the file.
      Serial.println("B1:Stopped recording. Recorded time: " + String(currentTime) + " ms ");
    }
  }
}
/////////////////////////////////////////////////////
//recordTime#() corresponds to each key behavioral 
//and when key behavioral buttons are pressed, 
//the timer starts and stop recording. The recording
//time will be save into a temporary file called
//saveSessionData("B#"+ ...+ " ms") on the
//smartwatch until button is pressed again of that 
//initial pressed behavioral. The time send will be 
//formatted to milliseconds.
/////////////////////////////////////////////////////
void recordTime2() {  //For Behavior 2
  if (timerRunning) {
    if (!recording2) {
      recordedStartTime2 = (millis() - sessionStartTime);
      recording2 = true;
      char labelText[32];
      sprintf(labelText, "Running\n %s", label2_title);  //calling sprintf convert it into a single string, <b></b> bold "R", %s is placeholder for label1_title, R is for Running for visual
      lv_label_set_text(btn2_label, labelText);
      saveSessionData("B2(" + String(label2_title) + "):" + String(recordedStartTime2) + " ms ");  //writes behavior 2 start time to the file.
      Serial.println("B2:Recording start time: " + String(recordedStartTime2) + " ms ");
    } else {
      unsigned long currentTime = millis() - sessionStartTime;
      recordedEndTime2 = currentTime;  //+ recordedStartTime2;
      recording2 = false;
      lv_label_set_text(btn2_label, label2_title);
      saveSessionData("B2(" + String(label2_title) + "):" + String(recordedEndTime2) + " ms ");  //writes behavior 2 end time to the file.
      Serial.println("B2:Stopped recording. Recorded time: " + String(recordedEndTime2) + " ms");
    }
  }
}
/////////////////////////////////////////////////////
//recordTime#() corresponds to each key behavioral 
//and when key behavioral buttons are pressed, 
//the timer starts and stop recording. The recording
//time will be save into a temporary file called
//saveSessionData("B#"+ ...+ " ms") on the
//smartwatch until button is pressed again of that 
//initial pressed behavioral. The time send will be 
//formatted to milliseconds.
/////////////////////////////////////////////////////
void recordTime3() {  //For Behavior 3
  if (timerRunning) {
    if (!recording3) {
      recordedStartTime3 = (millis() - sessionStartTime);
      recording3 = true;
      char labelText[32];
      sprintf(labelText, "Running\n %s", label3_title);  //calling sprintf convert it into a single string, <b></b> bold "R", %s is placeholder for label1_title, R is for Running for visual
      lv_label_set_text(btn3_label, labelText);
      saveSessionData("B3(" + String(label3_title) + "):" + String(recordedStartTime3) + " ms ");  //writes behavior 3 start time to the file.
      Serial.println("B3:Recording start time: " + String(recordedStartTime3) + " ms ");
    } else {
      unsigned long currentTime = millis() - sessionStartTime;
      recordedEndTime3 = currentTime;  // + recordedStartTime3;
      recording3 = false;
      lv_label_set_text(btn3_label, label3_title);
      saveSessionData("B3(" + String(label3_title) + "):" + String(recordedEndTime3) + " ms ");  //writes behavior 3 end time to the file.
      Serial.println("B3:Stopped recording. Recorded time: " + String(recordedEndTime3) + " ms ");
    }
  }
}
/////////////////////////////////////////////////////
//recordTime#() corresponds to each key behavioral 
//and when key behavioral buttons are pressed, 
//the timer starts and stop recording. The recording
//time will be save into a temporary file called
//saveSessionData("B#"+ ...+ " ms") on the
//smartwatch until button is pressed again of that 
//initial pressed behavioral. The time send will be 
//formatted to milliseconds.
/////////////////////////////////////////////////////
void recordTime4() {  //For Behavior 4
  if (timerRunning) {
    if (!recording4) {
      recordedStartTime4 = (millis() - sessionStartTime);
      recording4 = true;
      char labelText[32];
      sprintf(labelText, "Running\n %s", label4_title);  //calling sprintf convert it into a single string, <b></b> bold "R", %s is placeholder for label1_title, R is for Running for visual
      lv_label_set_text(btn4_label, labelText);
      saveSessionData("B4(" + String(label4_title) + "):" + String(recordedStartTime4) + " ms ");  //writes behavior 4 start time to the file.
      Serial.println("B4:Recording start time: " + String(recordedStartTime4) + " ms ");
    } else {
      unsigned long currentTime = millis() - sessionStartTime;
      recordedEndTime4 = currentTime;  //+ recordedStartTime4;
      recording4 = false;
      lv_label_set_text(btn4_label, label4_title);
      saveSessionData("B4(" + String(label4_title) + "):" + String(recordedEndTime4) + " ms ");  //writes behavior 4 end time to the file.
      Serial.println("B4:Stopped recording. Recorded time: " + String(recordedEndTime4) + " ms ");
    }
  }
}
/////////////////////////////////////////////////////
//save_exit_event_handler(lv_obj_t *obj, lv_event_t 
//event) function will pass the action result 
//of the button and based on that action it will 
//execute the code inside the if else statement.
//When the Save/Exit button is pressed will call
//multiple functions to control stop camera recording
//stop timer, save ending session time to a file
//sending file thru USB function and others.
/////////////////////////////////////////////////////
static void save_exit_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    sendSingleCharacter('N');//Send signal to miniPC
    sessionTimer.detach();                                                             //Detaches from the board clock
    timerRunning = false;                                                              //Make timer off
    saveSessionData("SET: " + String(lastPrintTime) + " ms");                          //sends start time in milli seconds to a file, divide by 1000 to see in seconds
    dataFile.close();                                                                  //File closes with the same data.
    Serial.println("File closed.");                                                    //Lets you knwo thru serial file was closed.
    Serial.println("Session End Time: " + String(lastPrintTime / 1000) + " seconds");  //Print End time


    lv_scr_load(start_session_screen);  //Load Start_session_Screen
    serialPrintFileContent();
    //sendRequestToServer();//No need to, using USB to send file
    sendFileOverUSB();
    deleteSessionFile();
  }
}
/////////////////////////////////////////////////////
//pause_resume_event_handler(lv_obj_t *obj, lv_event_t 
//event) function will pass the action result 
//of the button and based on that action it will 
//execute the code inside the if else statement.
//Due complexity of UI and functionalities, the 
//function stops the session timer of the smartwatch 
//and starts it again when pressed the Pause/Resume
//button.
/////////////////////////////////////////////////////
static void pause_resume_event_handler(lv_obj_t *obj, lv_event_t event) {
  //Pause
  //Resume
  if (event == LV_EVENT_CLICKED) {
    if (timerRunning) {
      timerRunning = false;  // Pause the session timer
    } else {
      timerRunning = true;  // Resume the session timer
      lv_scr_load(key_behavior_screen);
    }
  }
}
/////////////////////////////////////////////////////
//key_behavior_btn#_event_handler calls the 
//recordingTime#() function to track and record 
//specific key behavior time. This function relates
// like the save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void key_behavior_btn1_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {

    recordTime();
  }
}
/////////////////////////////////////////////////////
//key_behavior_btn#_event_handler calls the 
//recordingTime#() function to track and record 
//specific key behavior time. This function relates
// like the save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void key_behavior_btn2_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {

    recordTime2();
  }
}
/////////////////////////////////////////////////////
//key_behavior_btn#_event_handler calls the 
//recordingTime#() function to track and record 
//specific key behavior time. This function relates
// like the save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void key_behavior_btn3_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {

    recordTime3();
  }
}
/////////////////////////////////////////////////////
//key_behavior_btn#_event_handler calls the 
//recordingTime#() function to track and record 
//specific key behavior time. This function relates
// like the save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void key_behavior_btn4_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {

    recordTime4();
  }
}
/////////////////////////////////////////////////////
//wifi_event_handler(lv_obj_t *obj, lv_event_t event)
//function is called to check the WiFi connectavity 
//and disables and enables the Start Session button.
// This function relates like the 
//save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void wifi_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    connectToWiFi();
    // Depending on the WiFi connection status, enable or disable the start session button (security system.)
    if (wifiConnected) {
      enableButton();

    } else {
      disableButton();
    }
    //when click, connects to wifi and display a wifi check mark connection on top of the start_session_screen
  }
}
/////////////////////////////////////////////////////
//end_session_event_handler (lv_obj_t *obj, 
//lv_event_t event) is called when the 
//End Session button is pressed. Contains multiple
//functionalities like screen, buttons, styles,etc.
//This function relates like the 
//save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void end_session_event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    //Stops recording if therapist forgot to pressed on the button.
    if ((recording || recording2 || recording3 || recording4) == true)  //allows to stop any key behavior moment so it can be recorded
    {
      lv_label_set_text(TimestampingWarningLabel, "A Key Behavior still in Progress");  //initialize with a warning if a key behavior button is still in progress tracking time.


    } else {

      // Create a new screen (key_behavior_screen) and load it when the button is clicked
      control_session_screen = lv_obj_create(NULL, NULL);  //assign empty screen
      lv_scr_load(control_session_screen);                 //load key behavior screen


      // Create a style for the label and make the text bold
      static lv_style_t texttitle2_style;
      lv_style_init(&texttitle2_style);
      lv_style_set_text_color(&texttitle2_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
      lv_style_set_text_letter_space(&texttitle2_style, LV_STATE_DEFAULT, 3);
      lv_style_set_text_line_space(&texttitle2_style, LV_STATE_DEFAULT, 10);
      lv_style_set_text_decor(&texttitle2_style, LV_STATE_DEFAULT, LV_TEXT_DECOR_UNDERLINE);

      lv_obj_t *texttitle2 = lv_label_create(lv_scr_act(), NULL);  //define a title variable on the screen
      lv_obj_add_style(texttitle2, LV_LABEL_PART_MAIN, &texttitle2_style);
      lv_label_set_text(texttitle2, "Control Screen");          //puts title on screen
      lv_obj_align(texttitle2, NULL, LV_ALIGN_CENTER, 0, -65);  //align the title on the screen



      //Create a style for the button
      static lv_style_t save_exit_buttonStyle;
      //Initialte the style with the default values
      lv_style_init(&save_exit_buttonStyle);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&save_exit_buttonStyle, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&save_exit_buttonStyle, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));  //Red

      //Create save/exit Button
      lv_obj_t *save_exit_button = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(save_exit_button, save_exit_event_handler);  //task for save/exit(se) button
      lv_obj_align(save_exit_button, NULL, LV_ALIGN_CENTER, 0, 0);
      lv_obj_add_style(save_exit_button, LV_STATE_DEFAULT, &save_exit_buttonStyle);  //Apply style to
      //Attaches label to the button
      lv_obj_t *save_exit_label = lv_label_create(save_exit_button, NULL);
      lv_label_set_text(save_exit_label, "Save/Exit");

      //Create a style for the button
      static lv_style_t pause_resume_buttonStyle;
      //Initialte the style with the default values
      lv_style_init(&pause_resume_buttonStyle);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&pause_resume_buttonStyle, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&pause_resume_buttonStyle, LV_STATE_DEFAULT, lv_color_hex(0xFFFF00));  //Yellow

      //Create Button
      lv_obj_t *pause_resume_button = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(pause_resume_button, pause_resume_event_handler);  //task for Pause/Resume(pr) button
      lv_obj_align(pause_resume_button, NULL, LV_ALIGN_CENTER, 0, 60);
      lv_obj_add_style(pause_resume_button, LV_STATE_DEFAULT, &pause_resume_buttonStyle);  //Apply style to

      //Attaches label to the button
      lv_obj_t *pause_resume_label = lv_label_create(pause_resume_button, NULL);
      lv_label_set_text(pause_resume_label, "Pause/Resume");
    }
  }
}
/////////////////////////////////////////////////////
//event_handler (lv_obj_t *obj, 
//lv_event_t event) is called when the 
//Start Session button is pressed. Contains multiple
//functionalities like screen, buttons, styles,etc.
//This function relates like the 
//save_exit_event_handler functions.
/////////////////////////////////////////////////////
static void event_handlerss(lv_obj_t *obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    dataFile = SPIFFS.open("/recorded_times.txt", FILE_WRITE);  //Every time we click, the afile will be created with this name
    if (!dataFile) {                                            //if file doesnt exit print out a error warning

      Serial.println("Failed to open data file for writing.");
      return;
    }
    sendSingleCharacter('Y');//send signal to miniPC to start session
    Serial.println("File opened successfully.");  // print out that the file is open
    // Initialize the sessionTimer ticker to call printSessionTimer every 100 milliseconds, on setup fun intially
    sessionTimer.attach(0.1, printSessionTimer);  //ro print timer

    if (!timerRunning) {
      //isFileOpen = true;  //Opens a file once to initizilize it and leaves it open until the session ends
      sessionStartTime = millis();
      Serial.println("Session Start Time: " + String(sessionStartTime / 1000) + " seconds");  //Print Start Time
      timerRunning = true;
      saveSessionData("SST: " + String(sessionStartTime) + " ms");  //sends ending time in milli seconds, divide by 1000 to see in seconds

      // Create a new screen (key_behavior_screen) and load it when the button is clicked
      key_behavior_screen = lv_obj_create(NULL, NULL);  //assign empty screen
      lv_scr_load(key_behavior_screen);                 //load key behavior screen

      // Create a style for the label and make the text bold
      static lv_style_t key_behavior_screen_texttitle_style;
      lv_style_init(&key_behavior_screen_texttitle_style);
      lv_style_set_text_color(&key_behavior_screen_texttitle_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
      lv_style_set_text_letter_space(&key_behavior_screen_texttitle_style, LV_STATE_DEFAULT, 3);
      lv_style_set_text_line_space(&key_behavior_screen_texttitle_style, LV_STATE_DEFAULT, 10);
      lv_style_set_text_decor(&key_behavior_screen_texttitle_style, LV_STATE_DEFAULT, LV_TEXT_DECOR_UNDERLINE);

      lv_obj_t *key_behavior_screen_texttitle = lv_label_create(lv_scr_act(), NULL);  //define a title on the screen
      lv_obj_add_style(key_behavior_screen_texttitle, LV_LABEL_PART_MAIN, &key_behavior_screen_texttitle_style);
      lv_label_set_text(key_behavior_screen_texttitle, "Behavioral Screen");       //puts title on screen
      lv_obj_align(key_behavior_screen_texttitle, NULL, LV_ALIGN_CENTER, 0, -99);  //align the title on the screen

      static lv_style_t key_behavior_btn1Style;
      //Initialte the style with the default values
      lv_style_init(&key_behavior_btn1Style);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&key_behavior_btn1Style, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&key_behavior_btn1Style, LV_STATE_DEFAULT, lv_color_hex(0x90EE90));  //light green


      //Key behavior button 1
      key_behavior_btn1 = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(key_behavior_btn1, key_behavior_btn1_event_handler);         // Assign event handler for Button 1
      lv_obj_set_size(key_behavior_btn1, LV_HOR_RES / 2.4, LV_VER_RES / 6);            //y=8
      lv_obj_add_style(key_behavior_btn1, LV_STATE_DEFAULT, &key_behavior_btn1Style);  //Apply style to
      lv_obj_align(key_behavior_btn1, NULL, LV_ALIGN_CENTER, -57, -60);                //y=10 is center + move veritcally down screen
      // Attaches label to Button 1
      btn1_label = lv_label_create(key_behavior_btn1, NULL);
      lv_label_set_text(btn1_label, label1_title);

      static lv_style_t key_behavior_btn2Style;
      //Initialte the style with the default values
      lv_style_init(&key_behavior_btn2Style);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&key_behavior_btn2Style, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&key_behavior_btn2Style, LV_STATE_DEFAULT, lv_color_hex(0xFFD700));  //Light orange

      //Key behavior button 2
      key_behavior_btn2 = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(key_behavior_btn2, key_behavior_btn2_event_handler);         // Assign event handler for Button 1
      lv_obj_set_size(key_behavior_btn2, LV_HOR_RES / 2.4, LV_VER_RES / 6);            //y=8
      lv_obj_align(key_behavior_btn2, NULL, LV_ALIGN_CENTER, 57, -60);                 //y=10 is center + move veritcally down screen
      lv_obj_add_style(key_behavior_btn2, LV_STATE_DEFAULT, &key_behavior_btn2Style);  //Apply style to ssbtn
      // Attaches label to Button 1
      btn2_label = lv_label_create(key_behavior_btn2, NULL);
      lv_label_set_text(btn2_label, label2_title);

      static lv_style_t key_behavior_btn3Style;
      //Initialte the style with the default values
      lv_style_init(&key_behavior_btn3Style);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&key_behavior_btn3Style, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&key_behavior_btn3Style, LV_STATE_DEFAULT, lv_color_hex(0xADD8E6));  // light blue

      //Key behavior button 3
      key_behavior_btn3 = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(key_behavior_btn3, key_behavior_btn3_event_handler);         // Assign event handler for Button 1
      lv_obj_set_size(key_behavior_btn3, LV_HOR_RES / 2.4, LV_VER_RES / 6);            //y=8
      lv_obj_add_style(key_behavior_btn3, LV_STATE_DEFAULT, &key_behavior_btn3Style);  //Apply style to ssbtn
      lv_obj_align(key_behavior_btn3, NULL, LV_ALIGN_CENTER, -57, 10);                 //y=10 is center + move veritcally down screen
      // Attaches label to Button 1
      btn3_label = lv_label_create(key_behavior_btn3, NULL);
      lv_label_set_text(btn3_label, label3_title);

      static lv_style_t key_behavior_btn4Style;
      //Initialte the style with the default values
      lv_style_init(&key_behavior_btn4Style);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&key_behavior_btn4Style, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&key_behavior_btn4Style, LV_STATE_DEFAULT, lv_color_hex(0xFFB6C1));  //light pink

      //Key behavior button 4
      key_behavior_btn4 = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(key_behavior_btn4, key_behavior_btn4_event_handler);         // Assign event handler for Button 1
      lv_obj_set_size(key_behavior_btn4, LV_HOR_RES / 2.4, LV_VER_RES / 6);            //y=8
      lv_obj_add_style(key_behavior_btn4, LV_STATE_DEFAULT, &key_behavior_btn4Style);  //Apply style to ssbtn
      lv_obj_align(key_behavior_btn4, NULL, LV_ALIGN_CENTER, 57, 10);                  //y=10 is center + move veritcally down screen
      // Attaches label to Button 1
      btn4_label = lv_label_create(key_behavior_btn4, NULL);
      lv_label_set_text(btn4_label, label4_title);

      static lv_style_t end_session_btnStyle;
      //Initialte the style with the default values
      lv_style_init(&end_session_btnStyle);
      //Modify the style to the set the background color to green
      lv_style_set_radius(&end_session_btnStyle, LV_STATE_DEFAULT, 10);
      lv_style_set_bg_color(&end_session_btnStyle, LV_STATE_DEFAULT, lv_color_hex(0xFF9999));  //light red

      //End session Buttone
      end_session_btn = lv_btn_create(lv_scr_act(), NULL);
      lv_obj_set_event_cb(end_session_btn, end_session_event_handler);             // Assign event handler for Button 1
      lv_obj_set_size(end_session_btn, LV_HOR_RES / 2, LV_VER_RES / 6);            //y=8
      lv_obj_add_style(end_session_btn, LV_STATE_DEFAULT, &end_session_btnStyle);  //Apply style to ssbtn
      lv_obj_align(end_session_btn, NULL, LV_ALIGN_CENTER, 0, 80);                 //y=10 is center + move veritcally down screen
      // Attaches label to Button 1
      end_session_label = lv_label_create(end_session_btn, NULL);
      lv_label_set_text(end_session_label, end_session_title);

      // Create a label to display Wi-Fi status
      TimestampingWarningLabel = lv_label_create(lv_scr_act(), NULL);
      lv_label_set_text(TimestampingWarningLabel, "");
      lv_obj_align(TimestampingWarningLabel, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    }
  }
}
/////////////////////////////////////////////////////
//setup() is called when the 
//smartwatch is on. Contains multiple
//functionalities like screen, buttons, styles,etc.
//This function relates like the 
//save_exit_event_handler functions.
/////////////////////////////////////////////////////
void setup() {

  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->lvgl_begin();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }

  start_session_screen = lv_obj_create(NULL, NULL);  //define screen
  lv_scr_load(start_session_screen);                 //load screen

  // Create a style for the label and make the text bold
  static lv_style_t texttitle_style;
  lv_style_init(&texttitle_style);
  lv_style_set_text_color(&texttitle_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_style_set_text_letter_space(&texttitle_style, LV_STATE_DEFAULT, 3);
  lv_style_set_text_line_space(&texttitle_style, LV_STATE_DEFAULT, 10);
  lv_style_set_text_decor(&texttitle_style, LV_STATE_DEFAULT, LV_TEXT_DECOR_UNDERLINE);

  lv_obj_t *texttitle = lv_label_create(lv_scr_act(), NULL);  //define a title on the screen
  lv_obj_add_style(texttitle, LV_LABEL_PART_MAIN, &texttitle_style);
  lv_label_set_text(texttitle, "Thera-Log Application");   //puts title on screen
  lv_obj_align(texttitle, NULL, LV_ALIGN_CENTER, 0, -65);  //align the title on the screen

  //Create a style for the button
  static lv_style_t ssbtnStyle;
  //Initialte the style with the default values
  lv_style_init(&ssbtnStyle);
  //Modify the style to the set the background color to green
  lv_style_set_radius(&ssbtnStyle, LV_STATE_DEFAULT, 10);
  lv_style_set_bg_color(&ssbtnStyle, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));  //Green

  lv_obj_t *labelss;  //define labels for start session for the button

  ssbtn = lv_btn_create(lv_scr_act(), NULL);           //define button
  lv_obj_set_event_cb(ssbtn, event_handlerss);         //creates an acction for the button
  lv_obj_align(ssbtn, NULL, LV_ALIGN_CENTER, 0, +60);  //aligns button on the screen
  //Apply style to ssbtn
  lv_obj_add_style(ssbtn, LV_STATE_DEFAULT, &ssbtnStyle);
  labelss = lv_label_create(ssbtn, NULL);       //links label o the button
  lv_label_set_text(labelss, "Start Session");  //set the label to teh button
  //disableButton();                              // ****                                 //disables start session button until is connected to a Wifi.


  static lv_style_t wifi_btnStyle;
  //Initialte the style with the default values
  lv_style_init(&wifi_btnStyle);
  //Modify the style to the set the background color to green
  lv_style_set_radius(&wifi_btnStyle, LV_STATE_DEFAULT, 10);
  lv_style_set_bg_color(&wifi_btnStyle, LV_STATE_DEFAULT, lv_color_hex(0xD3D3D3));  //light gray

  lv_obj_t *wifilabel;                           //define labels for start session for the button
  wifi_btn = lv_btn_create(lv_scr_act(), NULL);  //define button
  //lv_obj_set_event_cb(wifi_btn, wifi_event_handler);    //creates an acction for the button*************
  lv_obj_align(wifi_btn, NULL, LV_ALIGN_CENTER, 0, 0);  //aligns button on the screen
    //Apply style to ssbtn
  lv_obj_add_style(wifi_btn, LV_STATE_DEFAULT, &wifi_btnStyle);
  wifilabel = lv_label_create(wifi_btn, NULL);       //links label o the button
  lv_label_set_text(wifilabel, "Connect to Wi-Fi");  //set the label to teh button

  // Create a label to display Wi-Fi status
  wifi_status_label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(wifi_status_label, "Disconnected");
  lv_obj_align(wifi_status_label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
}
/////////////////////////////////////////////////////
//deleteSessionFile() function is called when the 
//Save/Exit button is pressed. The function will
//delete the file created after the session ends and
//the file has been transmitted thru USB serial port.
/////////////////////////////////////////////////////
void deleteSessionFile() {
  String file = "/recorded_times.txt";             // File that was previously created
  if (SPIFFS.remove(file)) {                       //deletes file after creating it otherwise, if starting a new session, it will give an error
    Serial.println("File deleted successfully.");  //print on serial if was succesffully deleted
  } else {
    Serial.println("Failed to delete the file.");  //prints on serial if was NOT successfully deleted
  }
}
/////////////////////////////////////////////////////
//saveSessionData(String data) function is call
//when the Start Session, key behavioral buttons,
//and Save/Exit button are pressed. The function 
//will be called to save string of data from the 
//type relate to start and stop of the session 
//and pressed behavioral times.
/////////////////////////////////////////////////////
void saveSessionData(String data) {  //put this with the recording timer to open with

  if (dataFile) {            //Ensure the file is open
    dataFile.println(data);  //Write data to the file
  } else {
    Serial.println("File not open for writing.");
  }
}
/////////////////////////////////////////////////////
//serialPrintFileContent() function is called when
//the Save/Exit button is pressed. The function opens
//the save file on the smartwatch and prints the
//results thru USB serial port.
/////////////////////////////////////////////////////
void serialPrintFileContent() {

  // Open the file
  dataFile = SPIFFS.open("/recorded_times.txt", FILE_READ);  //remove fs::File since you declaring it as global, opens created file
  Serial.println("File open successfully.");
  if (dataFile) {  //if file exist then print out the data otherwise provide a noticed there no files.
    while (dataFile.available()) {
      char c = dataFile.read();
      Serial.write(c);  // Send each character to the serial port
    }

    dataFile.close();
  } else {
    Serial.println("File not found or unable to open.");
  }
}
/////////////////////////////////////////////////////
//loop() function loops every 5 ms to refresh the
//smartwatch interrupts. Additionally, it checks for
//WiFi connectivity and updates status.
/////////////////////////////////////////////////////
void loop() {

  updateWifiStatusText();
  // Check WiFi connection status periodically
  if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
    // WiFi connected, show the symbol
    wifiConnected = true;
    //enableButton();

  } else if (WiFi.status() != WL_CONNECTED && wifiConnected) {
    // WiFi disconnected, hide the symbol
    wifiConnected = false;
    //disableButton();
  }


  lv_task_handler();
  delay(5);
}