#! /usr/bin/python3

import os, sys
import re
from stat import *

def walktree(top):
  '''recursively descend the directory tree rooted at top,
     calling the callback function for each regular file'''

  for f in os.listdir(top):
    if re.match("^(\.git|tmp)$", f):
      print("SKIP  skiping {:s}".format(f))
      continue
    pathname = os.path.join(top, f)
    mode = os.stat(pathname)[ST_MODE]
    if S_ISDIR(mode):
      # It's a directory, recurse into it
      walktree(pathname)
    elif S_ISREG(mode):
      mtime = os.stat(pathname)[ST_MTIME]
      print("%d   %s"  %(mtime, pathname))
    else:
      # Unknown file type, print a message
      print('Skipping %s' % pathname)

if __name__ == '__main__':
  if len(sys.argv) > 1:
    walktree(sys.argv[1])
  else:
    walktree('.') 