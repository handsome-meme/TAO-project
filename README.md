# SHC (Software and Hardware Collaboration) Now Renamed TAO
The control code and P4 code for the Tofino ASIC on the programmable switch are located in `/share/platform`.
For detailed logic, refer to the paper: *"Accelerating Hardware/Software Combined Traffic Processing with Fast and Efficient Asynchronous Flow Offloading"*.

## Miscellaneous
### Runtime Environment Description
The container environment is developed by Tencent based on Ruijie programmable switches. To adapt to the switch environment of the university laboratory, partial modifications have been made to the project:
1. In the cmake folder, the command `-DP4PPFLAGS="-Xp4c=--disable-parse-depth-limit"` in the `p4.cmake` file has been commented out, as it is not supported by SDE 9.7.0.

### Laboratory Environment
There are two programmable switches from different vendors. It is necessary to request the Board Support Package (BSP) from the vendors, along with the ONL (Open Network Linux) images, kernel images and kernel header files.

#### Laboratory Tofino Switch Environment Configuration
- Physical Host OS: 3.16.39-OpenNetworkLinux
- Container OS: "Debian GNU/Linux 9 (stretch)"

**Configuration Command**:
```bash
./p4studio configure --bsp-path ../../bf-platforms-ingrasys-bsp-9.7.0.tgz
