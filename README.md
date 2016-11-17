# esxi_sense_decode

Interpreting SCSI sense codes in VMware ESXi and ESX

For ESXi/ESX 3.5 U5 and ESXi/ESX 4.0 and later versions:

H:0xA D:0xB P:0xC Possible sense data: 0xD 0xE 0xF
A = Host status (Initiator)
B = Device Status (Target)
C = Plugin (VMware Specific)
D = Sense Key
E = Additional Sense Code
F = Additional Sense Code Qualifier

# SCSI Host codes
SG_ERR_DID_OK           [0x00]	    NO error
SG_ERR_DID_NO_CONNECT   [0x01]	    Couldn't connect before timeout period
SG_ERR_DID_BUS_BUSY     [0x02]	    BUS stayed busy through time out period
SG_ERR_DID_TIME_OUT     [0x03]	    TIMED OUT for other reason (often this an unexpected device selection timeout)
SG_ERR_DID_BAD_TARGET   [0x04]	    BAD target, device not responding?
SG_ERR_DID_ABORT        [0x05]	    Told to abort for some other reason.
SG_ERR_DID_PARITY       [0x06]	    Parity error. Older SCSI parallel buses have a parity bit for error detection. This probably indicates a cable or termination problem.
SG_ERR_DID_ERROR        [0x07]	    Internal error detected in the host adapter.
SG_ERR_DID_RESET        [0x08]	    The SCSI bus (or this device) has been reset. Any SCSI device on a SCSI bus is capable of instigating a reset.
SG_ERR_DID_BAD_INTR     [0x09]	    Got an interrupt we weren't expecting
SG_ERR_DID_PASSTHROUGH  [0x0a]	    Force command past mid-layer
SG_ERR_DID_SOFT_ERROR   [0x0b]	    The low level driver wants a retry

For more information, see The Linux SCSI Generic (sg) HOWTO.
For more information on SCSI host codes, see Understanding SCSI host-side NMP errors/conditions in ESX 4.x and ESXi 5.x (1029039).

# SCSI Device/Status codes
Code Name

00h  GOOD
02h  CHECK CONDITION
04h  CONDITION MET
08h  BUSY
18h  RESERVATION CONFLICT
28h  TASK SET FULL
30h  ACA ACTIVE
40h  TASK ABORTED

For more information, see SCSI Status Codes.

# SCSI Sense Keys
Code Name

0h   NO SENSE
1h   RECOVERED ERROR
2h   NOT READY
3h   MEDIUM ERROR
4h   HARDWARE ERROR
5h   ILLEGAL REQUEST
6h   UNIT ATTENTION
7h   DATA PROTECT
8h   BLANK CHECK
9h   VENDOR SPECIFIC
Ah   COPY ABORTED
Bh   ABORTED COMMAND
Dh   VOLUME OVERFLOW
Eh   MISCOMPARE
For more information, see SCSI Sense Keys.


# SCSI Additional Sense Data
|-> ASC value (in hexadecimal)
||
|| |-> ASCQ value (in hexadecimal)
|| ||
|| || |-> Codes identifying devices that may use the ASC/ASCQ pair
|| || |-> value. (See list of device code letters below.)
|| || |
|| || | | |-> Error or exception indicated by the
|| || | | |-> ASC/ASCQ pair value.
|| || |------------| |------------------------------------------|

04/00 DTLPWROMAEBKVF LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
04/01 DTLPWROMAEBKVF LOGICAL UNIT IS IN PROCESS OF BECOMING READY
04/02 DTLPWROMAEBKVF LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
04/03 DTLPWROMAEBKVF LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
For more information, see SCSI Additional Sense Data.
