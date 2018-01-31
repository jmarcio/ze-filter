#!/bin/bash
# shell script 'learn'
# Learn ham and spam wrapper script
#
# Author:  Thomas Spahni
# License: public domain, no warranty
# Version: 1.0 (initial release of 2009-08-02)
#
# Requires:
# Jose-Marcio Martins da Cruz' ze-filter
# (see: http://foss.jose-marcio.org/wiki/doku.php/start)
# and formail (which is part of the procmail package).
#
# This script is used for training the bayes classifier
# with ze-filter. It has been tested with pine as mail client.
# The first message of each folder created with pine will
# contain folder internal data and should not be deleted.
# For other clients not using the first message for
# internal purposes change the user settings to 'no'.
#
# How to use:
# Use your mail client to sort all messages to be trained into 
# two folders, one for ham and another one for spam.
# You may choose other names for these folders than 'corpusham'
# and 'corpusspam' but don't forget to adjust the variables below
# accordingly.
#
# Running 'learn' (no arguments) will copy sorted messages to the
# appropriate mailboxes in $DESTPATH and rotate files at the
# presettable size threshold to keep processing times low.
# It launches the commands to process the ham and spam messages
# and to load them into the ze-filter database.
#
# How to install:
# 'learn' should be installed in a private path of the user and
# made executable for this user. $HOME/bin is a good place, if this
# is within the user's path.
#
# A small companion script named 'ze-bayes-rebuild-db' has to be
# installed and the user must have permission to execute it with 
# sudo as root. See the code at the end of this script.
#
#####################
# User Configuration:
#####################
LOCAL_HAM_FOLDER="corpusham"
LOCAL_SPAM_FOLDER="corpusspam"
LOCAL_MAILDIR="Mail"
JCHKMAILPATH="/var/ze-filter"
TOOLBOX_DIR="bayes-toolbox"
# Maximum size of mailbox in
# bytes before we rotate:
MAXSIZE=30000000
FIRST_MESSAGE_IS_INTERNAL="yes"
###########################
# End of User Configuration
###########################

# Set paths and options
DESTPATH="$JCHKMAILPATH/$TOOLBOX_DIR"
MAILPATH="$HOME/$LOCAL_MAILDIR"
HFOLDERS="$MAILPATH/$LOCAL_HAM_FOLDER"
SFOLDERS="$MAILPATH/$LOCAL_SPAM_FOLDER"
if test $FIRST_MESSAGE_IS_INTERNAL == yes ; then
	FORMAILOPT="+1"
else
	FORMAILOPT=""
fi

# Rotate those mailbox files when they grow too large
HBOXSIZE=$(find $DESTPATH -name corpus-ham.hbox -printf '%s\n')
if test $(($HBOXSIZE)) -gt $(($MAXSIZE)) ; then
	# get the next free running number
	LASTNUM=$(find $DESTPATH -name corpus-ham.\*.hbox | sed \
		-e "s/\\.hbox$//" \
		-e "s/^.*corpus-ham\\.//" \
		-e "s/^0*//" | sort -n | tail -n 1)
	if test -z "$LASTNUM" ; then
		LASTNUM=0
	fi
	NEXTNUM=$(($LASTNUM + 1))
	NEXTNUM=$(echo "00000$NEXTNUM" | sed \
		-e "s/^\\(0*\\)\\([0-9][0-9][0-9][0-9]\\)$/\\2/")
	mv $DESTPATH/corpus-ham.hbox $DESTPATH/corpus-ham.$NEXTNUM.hbox
	mv $DESTPATH/corpus-ham.tok $DESTPATH/corpus-ham.$NEXTNUM.tok
fi

SBOXSIZE=$(find $DESTPATH -name corpus-spam.sbox -printf '%s\n')
if test $(($SBOXSIZE)) -gt $(($MAXSIZE)) ; then
	# get the next free running number
	LASTNUM=$(find $DESTPATH -name 'corpus-spam.*.sbox' | sed \
		-e "s/\\.sbox$//" \
		-e "s/^.*corpus-spam\\.//" \
		-e "s/^0*//" | sort -n | tail -n 1)
	if test -z "$LASTNUM" ; then
		LASTNUM=0
	fi
	NEXTNUM="$(($LASTNUM + 1))"
	NEXTNUM=$(echo "00000$NEXTNUM" | sed \
		-e "s/^\\(0*\\)\\([0-9][0-9][0-9][0-9]\\)$/\\2/")
	mv $DESTPATH/corpus-spam.sbox $DESTPATH/corpus-spam.$NEXTNUM.sbox
	mv $DESTPATH/corpus-spam.tok $DESTPATH/corpus-spam.$NEXTNUM.tok
