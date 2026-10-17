The AlphaMon Platform uses Defice Definition Files (DDFs) to describe various external, Modbus-enabled devices to the AlphaMon's internal services.

You can create your own Json-formatted DDF for your specific inverter, smart meter or energy storage system of download one of the pre-configured DDFs available in this folder.

DDF file name must follow the 8.3 DOS-like file naming conventions, where the main name must not exceed 8 characters. The file name extent (the last 3 characters after the dot in the file name) must match the firmware your AlphaMon is running. For example use "417" as the file name extent to match 
firmware versions 4.17.x. DDF names are case-insensitive, but we recommend using full upper-case to ensure compatibility with future software libraries.

To load your customised DDF/s, place them in the root directory on the AlphaMon's microSD card. During the boot process your AlphaMon will check for any missing DDFs; first on your microSD card and then the Solargy Over-the-Air (OTA) download page, and pick the first one found.

If your custom/modified DDF file has the same name as one already on the AlphaMon's internally SPIFFS storage it may get ignored unless the time & date stamps on the files are correct. Thus, it is wise, as a preventative measure, to delete the old DDF before rebooting using the "FILE DELETE <your-ddf-name.417".  Alternatively, to delete ALL DDFs from SPIFFS with a single command, use the CLI command "FILE DELETE DEVICES".

