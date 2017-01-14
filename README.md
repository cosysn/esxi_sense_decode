# esxi_sense

注意：本文仅用于ESXi 5.0及以以后的版本，其他的版本暂不支持。

## 1. 引言
我们在定位ESXi或ESX出现的问题的时候，经常会在ESXi/ESX主机上的vmkernel.log或messages.log的系统日志文件中，见到类似下面的日志信息：
```
2011-04-04T21:07:30.257Z cpu2:2050)ScsiDeviceIO: 2315: Cmd(0x4124003edb00) 0x12, CmdSN 0x51 to dev "naa.600508e000000000c9f6baa7c19f6900" failed H:0x0 D:0x2 P:0x0 Valid sense data: 0x5 0x24 0x0. 
Mar 9 23:53:24 esx405 vmkernel: 2:14:39:54.069 cpu3:4300)ScsiDeviceIO: 1688: Command 0x28 to device "naa.60000970000292600219533031453245" failed H:0x1 D:0x0 P:0x3 Possible sense data: 0x0 0x0 0x0.
```
这些日志中记录了SCSI状态代码表示I/O以某种给定的状态失败。此状况对任何给定的工作负载而言可能是临时的、暂时性的、良性的或严重的，具体取决于收到的状态。如果能够从日志中记录的状态码中获得错误的原因，那么对于定位问题有很好的指导作用。

找出 SCSI 事件，并关注下面十六进制值：
```
H:0x0 D:0x2 P:0x0 Valid sense data: 0x5 0x24 0x0
```
这些SCSI状态代码表示I/O以某种给定的状态失败。此状况对任何给定的工作负载而言可能是临时的、暂时性的、良性的或严重的，具体取决于收到的状态。如果能够从日志中记录的状态码中找到错误的原因，那么对于定位问题有很大的帮助。

