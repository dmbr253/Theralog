import os
import json
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import datetime
from datetime import timedelta  
import pyautogui
import time
import subprocess
import threading
import glob

# Define the folder to monitor for .txt files
folder_to_monitor = "C:/Users/keaga/OneDrive/Desktop/Session_Files"
conversion_done_event = threading.Event()  # Event to signal when conversion is done
conversion_done_lock = threading.Lock()  # Lock to synchronize access to conversion_done_flag
conversion_done_flag = False  # Flag to indicate if conversion is done

def ConversionScript():
    global folder_to_monitor
    global conversion_done_flag
    
    # Define a function to parse a text file and create a list of dictionaries
    def parse_text_to_json(txt_file_path):
        json_data = []  # Initialize an empty list

        with open(txt_file_path, 'r') as txt_file:
            for line in txt_file:
                line = line.strip()
                if not line.startswith("SST") and not line.startswith("SET"):
                    # Strip any additional spaces from the end of the line
                    line = line.rstrip()
                
                    # Split the line on ':' and handle cases where there might be fewer elements
                    parts = line.split(':')
                    if len(parts) >= 2:
                        behavior = parts[0].strip()
                        timestamp = parts[1].strip().split(' ')[0]
                        # Convert timestamp from milliseconds to seconds
                        timestamp_seconds = int(timestamp) / 1000
                        
                        timestamp_formatted = str(timedelta(seconds=timestamp_seconds))   

                        json_data.append({"Behavior": behavior, "Timestamp": timestamp_formatted})
                
        return json_data

    # Define a handler for file events
    class MyHandler(FileSystemEventHandler):
        def on_created(self, event):
            if event.is_directory:
                return
            if event.src_path.endswith('.txt'):
                # Get the base filename without the extension
                base_filename = os.path.splitext(os.path.basename(event.src_path))[0]

                # Convert the .txt file to a list of dictionaries
                json_data = parse_text_to_json(event.src_path)

                # Save the converted data as a .boris file
                boris_file_path = os.path.join(folder_to_monitor, f'{base_filename}.boris')
                

                with open(boris_file_path, 'w') as boris_file:
                    json.dump(json_data, boris_file, indent=2)

                print(f"Converted {event.src_path} to Boris format and saved as {base_filename}.boris")

                # Call BorisFormatting function
                BorisFormatting(boris_file_path)

    observer = Observer()
    observer.schedule(MyHandler(), path=folder_to_monitor)
    observer.start()
    try:
        while True:
            time.sleep(1)
            with conversion_done_lock:
                if not conversion_done_flag:
                    continue
                else:
                    break
    finally:
        observer.stop()
        observer.join()
        conversion_done_event.set()  # Set the event to signal that conversion is done

def BorisFormatting(boris_file_path):
    global conversion_done_flag
    # Load the existing JSON data
    with open(boris_file_path, "r") as file:
        data = json.load(file)
    # Get the current date in the format expected by Boris
    current_date = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")

    # Create a new dictionary in the Boris format
    boris_data = {
        "time_format": "hh:mm:ss",
        "project_date": current_date,
        "project_name": "Your Project Name",
        "project_description": "Your Project Description",
        "project_format_version": "7.0",
        "subjects_conf": {
            "0": {
                "key": "1",
                "name": "subj1",
                "description": ""
            },
        },
        "behaviors_conf": {
            "0": {
                "type": "State event",
                "key": "1",
                "code": "B1(Smiling)",
                "description": ""
            },
            "1": {
                "type": "State event",
                "key": "2",
                "code": "B2(Vocalization)",
                "description": ""
            },
            "2": {
                "type": "State event",
                "key": "3",
                "code": "B3(AB)",
                "description": ""
            },
            "3": {
                "type": "State event",
                "key": "4",
                "code": "B4(SIB)",
                "description": ""
            }
        },
        "observations": {
            "Session 1": {
                "file": {
                    "1": [],
                    "2": [],
                    "3": [],
                    "4": []
                },
                "type": "MEDIA",
                "date": current_date,
                "description": "",
                "time offset": 0.0,
                "events": []
            }
        },
        "behavioral_categories": [],
        "independent_variables": {},
        "coding_map": {},
        "behaviors_coding_map": [],
        "converters": {},
    }

    # Taking the information from the converted .boris file and formatting it to Boris format
    for item in data:
        timestamp = item["Timestamp"]
        timestamp_parts = timestamp.split(":")
        # Convert the timestamp to seconds as a decimal
        seconds = float(timestamp_parts[0]) * 3600 + float(timestamp_parts[1]) * 60 + float(timestamp_parts[2])
        event = [f"{seconds:.3f}", "", item["Behavior"], "", "", 0]  # Format to 3 decimal places
        boris_data["observations"]["Session 1"]["events"].append(event)

    # Save the modified .boris file
    with open(boris_file_path, "w") as output_file:
        json.dump(boris_data, output_file, indent=4)

    with conversion_done_lock:
        conversion_done_flag = True  # Set the flag to True when conversion is done

