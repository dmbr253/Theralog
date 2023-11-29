from socket import * 
import pyaudio
import wave
import time
import threading
import os
import serial
from datetime import datetime

# Adjust baud rate if needed
ser = serial.Serial('COM3', 115200)

# Static Folder for Video/Audio Storage
folder = 'C:\\Users\\Thera-log\\Desktop\\Local_Session_Files'

# Replace with actual IP Address of Jetson Nano Cameras
camera_1_IP = '192.168.137.10'
camera_1_port = 8888

# Sets path of the video (name of the file)
output_path = 'camera_1_recording.mp4'

camera_1_Socket = socket(AF_INET, SOCK_STREAM)

def start_audio_recording():
    audio = pyaudio.PyAudio()

    stream = audio.open(format=pyaudio.paInt16, channels=1, rate=44100, input=True, frames_per_buffer=1024)

    frames = []

    print("Recording audio. . .")

    while audio_recording_flag.is_set():
        data = stream.read(1024)
        frames.append(data)
        
    print("Finished recording audio")

    stream.stop_stream()
    stream.close()
    audio.terminate()

    sound_file = wave.open(os.path.join(folder,"myrecording.wav"), "wb")
    sound_file.setnchannels(1)
    sound_file.setsampwidth(audio.get_sample_size(pyaudio.paInt16))
    sound_file.setframerate(44100)
    sound_file.writeframes(b''.join(frames))
    sound_file.close()
    
def start_audio_and_video():
    camera_1_Socket.send("Y".encode())
    audio_recording_flag.set()
    print("Both audio and video recording started.")
    
def stop_audio_and_video():
    camera_1_Socket.send("Y".encode())
    audio_recording_flag.clear()
    
    # Opens the sent file and writes the received video data into a specific file
    with open(os.path.join(folder, output_path), 'wb') as output_file:
        while True:
            video_chunk = camera_1_Socket.recv(1024)
            if not video_chunk:
                break
            output_file.write(video_chunk)
    
    print("Both audio and video recording stopped.")
    audio_thread.join()
    
def receive_file():
    file_name = "recorded_times.txt"
    
    # Wait for the start signal
    start_signal = "START_OF_TRANSMISSION"
    start_signal_received = False

    while True:
        data = ser.readline().decode().strip()
        print("Data:", data)
        if data == start_signal:
            start_signal_received = True
            print(f"start signal received: True")
            break

    if not start_signal_received:
        print("Failed to receive start signal. Exiting.")
        return

    # Generate a received file name based on the current date and time
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    received_file_path = os.path.join(folder, f"received_{timestamp}_{file_name}")

    with open(os.path.join(folder, received_file_path), 'wb') as file:
        print(f"Receiving file {file_name}...")

        while True:
            data = ser.readline().decode().strip()
            print("Data:", data)
            
            # Check for the end of transmission signal
            if data == "END_OF_TRANSMISSION":
                print(f"End transmission")
                break

            file.write(data.encode())
            file.write("\n".encode())

    print(f"File received successfully: {received_file_path}")


varinputloop = True;
# Instruct Jetson and microphone to start session recording
#response = input("Start recording? (Y) ")
while(varinputloop == True):
    data = ser.read(1).decode()#decode serial for single character
    print("Reading Smartwatch Input: ", data)
    if data == "Y":
        camera_1_Socket.connect((camera_1_IP, camera_1_port))
        audio_recording_flag = threading.Event()
        audio_thread = threading.Thread(target=start_audio_recording)
        print("Start video/audio")
        start_audio_and_video()
        audio_thread.start()


# Instruct Jetson and microphone to end session recording
#response = input("Stop recording? (Y) ")
    if data == "N":
        receive_file()
        stop_audio_and_video()
        camera_1_Socket.close()
        varinputloop = False