fi

# Create new mailbox file after rotation and for the first time
# when the script is run.
if ! test -w $DESTPATH/corpus-ham.hbox ; then
	touch $DESTPATH/corpus-ham.hbox
fi

if ! test -w $DESTPATH/corpus-spam.sbox ; then
	touch $DESTPATH/corpus-spam.sbox
fi

NEWADD=no
for FOLDER in $HFOLDERS ; do
	# make sure there is at least one message
	MSG=$(cat $FOLDER | formail $FORMAILOPT -1 -s)
	if test -n "$MSG" ; then
	    NEWADD=yes
	    echo "ze-filter: adding $(basename $FOLDER)  to corpus-ham.hbox"
	    cat $FOLDER | formail $FORMAILOPT -b -c -s \
		>> $DESTPATH/corpus-ham.hbox
	fi
done

# delete copied ham messages from local folder
if test $NEWADD == yes ; then
	if test $FIRST_MESSAGE_IS_INTERNAL == yes ; then
		cat $MAILPATH/$LOCAL_HAM_FOLDER | formail -1 -s \
		> $MAILPATH/$LOCAL_HAM_FOLDER.tmp
	else
		touch $MAILPATH/$LOCAL_HAM_FOLDER.tmp
	fi
	chmod u=rw,g-r,o-r $MAILPATH/$LOCAL_HAM_FOLDER.tmp
	rm -f $MAILPATH/$LOCAL_HAM_FOLDER
	mv $MAILPATH/$LOCAL_HAM_FOLDER.tmp $MAILPATH/$LOCAL_HAM_FOLDER
fi

# Same for spam
for FOLDER in $SFOLDERS ; do
	# make sure there is at least one message
	MSG=$(cat $FOLDER | formail $FORMAILOPT -1 -s)
	if test -n "$MSG" ; then
	    NEWADD=yes
	    echo "ze-filter: adding $(basename $FOLDER) to corpus-spam.sbox"
	    cat $FOLDER | formail $FORMAILOPT -b -c -s \
		>> $DESTPATH/corpus-spam.sbox
	fi
done

# delete copied spam messages from local folder
if test $NEWADD == yes ; then
	if test $FIRST_MESSAGE_IS_INTERNAL == yes ; then
		cat $MAILPATH/$LOCAL_SPAM_FOLDER | formail -1 -s \
		> $MAILPATH/$LOCAL_SPAM_FOLDER.tmp
	else
		touch $MAILPATH/$LOCAL_SPAM_FOLDER.tmp
	fi
	chmod u=rw,g-r,o-r $MAILPATH/$LOCAL_SPAM_FOLDER.tmp
	rm -f $MAILPATH/$LOCAL_SPAM_FOLDER
	mv $MAILPATH/$LOCAL_SPAM_FOLDER.tmp $MAILPATH/$LOCAL_SPAM_FOLDER
fi

# Rebuild database for ze-filter
if test $NEWADD == yes ; then
	(cd $JCHKMAILPATH/$TOOLBOX_DIR && make)
	#
	# Script 'ze-filter-rebuild-db' will copy from bayes-toolbox 
	# directory to the cdb directory and execute make.
	# This must be done as user root.
	# Install the following script in the cdb directory, chown it
	# to user root and group root and make it executable for root.
	# Add the following line to /etc/sudoers using the 'visudo'
	# command as root (example only!):
	# usrname ALL = (root) NOPASSWD: /var/ze-filter/cdb/ze-bayes-rebuild-db
	#
	# Note: 'usrname' in the line above is your user name.
	# ze-filter-rebuild-db looks like this (adapt paths to your needs):
	#-----------------------------------------------------------
	# #!/bin/bash
	# cp /var/ze-filter/bayes-toolbox/ze-bayes.txt \
	#    /var/ze-filter/cdb/ze-bayes.txt
	# (cd /var/ze-filter/cdb && make)
	# exit 0
	#-----------------------------------------------------------
	sudo $JCHKMAILPATH/cdb/ze-bayes-rebuild-db
fi

exit 0