# Call the ConversionScript function in a separate thread
conversion_thread = threading.Thread(target=ConversionScript)
conversion_thread.start()

# Wait for the conversion to be done for at least one file
conversion_done_event.wait()


def open_application_and_files():
    global video_path
    global audio_path
    # shortcut path to boris .lnk file
    shortcut_path = r"C:\Users\keaga\OneDrive\Desktop\BORIS.lnk"
    
    # Define the directory where your .boris files are located
    
    directory_path = r"C:\Users\keaga\OneDrive\Desktop\Session_Files"

    # Search for .boris files in the specified directory
    list_of_files = glob.glob(os.path.join(directory_path, "*.boris"))

    # Select the most recent .boris file based on its modification time
    if list_of_files:
        most_recent_file = max(list_of_files, key=os.path.getmtime)
        boris_path = most_recent_file
        print(f"Most recent .boris file: {boris_path}")

        # Extract the directory of the most recent .boris file
        file_directory = os.path.dirname(most_recent_file)

        # Search for video files in the same directory
        list_of_video_files = glob.glob(os.path.join(file_directory, "*.mp4"))
        if list_of_video_files:
            most_recent_video = max(list_of_video_files, key=os.path.getmtime)
            video_path = most_recent_video
            print(f"Most recent video file: {most_recent_video}")
            

        # Search for audio files in the same directory
        list_of_audio_files = glob.glob(os.path.join(file_directory, "*.WAV"))
        if list_of_audio_files:
            most_recent_audio = max(list_of_audio_files, key=os.path.getmtime)
            audio_path = most_recent_audio
            print(f"Most recent audio file: {most_recent_audio}")
           

        try:
            # Open the application using the 'start' command and wait for it to finish
            subprocess.run(['start', 'cmd', '/c', shortcut_path], shell=True, check=True)

            # Add a delay to ensure the application has enough time to open
            time.sleep(15)  

            # Use pyautogui to perform actions
            # Click on Project menu
            pyautogui.click(x=34, y=42)  
            time.sleep(2)

            # Click on Open option in the File menu
            pyautogui.click(x=65, y=84)  
            time.sleep(2)

            # Use the most recent .boris file found
            pyautogui.write(boris_path)
            pyautogui.press('enter')
            
            #click observations tab
            pyautogui.click(x=119,y=52)
            time.sleep(2)

            #click edit observation tab
            pyautogui.click(x=164,y=134)
            time.sleep(2)

            #click on observation for editing
            pyautogui.click(x=515,y=335)
            time.sleep(2)


            #click edit button 
            pyautogui.click(x=1108,y=799)
            time.sleep(2)

            #click on add media button
            pyautogui.click(x=638, y=654)
            time.sleep(2)

            #click on absolute path button
            pyautogui.click(x=658,y=685)
            time.sleep(2)

            # Use the most recent audio file found
            pyautogui.write(audio_path)
            pyautogui.press('enter')

            #click on add media button again
            pyautogui.click(x=638, y=654)
            time.sleep(2)

            #click on absolute path again
            pyautogui.click(x=658,y=685)
            time.sleep(2)

            # Use most recent video file found
            pyautogui.write(video_path)
            pyautogui.press('enter')

            #click on media player selection tab for video file
            pyautogui.click(x=666,y=614)
            time.sleep(2)

            #click on media player option 2
            pyautogui.click(x=674,y=663)
            time.sleep(2)

            #click start button for observation
            pyautogui.click(x=1478,y=799)
            time.sleep(2)

            # Wait for user input before exiting (to ensure application does not close)
            input("Press Enter when you are finished. The script will exit.")

        except subprocess.CalledProcessError as e:
            print(f"An error occurred: {e}")

    else:
        print("No .boris files found in the specified directory")
        
        return

if __name__ == "__main__":
    open_application_and_files()



# Wait for the ConversionScript thread to finish
conversion_thread.join()