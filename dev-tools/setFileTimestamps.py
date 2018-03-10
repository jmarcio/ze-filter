#! /usr/bin/python3

import fileinput
import os

def process(line):
  s = line.split()
  if s[0] == 'SKIP':
    return
  print('* Handling {:s} - {:s}'.format(s[0], s[1]))
  ts = int(s[0])
  fin = s[1]
  if (os.access(fin, os.R_OK)):
    os.utime(fin, (ts, ts))
  else:
    print(" ERROR : {:s} doesn't exist or access denied".format(fin))

if __name__ == "__main__":
  for line in fileinput.input():
    process(line.rstrip())
else:
  print("This isn't the main program")



