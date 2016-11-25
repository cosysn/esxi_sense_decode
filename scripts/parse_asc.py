#!/usr/bin/python
import os

if __name__ == '__main__':
    file = open(r'../docs/asc-alph.txt')
    lines = file.readlines()[21:]
    file.close()

    asc_struct = open(r'./asc.h', 'w')
    asc_struct.write('static sense_reason_t asc_ascq_list[] = \n{\n')
    for line in lines:
        try:
            asc_ascq = line[:8].split('/', 1)
            asc = int(asc_ascq[0][:2], 16)
            ascq = int(asc_ascq[1][:2], 16)

            desc = line[24:-2]

            str = '    {ASC_ASCQ_TO_INT(0x%02x, 0x%02x), "", "%s"},\n' % (asc, ascq, desc)
            asc_struct.write(str)
        except ValueError:
            print line
	
    asc_struct.write('    {ASC_ASCQ_TO_INT(0xff, 0xff), "", "Additional Sense Data unknown"},\n')
    asc_struct.write('};\n')
    asc_struct.close()
