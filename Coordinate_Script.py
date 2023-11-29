import subprocess
import pyautogui
import time

def find_button_coordinates():
    try:
        print("Move the mouse over the button and press Ctrl+C to capture coordinates.")
        time.sleep(7)  # Give some time to position the mouse

        # Get the current mouse cursor position
        button_x, button_y = pyautogui.position()
        print(f"Button coordinates: ({button_x}, {button_y})")

        # Run the command line (replace 'your_command' with the actual command)
        #cmd_command = "your_command"
       # subprocess.Popen(cmd_command, shell=True)

    except KeyboardInterrupt:
        print("\nCoordinates captured. Exiting...")

if __name__ == "__main__":
    find_button_coordinates()