有关如何解析这些SCSI状态码，请参见[Interpreting SCSI sense codes in VMware ESXi and ESX (289902)](https://kb.vmware.com/selfservice/microsites/search.do?language=en_US&cmd=displayKC&externalId=289902) 

但是如果每次出现问题都要去查一下文档，不仅费时费力，效率还低。为了提高效率，我开发了esxi_sense工具用来解析SCSI感知代码， 省去了查询文档的麻烦。

## 2. 示例
如果在日志文件中打印了下面的日志信息：
```
2011-04-04T21:07:30.257Z cpu2:2050)ScsiDeviceIO: 2315: Cmd(0x4124003edb00) 0x12, CmdSN 0x51 to dev "naa.600508e000000000c9f6baa7c19f6900" failed H:0x0 D:0x2 P:0x0 Valid sense data: 0x5 0x24 0x0.
```
那么可以使用esxi_sense解析日志中的状态码。
```
root@localhost:/mnt/src# ./esxi_sense -h 0x0 -d 0x2 -p 0x0 -s 0x5 -a 0x24/0x0
==================== Host Status ======================
Code: [0x0]
Name: OK
Description: This status is returned when there is no error on the host side. This is when you see if there is a status for a device or plugin. This status is also when you see valid sense data instead of possible sense data.

==================== Device Status ======================
Code: [0x2]
Name: CHECK_CONDITION
Description: This status is returned when a command fails for a specific reason. When a CHECK CONDITION is received, the ESX storage stack will send out a SCSI command 0x3 (REQUEST SENSE) in order to get the SCSI sense data (Sense Key, Additional Sense Code, ASC Qualifier, and other bits). The sense data is listed after Valid sense data in the order of Sense Key, Additional Sense Code, and ASC Qualifier.

==================== Plugin Status ======================
Code: [0x0]
Name: GOOD
Description: No error.(ESXi 5.x / 6.x only)

==================== Sense Key ======================
Code: [0x5]
Name: ILLEGAL REQUEST

==================== Additional Sense Data ======================
Code: [0x24/0x0]
Description: INVALID FIELD IN CDB
```
可以看到上面解析了每个状态代码所代表的意思。


## 3. 使用帮助
Usgae: esxi_sense [OPTION]...
Interpreting SCSI sense codes in VMware ESXi and ESX

Options:
-h, --host                     Input the host status, the value must be an hex number.
-d, --device                   Input the device status, the value must be an hex number.
-p, --plugin                   Input the plugin status, the value must be an hex number.
-s, --sense-key                Input the sense key, the value must be an hex number.
-a, --additional-sense-data    Input the additional sense data, it is an ASC/ASCQ pair value.
    --help                     Display this help and exit
-v, --version                  Display version information and exit


-h或—host，用来解析SCSI主机代码，这些代码可能来自主机适配器上的固件或来自适配器驱动冲虚控制的若干主机之一。
有关SCSI主机代码的详细信息，请参见 [Understanding SCSI host-side NMP errors/conditions in ESX 4.x and ESXi 5.x (1029039)](https://kb.vmware.com/selfservice/search.do?cmd=displayKC&amp;docType=kc&amp;docTypeID=DT_KB_1_1&amp;externalId=1029039)。

-d或—device，用来解析SCSI设备/状态代码，SCSI状态代码表示命令处理完成之后的状态，代码值由T10委员会指定。有关详细信息，请参见 [SCSI Status Codes](http://www.t10.org/lists/2status.htm)。
而有关 SCSI 设备代码的其他信息，请参见 [Understanding SCSI device/target NMP errors/conditions in ESX/ESXi 4.x and ESXi 5.0 (1030381)](https://kb.vmware.com/selfservice/search.do?cmd=displayKC&amp;docType=kc&amp;docTypeID=DT_KB_1_1&amp;externalId=1030381)。

-p或—plugin，用来解析VMware插件传输代码，有关 SCSI 插件 NMP 条件和错误代码的列表，请参见 [Understanding SCSI plug-in NMP errors/conditions in ESX/ESXi 4.x/5.x/6.0 (2004086)](https://kb.vmware.com/selfservice/search.do?cmd=displayKC&amp;docType=kc&amp;docTypeID=DT_KB_1_1&amp;externalId=2004086)。

-s或—sense-key，用来解析SCSI感知秘钥，SCSI感知秘钥显示在命令返回CHECK CONDITION状态时可用的感知数据中。感知密钥包含了解命令失败原因所必需的所有信息。
有关详细信息，请参见 [SCSI Sense Keys](http://www.t10.org/lists/2sensekey.htm)。

-a 或--additional-sense-data，用来解析SCSI其他感知数据，其是由两个字节组成，通常由REQUEST SENSE命令返回。其他感知代码 (ASC) 字节说明感知密钥字段中报告的错误异常的相关信息。其他感知代码限定符 (ASCQ) 表示与其他感知代码相关的详细信息。
```
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
```
有关详细信息，请参见 [SCSI Additional Sense Data](http://www.t10.org/lists/2asc.htm)。

## 4. 参考文献
[1] [Interpreting SCSI sense codes in VMware ESXi and ESX (289902)](https://kb.vmware.com/selfservice/microsites/search.do?language=en_US&amp;cmd=displayKC&amp;externalId=289902)

[2] [Understanding SCSI host-side NMP errors/conditions in ESX 4.x and ESXi 5.x (1029039)](https://kb.vmware.com/selfservice/search.do?cmd=displayKC&amp;docType=kc&amp;docTypeID=DT_KB_1_1&amp;externalId=1029039)

[3] [SCSI Status Codes](http://www.t10.org/lists/2status.htm)

[4] [Understanding SCSI device/target NMP errors/conditions in ESX/ESXi 4.x and ESXi 5.0 (1030381)](https://kb.vmware.com/selfservice/search.do?cmd=displayKC&amp;docType=kc&amp;docTypeID=DT_KB_1_1&amp;externalId=1030381)

[5] [Understanding SCSI plug-in NMP errors/conditions in ESX/ESXi 4.x/5.x/6.0 (2004086)](https://kb.vmware.com/selfservice/search.do?cmd=displayKC&amp;docType=kc&amp;docTypeID=DT_KB_1_1&amp;externalId=2004086)

[6] [SCSI Sense Keys](http://www.t10.org/lists/2sensekey.htm)

[7] [SCSI Additional Sense Data](http://www.t10.org/lists/2asc.htm)

