#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "esd.h"

/**
 * For a list of SCSI plug-in NMP conditions and error codes, see Understanding SCSI plug-in NMP errors/conditions in ESX/ESXi 4.x/5.x/6.0 (2004086) 
 *    https://kb.vmware.com/selfservice/search.do?cmd=displayKC&docType=kc&docTypeID=DT_KB_1_1&externalId=2004086
 * For more information on SCSI host codes, see Understanding SCSI host-side NMP errors/conditions in ESX 4.x and ESXi 5.x (1029039).
 *    https://kb.vmware.com/selfservice/search.do?cmd=displayKC&docType=kc&docTypeID=DT_KB_1_1&externalId=1029039
 *    http://tldp.org/HOWTO/SCSI-Generic-HOWTO/x291.html
 *    http://tldp.org/HOWTO/SCSI-Generic-HOWTO/sg3_utils.html
 * For additional information on SCSI device codes, see Understanding SCSI device/target NMP errors/conditions in ESX/ESXi 4.x and ESXi 5.0 (1030381).
 *    https://kb.vmware.com/selfservice/search.do?cmd=displayKC&docType=kc&docTypeID=DT_KB_1_1&externalId=1030381
 * For more information, see SCSI Sense Keys.
 *    http://www.t10.org/lists/2sensekey.htm
 */

static sense_reason_t host_status[] =
{
    {VMK_SCSI_HOST_OK, "OK", "This status is returned when there is no error on the host side. This is when you see if there is a status for a device or plugin. This status is also when you see valid sense data instead of possible sense data."},
    {VMK_SCSI_HOST_NO_CONNECT, "NO_CONNECT", "This status is returned if the connection is lost to the LUN. This can occur if the LUN is no longer visible to the host from the array side or if the physical connection to the array has been removed."},
    {VMK_SCSI_HOST_BUS_BUSY, "BUS_BUSY", "This status is returned when the HBA driver is unable to issue a command to the device. This status occurs due to dropped FCP frames in the environment."},
    {VMK_SCSI_HOST_TIMEOUT, "TIMEOUT", "This status is returned when the command in-flight to the array times out."},
    {VMK_SCSI_HOST_BAD_TARGET, "BAD_TARGET", "This status is returned after the driver abort commands to a bad target. Typically this status occurs when the target experiences a hardware error, butit can also occurif a command is sent to a bad target ID."},
    {VMK_SCSI_HOST_ABORT, "ABORT", "This status is returned if the driver aborts commands in-flight to the target. This occurs due to a command timeout or parity error in the frame."},
    {VMK_SCSI_HOST_PARITY, "PARITY", "This status is returned for generic errors. For example, this status occurs for events not covered by the other errors (such as data overrun or underrun)."},
    {VMK_SCSI_HOST_ERROR, "ERROR", "This status is returned when a device is reset due to a Storage Initiator Error. This typically occurs due to an outdated HBA firmware or possibly (though rarely) as the result of a bad HBA."},
    {VMK_SCSI_HOST_RESET, "RESET", "This status is returned when the HBA driver aborts I/O. It also occurs if the HBA resets the target."},
    {VMK_SCSI_HOST_BAD_INTR, "BAD_INTR", "This is a legacy error and never be returned."},
    {VMK_SCSI_HOST_PASSTHROUGH, "PASSTHROUGH", "This is a legacy error and never be returned. It was meant for a way for drivers to return an I/O that failed due to temporary conditions in the driver and that I/O be retried."},
    {VMK_SCSI_HOST_SOFT_ERROR, "SOFT_ERROR", "This status is returned if the HBA driver returns a DID_REQUEUE command. Upon receiving this status, the I/O command is reissued immediately."},
    {VMK_SCSI_HOST_RETRY, "RETRY", "This status is returned due to a transient error. When this status is returned, the I/O command is requeued and issued again."},
    {VMK_SCSI_HOST_REQUEUE, "REQUEUE", "This status is returned when the HBA driver tries to abort a command which then sets the IOSTAT_LOCAL_REJECT status on all commands in the iocb ring. This causes the original command to requeue."}
};

