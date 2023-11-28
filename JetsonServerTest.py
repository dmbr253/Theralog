from socket import *
import subprocess

serverPort = 8888

# Replace server IP address with actual Jetson IP address
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(("192.168.137.10", serverPort))

serverSocket.listen(1)

print("Jetson Server is ready to receive...")

# Has a pre-set video path of the recording
videopath = 'output.mp4'

while True:
    # Establish connection between the Jetson Nano and the MiniPC
    clientSocket, addr = serverSocket.accept()

    response = clientSocket.recv(1024).decode()
    print("Received response from MiniPC:", response)
    
    # Handles the response sent by the client, sends video if "Y", closes connection if "N"
    if response == "Y":
        # Turns the cameras on, waits for response from client to turn them off
        
        # Stores video in output.mp4
    
        # Open the video at the designated file path
        with open(videopath, 'rb') as video_file:
            # Keep reading the file into video_chunk until empty and then send it to MiniPC
            while True:
                video_chunk = video_file.read(1024)
                if not video_chunk:
                    break
                clientSocket.send(video_chunk)
        # Closes MiniPC connection
        clientSocket.close()
    else:
        message = "Alright, closing connection."
        clientSocket.send(message.encode())
        clientSocket.close()

# Closes Jetson Nano connection
serverSocket.close()


