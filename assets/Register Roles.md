Introduction:
The AlphaMon Platform uses pre-defined Roles to define what type of data is stored in each Modbus register.
Each Role is a numeric, C++ enum value.

Why do we need Roles?
Roles are a memory efficient way of describing what the register does and holds.
They also allow for easy aggregation of data across all registers, on all connected and remote Modbus devices,  
that have the same Role number.

Where do Roles appear?
You'll find the Role numbers assigned to individual registers in your Data Definition Files (DDFs).
Whilst we attempt to accurately assign the correct Role number to third-party devices at the time we build the 
DDFs, you can edit your DDF using any of the popular text editors and change assigned Role numbers, as required.
Then, simply save your updated DDF to your AlphaMon's microSD card, delete any saved DDF of the same name on your
AlphaMon using the "FILE DELETE <your-ddf-name>" command.

Your AlphaMon will then copy your edited DDF to internal SPIFFS storage during the boot-up DDF synchronization steps.

What are the currently defined Role numbers?
See the attached "Register Roles.cpp" file. Note, this enum may be converted to a uint16_t data type at some point 
soon due to the space being rather crowded.