void decode_host_status(int host)
{
    sense_reason_t *reason = NULL;

    reason = &host_status[host];
    printf("==================== Host Status ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
    printf("Description: %s\n\n", reason->desc);
}

static sense_reason_t device_status[] =
{
    {VMK_SCSI_DEVICE_GOOD, "GOOD", "This status is returned when there is no error from the device or target side. This is when you will see if there is a status for Host or Plugin."},
    {VMK_SCSI_DEVICE_CHECK_CONDITION, "CHECK_CONDITION", "This status is returned when a command fails for a specific reason. When a CHECK CONDITION is received, the ESX storage stack will send out a SCSI command 0x3 (REQUEST SENSE) in order to get the SCSI sense data (Sense Key, Additional Sense Code, ASC Qualifier, and other bits). The sense data is listed after Valid sense data in the order of Sense Key, Additional Sense Code, and ASC Qualifier."},
    {VMK_SCSI_DEVICE_CONDITION_MET, "CONDITION_MET", "This status is returned for successful completion of conditional commands, such as PREFETCH."},
    {VMK_SCSI_DEVICE_BUSY, "BUSY", "This status is returned when a LUN cannot accept SCSI commands at the moment. As this should be a temporary condition, the command is tried again."},
    {VMK_SCSI_DEVICE_INTERMEDIATE, "INTERMEDIATE", "This status is the same as 0x0 (GOOD) and is returned for a command that was part of a series of linked commands."},
    {VMK_SCSI_DEVICE_INTERMEDIATE_CONDITION_MET, "INTERMEDIATE_CONDITION_MET", "This status is the combination of device statuses CONDITION MET (0x4) and INTERMEDIATE (0x10)."},
    {VMK_SCSI_DEVICE_RESERVATION_CONFLICT , "RESERVATION CONFLICT", "This status is returned when a LUN is in a Reserved status and commands from initiators that did not place that SCSI reservation attempt to issue commands to it."},
    {VMK_SCSI_DEVICE_COMMAND_TERMINATED, "COMMAND_TERMINATED", "Obsolete status code. Was originally returned as a result of a TERMINATE I/O message."},
    {VMK_SCSI_DEVICE_QUEUE_FULL, "TASK_SET_FULL", "This status is returned when the LUN prevents accepting SCSI commands from initiators due to lack of resources, namely the queue depth on the array. Adaptive queue depth code was introduced into ESX 3.5 U4 (native in ESX 4.x) that adjusts the LUN queue depth in the VMkernel. If configured, this code will activate when device status TASK SET FULL (0x28) is return for failed commands and essentially throttles back the I/O until the array stops returning this status."},
    {VMK_SCSI_DEVICE_ACA_ACTIVE, "ACA_ACTIVE", "This status is returned when an Auto Contingent Allegiance (ACA) has been aborted by another SCSI initiator. This status has been observed on arrays running older/unsupported firmware with ESX 4.1, specifically because the firmware does not support VAAI commands."},
    {VMK_SCSI_DEVICE_TASK_ABORTED, "TASK ABORTED", "This status is returned when a command has been aborted by another SCSI initiator and the TASK ABORT STATUS bit is set to '1'. So far this status has only been returned by iSCSI arrays."},
    {VMK_SCSI_DEVICE_UNKNOWN, "UNKNOWN", "Device Status Code unknown."},
    
};

void decode_device_status(int device)
{
    sense_reason_t *reason = NULL;
    int i = 0;

    for (i = 0; i < VMK_SCSI_DEVICE_NUM-1; i++) {
        if (device_status[i].code == device)
            break;
    }
    printf("i = %d\n", i);

    reason = &device_status[i];
    if (i == VMK_SCSI_DEVICE_NUM-1) {
        reason->code = device;
    }
    printf("==================== Device Status ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
    printf("Description: %s\n\n", reason->desc);
}


static sense_reason_t plugin_status[] =
{
    {VMK_SCSI_PLUGIN_GOOD, "GOOD", "No error.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_TRANSIENT, "TRANSIENT", "An unspecified error occurred. Note: The I/O cmd should be tried.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_SNAPSHOT, "SNAPSHOT", "The device is a deactivated snapshot. Note: The I/O cmd failed because the device is a deactivated snapshot and so the LUN is read-only.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_RESERVATION_LOST, "RESERVATION_LOST", "SCSI-2 reservation was lost.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_REQUEUE, "REQUEUE", "The plug-in wants to requeue the I/O back. Note: The I/O will be retried.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_ATS_MISCOMPARE, "ATS_MISCOMPARE", "The test and set data in the ATS request returned false for equality.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_THINPROV_BUSY_GROWING, "THINPROV_BUSY_GROWING", "Allocating more thin provision space. Device server is in the process of allocating more space in the backing pool for a thin provisioned LUN.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_THINPROV_ATQUOTA , "THINPROV_ATQUOTA", "Thin provisioning soft-limit exceeded.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_THINPROV_NOSPACE, "THINPROV_NOSPACE", "Backing pool for thin provisioned LUN is out of space.(ESXi 5.x / 6.x only)"},
    {VMK_SCSI_PLUGIN_UNKOWN, "UNKNOWN", "Device Status Code unknown."},
};

void decode_plugin_status(int plugin)
{
    sense_reason_t *reason = NULL;
    int i = 0;

    for (i = 0; i < VMK_SCSI_PLUGIN_NUM-1; i++) {
        if (plugin_status[i].code == plugin)
            break;
    }
    printf("i = %d\n", i);

    reason = &plugin_status[i];
    if (i == VMK_SCSI_PLUGIN_NUM-1) {
        reason->code = plugin;
    }
    printf("==================== Plugin Status ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
    printf("Description: %s\n\n", reason->desc);
}

static sense_reason_t sense_keys[] =
{
    {VMK_SCSI_SENSE_NO_SENSE, "NO SENSE", ""},
    {VMK_SCSI_SENSE_RECOVERED_ERROR, "RECOVERED ERROR", ""},
    {VMK_SCSI_SENSE_NOT_READY, "NOT READY", ""},
    {VMK_SCSI_SENSE_MEDIUM_ERROR, "MEDIUM ERROR", ""},
    {VMK_SCSI_SENSE_HARDWARE_ERROR, "HARDWARE ERROR", ""},
    {VMK_SCSI_SENSE_ILLEGAL_REQUEST, "ILLEGAL REQUEST", ""},
    {VMK_SCSI_SENSE_UNIT_ATTENTION, "UNIT ATTENTION", ""},
    {VMK_SCSI_SENSE_DATA_PROTECT , "DATA PROTECT", ""},
    {VMK_SCSI_SENSE_BLANK_CHECK, "BLANK CHECK", ""},
    {VMK_SCSI_SENSE_VENDOR_SPECIFIC, "VENDOR SPECIFIC", ""},
    {VMK_SCSI_SENSE_COPY_ABORTED, "COPY ABORTED", ""},
    {VMK_SCSI_SENSE_ABORTED_COMMAND, "ABORTED COMMAND", ""},
    {VMK_SCSI_SENSE_VOLUME_OVERFLOW, "VOLUME OVERFLOW", ""},
    {VMK_SCSI_SENSE_MISCOMPARE, "MISCOMPARE", ""},
    {VMK_SCSI_SENSE_COMPLETED, "COMPLETED", ""},
    {VMK_SCSI_PLUGIN_UNKOWN, "UNKNOWN", ""},
};

void decode_sense_key(int sense_key)
{
    sense_reason_t *reason = NULL;
    int i = 0;

    for (i = 0; i < VMK_SCSI_SENSE_NUM-1; i++) {
        if (sense_keys[i].code == sense_key)
            break;
    }
    printf("i = %d\n", i);

    reason = &sense_keys[i];
    if (i == VMK_SCSI_SENSE_NUM-1) {
        reason->code = sense_key;
    }
    printf("==================== Sense Key ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
}

int main(int argc, char **argv)
{
    int ret = 0;
    int idx = 0, opt = 0;
    int host_flag = 0, device_flag = 0, plugin_flag = 0, 
        sense_flag = 0, asd_flag = 0; 
    static int help_flag;
    int host = 0, device = 0, plugin = 0, sense_key = 0;
    const char *short_opts = "h:d:p:s:a:v";
    const static struct option long_opts[] =
    {
        {"host",                  required_argument, NULL, 'h'},
        {"device",                required_argument, NULL, 'd'},
        {"plugin",                required_argument, NULL, 'p'},
        {"sense-key",             required_argument, NULL, 's'},
        {"additional-sense-data", required_argument, NULL, 'a'},
        {"help",                  no_argument, &help_flag, 1},
        {"version",               no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    while((opt = getopt_long(argc, argv, short_opts, long_opts, &idx)) != -1) {
        switch (opt) {
            case 'h':
                host_flag = 1;
                host = strtol(optarg, NULL, 10);
                printf("opt h: %s\n", optarg);
                break;
            case 'd':
                device_flag = 1;
                device = strtol(optarg, NULL, 10);
                printf("opt d: %s\n", optarg);
                break;
            case 'p':
                plugin_flag = 1;
                plugin = strtol(optarg, NULL, 10);
                printf("opt p: %s\n", optarg);
                break;
            case 's':
                sense_flag = 1;
                sense_key = strtol(optarg, NULL, 10); 
                printf("opt s: %s\n", optarg);
                break;
            case 'a':
                printf("opt a: %s\n", optarg);
                break;
            case 'v':
                printf("opt v: %s\n", optarg);
                break;
            case '?':
                break;
            default:
                break;
        }
    }

    if (help_flag) {
        printf("help?\n");
    }

    if (host_flag) {
        decode_host_status(host);
    }

    if (device_flag) {
        decode_device_status(device);
    }

    if (plugin_flag) {
        decode_plugin_status(plugin);
    }
    
    if (sense_flag) {
        decode_sense_key(sense_key);
    }

    return ret;
}

