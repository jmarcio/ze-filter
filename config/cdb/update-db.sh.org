#! /bin/bash

RsyncSrv=rsync://foss.jose-marcio.org:1873

cd /var/ze-filter/cdb

# Create empty files if they aren't there
TXT="ze-bayes.txt ze-urlbl.txt ze-rcpt.txt"
for ftxt in $TXT
do
  [ -f $ftxt ] || touch $ftxt
done

#
# do all stuff
while read line
do
  echo $line | grep -q "^\s*$"  && continue
  echo $line | grep -q "#"  && continue
  echo "  OK : $line"
  ${line} > /dev/null || break
done <<EOT
# Getting databases from server
rsync -aq rsync://foss.jose-marcio.org:1873/urlbl/ze-urlbl.txt      .
rsync -aq rsync://foss.jose-marcio.org:1873/ze-toolbox/ze-bayes.txt .
rsync -aq rsync://foss.jose-marcio.org:1873/lr-data/                lr-data/
# Check links
rm -f ze-lr.txt
ln -s lr-data/ze-lr.txt ze-lr.txt
# Finally, make...
make
EOT

