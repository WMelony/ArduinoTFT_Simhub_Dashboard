# ArduinoTFT_Simhub_Dashboard
Uses SimHub's custom protocol to send telemetry data to the Arduino and displays it with a 3.5 inch TFT shield.
![20250706_133803](https://github.com/user-attachments/assets/40750e6e-4f6f-4287-bf0a-33af1c73173b)


# INSTALLATION
  - #### Install SimHub.
  - #### Go to C:\Program Files (x86)\SimHub\_Addons\Arduino\DisplayClientV2 and replace SHCustomProtocol.h with mine.
  - #### In SimHub:
    - Go to Arduino -> My Hardware -> Open arduino setup tool.
    - Configure and press Upload to Arduino.
    - Your dash should appear in the device list, click on EDIT in Custom Protocol.
    - Choose NCALC and paste my custom protocol.
    ![image](https://github.com/user-attachments/assets/1611bf49-41c5-4dab-9fb1-64125009d26b)

  - #### Configure your SimHub with your desired game and play.


# PROBLEMS

  ### GENERAL
  - Gear and speed text does not clear old text with matching background colour, rather still using the default black, whether the background behind it is black or not.
  ### ACC
  - The brake bias will use the brake bias percent instead of brake bias value.
  - Lap info still counts the formation lap while the in-game UI does not.
  ### AC
  - Battery percent is reversed compared to other titles (Data sends the amount of battery used rather than the amount of battery left).
