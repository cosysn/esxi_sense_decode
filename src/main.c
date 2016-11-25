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
void decode_host_status(int host)
{
	int i = 0, len = 0;
    sense_reason_t *reason = NULL;

	len = sizeof(host_status_list) / sizeof(host_status_list[0]);
	for (i = 0; i < len-1; i++) {
		if (host_status_list[i].code == host)
			break;
	}
	
    reason = &host_status_list[i];
    printf("==================== Host Status ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
    printf("Description: %s\n\n", reason->desc);
}

void decode_device_status(int device)
{
	int i = 0, len = 0;
    sense_reason_t *reason = NULL;
    
	len = sizeof(device_status_list) / sizeof(device_status_list[0]);
    for (i = 0; i < len-1; i++) {
        if (device_status[i].code == device)
            break;
    }

    reason = &device_status[i];
    if (i == len-1) {
        reason->code = device;
    }
    printf("==================== Device Status ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
    printf("Description: %s\n\n", reason->desc);
}

void decode_plugin_status(int plugin)
{
	int i = 0, len = 0;
    sense_reason_t *reason = NULL;
    
	len = sizeof(plugin_status_list) / sizeof(plugin_status_list[0]);
    for (i = 0; i < len-1; i++) {
        if (plugin_status_list[i].code == plugin)
            break;
    }

    reason = &plugin_status_list[i];
    if (i == len-1) {
        reason->code = plugin;
    }
    printf("==================== Plugin Status ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
    printf("Description: %s\n\n", reason->desc);
}

void decode_sense_key(int sense_key)
{
	int i = 0, len = 0;
    sense_reason_t *reason = NULL;
    
	len = sizeof(sense_keys_list) / sizeof(sense_keys_list[0]);
    for (i = 0; i < len-1; i++) {
        if (sense_keys_list[i].code == sense_key)
            break;
    }

    reason = &sense_keys_list[i];
    if (i == len-1) {
        reason->code = sense_key;
    }
    printf("==================== Sense Key ======================\n");
    printf("Code: [0x%d]\n", reason->code);
    printf("Name: %s\n", reason->name);
}

void decode_asc_ascq(int asc, int ascq)
{
	int asc_ascq = 0;
	int i = 0, len = 0;
	sense_reason_t *reason = NULL;
	
	asc_ascq = ASC_ASCQ_TO_INT(asc, ascq);
	len = sizeof(asc_ascq_list) / sizeof(asc_ascq_list[0]);
	for (i = 0; i < len-1; i++) {
		if (asc_ascq_list[i].code == asc_ascq)
			break;
	}
	
	reason = &asc_ascq_list[i];
	if (i == len-1) {
		reason->code = asc_ascq;
	}
	
	printf("==================== Additional Sense Data ======================\n");
    printf("Code: [0x%x/0x%x]\n", asc, ascq);
    printf("Description: %s\n", reason->desc);
}

int main(int argc, char **argv)
{
    int ret = 0;
    int idx = 0, opt = 0;
    int host_flag = 0, device_flag = 0, plugin_flag = 0, 
        sense_flag = 0, asd_flag = 0; 
    static int help_flag;
    int host = 0, device = 0, plugin = 0, sense_key = 0, asc = 0, ascq = 0;
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
	
	ret = ASC_ASCQ_TO_INT(0x20, 0x0b);
	printf("ret = %x\n", ret);

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
				asd_flag = 1;
				if (sscanf(optarg, "%x/%x", &asc, &ascq) != 2) {
					return -1;
				}
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
	
	if (asd_flag) {
		decode_asc_ascq(asc, ascq);
	}
	
    return ret;
}

