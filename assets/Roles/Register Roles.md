Introduction:
The AlphaMon Platform uses pre-defined Roles to define what type of data is stored in each Modbus register.
Each Role is a numeric, C++ enum value (unsigned integer, 8 bits).

Why do we need Roles?
Roles are a memory efficient way of describing what the register does and holds.
They also allow for easy aggregation of data across all registers, on all connected and remote Modbus devices,  
that have the same Role number.

Note: The Role number doesn't indicate anything other than the primary purpose of a register. For example, it isn't
intended to tell us anything about the default sign of the value, the size of the register, the data type, etc.
This extra register information can be found in other DDF values such as cUom (Unit of Measure; V, KwH, A, W, ect.) and
fMultiplier (which defaults to 1 but can be used to scale the output or to flip the sign of the read value).  

Where do Roles appear?
You'll find the Role numbers assigned to individual registers in your Data Definition Files (DDFs).
Whilst we attempt to accurately assign the correct Role number to third-party devices at the time we build the 
DDFs, you can edit your DDF using any of the popular text editors and change assigned Role numbers, as required.
Then, simply save your updated DDF to your AlphaMon's microSD card, delete any saved DDF of the same name on your
AlphaMon using the "FILE DELETE <your-ddf-name>" command.

Why don't all DDF registers have an assigned role?
AlphaMon Roles are primarily user for enhanced report-generation purposes.  Thus, low-level or highly device-specific registers, 
such as a register which holds the cell temperature of a single cell in a Lithium battery pack, are ignored and should be assigned 
a Role of 0 (unassigned value).

Your AlphaMon will then copy your edited DDF to internal SPIFFS storage during the boot-up DDF synchronization steps.

What are the currently defined Role numbers?
See the attached "Register Roles.cpp" file. Note, this enum may be converted to a uint16_t data type at some point 
soon due to the space being rather crowded.
