# S3 Flight Software

## How to install the Arduino IDE on Windows

0. Download Arduino IDE from [here](https://www.arduino.cc/en/software)
0. TBD - need to add "how to download + run arduino IDE" detail for Windows Version
0. If on Windows, you will need to install a driver to connect to the board:
    - Open [this link](https://github.com/adafruit/Adafruit_Windows_Drivers/releases) in a browser
    - Click "adafruit_drivers_X.X.X.X.exe" to download it
    - Double-click "adafruit_drivers_X.X.X.X.exe"
    - Click "I Agree", "Next", "Close" to get the driver installed


## How to install the Arduino IDE on Linux

0. Download Arduino IDE from [here](https://www.arduino.cc/en/software)
0. Choose "Linux AppImage 64 bits (X86-64)"
0. The file "arduino-ide_2.1.1_Linux_64bit.AppImage" will be saved to your Downloads folder
0. Run these commands in a terminal

        cd ~/Downloads/
        chmod +x arduino-ide_*
        ./arduino-ide_*

0. The Arduino IDE should open
0. Click "File->Preferences"
0. Paste the following into the "Additional Board URLs" textbox:

        https://adafruit.github.io/arduino-board-index/package_adafruit_index.json

0. Click "Ok" to close the Preferences window
0. Click "Tools -> Board -> Boards Manager"
0. Search for "Arduino SAMD Boards" and click "Install"
0. Wait a few minutes for the installation to complete
0. Next, search for "Adafruit SAMD Boards" and click "Install"
0. Wait a few minutes for the installation to complete
0. Reopen the Arduino IDE
    - Click "File -> Quit"
    - Run these commands in a terminal

          cd ~/Downloads/
          chmod +x arduino-ide_*
          ./arduino-ide_*

0. Select "Tools -> Board -> Adafruit SAMD boards -> Adafruit ItsyBitsy M4 (SAMD51)"
0. Done with setup! Continue to a different section.


## How to get the latest Quest software on Linux

0. Go to the Flight SW repository release page ([link](https://github.com/howellivy/S3-Flight))
0. Download the latest "Source code (zip)"
0. Unzip the folder

        cd ~/Downloads
        unzip 0.0.1.zip

0. Inside, you will find a "libraries" folder
0. Copy the "libraries" folder
0. Navigate to "~/Arduino/", delete the "libraries" folder if it exists, and paste the copied "libraries" folder

        rm -r ~/Arduino/libraries
        mkdir -p ~/Arduino/libraries
        cp -r ~/Downloads/0.0.1/libraries ~/Arduino

0. If you are updating a previously-installed version, you are done. Try building your project's `flying.ino` file in the Arduino IDE.
0. If this is your first time downloading the software, follow these steps to get the template `flying.ino`:
0. Make a folder somewhere on your PC to hold your project files. In this example, we will use `~/Projects/My_Experiment`
0. Make the folder using a file explorer window, or run this command:

        mkdir -p ~/Projects/My_Experiment

0. Copy `flying.ino` and `flying.h` into your project folder

        cp ~/Downloads/0.0.1/flying.ino ~/Downloads/0.0.1/flying.h ~/Projects/My_Experiment/

0. Open the Arduino IDE (If you have not installed it yet, see the section for [Windows](#how-to-install-the-arduino-ide-on-windows) or [Linux](#how-to-install-the-arduino-ide-on-linux)) first
0. In the Arduino IDE, select "File -> Open"
0. Navigate to `~/Projects/My_Experiment/` and double-click `flying.ino`
0. Click "Sketch -> Verify/Compile"
0. Wait a few seconds - the compilation should complete and output a message "Sketch uses XXX bytes of program..."
0. Move to the next section


## Connect to a development board

0. Open the Arduino IDE (If you have not installed it yet, see the section for [Windows](#how-to-install-the-arduino-ide-on-windows) or [Linux](#how-to-install-the-arduino-ide-on-linux)) first
0. In the Arduino IDE, select "File -> Open"
0. Navigate to `~/Projects/My_Experiment/` and double-click `flying.ino`
0. Connect a development board to your PC using a USB cable
0. Click "Tools -> Port -> /dev/ttyACM0 (Adafruit ItsyBitsy M4 (SAMD51))"
0. Click Tools -> Serial Monitor
0. To the right of the "Message" text box, there is a dropdown menu. If it says "New Line", select "Carrage Return" instead.
0. Click "Sketch -> Upload" (or press Ctrl+U)
0. Wait a few seconds for the code to compile and upload.
0. If all goes well, the serial monitor should output something similar to the following:

        Verify successful
        System Boot Version XX
        Compiled Date => XX
        Source file => XX

        Free Memory = XX HEX or XX DEC
        Input 'T' to enter test within 15 seconds

0. You can now type a `T` and hit `<Enter>` to start test mode. If you don't hit `T` fast enough, hit the RESET button on the development board to reboot and output the prompt again.


# Write your own code for your experiment

0. Open the Arduino IDE (If you have not installed it yet, see the section for [Windows](#how-to-install-the-arduino-ide-on-windows) or [Linux](#how-to-install-the-arduino-ide-on-linux)) first
0. In the Arduino IDE, select "File -> Open"
0. Navigate to `~/Projects/My_Experiment/` and double-click `flying.ino`
0. Edit the `Flying()` function in `flying.ino` with your experiment's custom code
0. Click "Sketch -> Upload" (or press Ctrl+U)
0. Your custom code will run


