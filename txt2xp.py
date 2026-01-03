#
# translate a plain text file into the REXPaint file format
#
# NOTE: seems to come out rotated -90 for some reason
# writing order is off? idk
import struct

INFILE = 'coffee.txt'
OUTFILE = 'coffee.xp'

def doit():
    fp = open(INFILE)
    lines = fp.readlines()
    fp.close()

    with open(OUTFILE, 'wb') as fp:
        fp.write(struct.pack('i', 1))
        fp.write(struct.pack('i', 1))
        fp.write(struct.pack('i', len(lines[0])))
        fp.write(struct.pack('i', len(lines)))
        for line in lines:
            for ch in line:
                fp.write(struct.pack('i', ord(ch)))
                fp.write(struct.pack('BBBBBB', 255,255,255,0,0,0))



if __name__ == '__main__':
    doit()