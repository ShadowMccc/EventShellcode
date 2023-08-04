# EventShellcodeAttackTool

**Reproduction and Enhancement of a Method Exploiting Kaspersky's Discovery of Writing Shellcode to Eventlog**

## Functionality:

### Dropper:
1. Download Shellcode and write it into the HardwareEvents (or any other event you may choose) event. If the file size is greater than 6000, it will be written into multiple events; if it is smaller than or equal to 6000, it will be written into a single event.
2. Download Loader and set the file attributes to hidden before execution.

### Loader:
1. Read Shellcode from HardwareEvents and execute it. After executing the Shellcode, the event logs will be cleared. Of course, you can choose to modify this behavior to not clear logs.

**Note:** Admin privileges are required to write to HardwareEvents. The Shellcode can be encrypted using any preferred method.

### References:
 https://securelist.com/a-new-secret-stash-for-fileless-malware/106393/
