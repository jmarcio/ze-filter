

j-chkmail 2.0.0
***************

* What's new ?
**************

  j-chkmail version 1.14.0 was renamed to 2.0.x  !!!

  This is a milestone in the developpement of j-chkmail. This milestone
  consolidates all changes since j-chkmail 1.0 (long time ago) and add
  some few bug fixes, changes and features since 1.13 version.

* Changes
*********
  
Some main changes in this release are :

1. More debugging on j-greyd and initial code to synchronize two jgreyd
   servers

2. Checks on very short messages (body length < some value) without MIME 
   parts, to detect address verification messages.

   These feature can be configured with the options :

        REJECT_SHORT_BODIES             YES
        MIN_BODY_LENGTH                 10

   Messages flaged are those which boby size is shorter than MIN_BODY_LENGTH
   characters, come from an UNKNOWN network. 

   Exceptions can be defined at j-policy database, using the following 
   prefixes and usual policy database logic :
      RejectShortMsgsConnect
      RejectShortMsgsFrom
      RejectShortMsgsTo

   Messages sent to mailing list managers (subscribe, unsubscribe, ...)
   aren't rejected.

3. Discard Delivery-Notification-Request headers. To enable this feature,
   set the following option :

    DROP_DELIVERY_NOTIFICATION_REQUEST   YES

4. Date validity verification was splited into two options :
     CHECK_DATE_IN_FUTURE
     CHECK_DATE_IN_PAST

5. Bugs corrected :
   - authenticated connections hadn't the NetworkClass defined as it should
     be : overriding previously detected netclass
   - if configured, DNS URLBL was queried even if the message body matched
     Berkeley DB URLBL 
   - Archived messages were saved with extension '.unknown' instead of
     '.archive'
   - ordering at which j-policy.*.txt files are taken into account to
     build j-policy.txt file and j-policy.db database

6. Bundled PCRE library updated to 7.8     

7. Configuration files updating
   Some configuration data shall be updated from time to time, even if
   you don't update the filter program. These files are available at
         http://j-chkmail.ensmp.fr/data/conf-data
   You're urged to update these files from time to time (say one or
   two times a month).
   Script tools/update-conf-data can be used to get files to be 
   updated. This script is also available at the same place mentioned
   above.

8. contrib/scripts updated   

9. get-iana, a script to update the list of networks unassigned by
   IANA is installed at /var/jchkmail/bin directory. To use it, you
   shall, from time to time, do :
     cd /var/jchkmail/cdb
     ../bin/get-iana > j-policy.z-iana.txt
     make    
 
10. j-ndc has now a configuration file

11. j-makemap default mode was changed to "erase and skip". This means
    content of original database is erased before updating an existing
    database, and when the same record (same key) is found multiple times,
    only the first one is taken into account.
    The Makefile used to build databases at the constant database directory
    shall be updated to take this into account.

12. Default Greylisting value defined at j-policy.z-defaults.txt file 
    changed from :
        GreyCheckTo:default                     YES
    to
        GreyCheckConnect:default                YES

13. Changes to adapt to new protocol used by Clamd 0.95

14. Some verbosity reduction - if you don't want j-chkmail to be verbose,
    define "notice" log level at syslog.conf and level 9 at j-chkmail.cf.
    Detail of what was found in a message is logged with priority "info"

15. Added option (DEFAULT_BAD_MX_REPLY) to define the reply when the sender
    address is unreacheable : the sender domain don't have nor an Mx nor
    an IP address associated to it. Possible values are :
        
16. j-ndc sends a "QUIT" command to the server before exiting. This is
    necessary when using j-ndc with j-greyd server.

17. .stok and .htok suffixes added to bayes-toolbox/Makefile in order to
    allow spam and ham mailboxes with the same basename.

 
