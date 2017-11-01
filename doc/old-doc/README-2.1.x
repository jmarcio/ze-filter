

j-chkmail 2.1.0
***************

* Changes
*********
  
Some main changes in this release are :

1. DNS RBWLs definition can now include negation of some codes. E.g., 
   the definiton :

   dnsbl.domain.com  ...; code=!127.0.1.0,all; ...

   means to accept  all return codes except 127.0.1.0.
   This is useful to avoid having to list all valid return codes except 
   some of them.

2.  DNS RBWLs can now be used to assign SMTP clients into known netclasses 
   (LOCAL, DOMAIN, FRIEND or OTHER).

3. Environment variables DB_CACHE_SIZE, DB_LK_MAX_LOCKS, DB_LK_MAX_LOCKERS
   and DB_LK_MAX_OBJECTS used to tune Berkeley DB working databases. This
   may be necessary on very busy servers.
     http://foss.jose-marcio.org/wiki/doku.php/doc:reference:envvars
   Consult BerkeleyDB documentation before changing these variables.

4. New configuration option GREY_REPLY allows one to change the default
   greylisting reply codes and message.

5. j-chkmail.cf main configuration file changes :

   * The order at which options appear were changed in order to improve
     understanding.
     
   * Options renamed 
     FD_FREE_SOFT            -> FD_FREE_SOFT_LIMIT
     FD_FREE_HARD            -> FD_FREE_HARD_LIMIT
     LOAD_CPU_IDLE_SOFT      -> CPU_IDLE_SOFT_LIMIT
     LOAD_CPU_IDLE_HARD      -> CPU_IDLE_HARD_LIMIT
     GREY_PENDING_NORMAL     -> GREY_MAX_PENDING_NORMAL
     GREY_PENDING_NULLSENDER -> GREY_MAX_PENDING_NULLSENDER

   * New options 
     GREY_REPLY 
     DELAY_REJECT

   * Options removed
     DUMP_THROTTLE
     DUMP_LOAD

6. PCRE library updated to release 7.9

7. Option DELAY_REJECT allow to report till the first SMTP MAIL command
   all reject decisions based on client behaviour (rate limits, too many
   errors, ...), when information about client authentication will be
   available.

8. Added VPATH definition to the Makefile installed at constant database
   directory. This way, when needed text files aren't found at the same
   directory, they'll be looked at $sysconfdir directory.

* Bugs
******

1. Assignement of codes returned by DNS RBWLs weren't correctly checked 
   when only some codes where to be selected

2. When telling j-chkmail to remove old headers (REMOVE_OLD_SCORES)
   X-j-chkmail-Scores weren't removed

3. j-easy-install error when creating default text files inside cdb dir

4. Unsetting recurse option didn't work on DNS URLBL checkings.

5. Configuration option DB_CACHE_SIZE wasn't correctly taken into account
   by the filter, and some other BerkeleyDB tuning values shall be defined
   on huge servers (this problem appeared with j-greyd).

6. Sometimes (but not allways) one thread of j-greyd could end in a endless 
   loop consumming all cycles of a CPU when the client didn't disconnected 
   cleanly. Adding environment variables to tune it. Either way, BerkeleyDB
   configuration file can be used to tune BerkeleyDB databases behaviour.

7. Timeout of client side of j-greyd was too low and error handling not
   adequate, resulting in data garbling (out of sync). Environment variables
   added to allow fine tuning on huge servers.

8. Greylisting database of pending entries with too many entries coming from
   the same client could prevents expire background task from working as
   intended. "Too many" means something like a hundred thousand entries.

9. Limiting the number of pending entries, per SMTP client, didn't
   work before.

10. A possible race condition could happen when reopening j-urlbl or 
   j-bayes databases.

11. make upgrade didn't stop the filter. Problem pointed out by William
    Montgomery

12. Pattern matching done elsewhere than in body part of messages weren't
    taken into account when the body size was bigger than the maximum
    defined to check body size. Pb pointed out by Xavier.

13. Added "autoupdate" option to j-easy-install, in order to autoupdate
    j-easy-install tool.




j-chkmail 2.1.1
***************

* Changes
*********

1. Corrected bug when compiling j-chkmail under FreeBSD 7.2


 
