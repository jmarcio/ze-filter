

j-chkmail 2.2.0
***************

* Changes
*********
  
Some main changes in this release are :

1. External libraries upgrade
   PCRE       library updated to 8.01
   BerkeleyDB library updated to 4.8.26

2. Renamed options :
   DELAY_REJECT             -> DELAY_CHECKS
   CHECK_BADEHLO            -> REJECT_BADEHLO
   CHECK_BAD_NULL_SENDER    -> REJECT_BAD_NULL_SENDER
   CHECK_BAD_SENDER_MX      -> REJECT_BAD_SENDER_MX
   CHECK_DATE_IN_PAST       -> REJECT_DATE_IN_PAST
   CHECK_DATE_IN_FUTURE     -> REJECT_DATE_IN_FUTURE
   REMOVED_OLD_SCORES       -> REMOVE_SCORES
   REMOVED_OLD_HEADERS      -> REMOVE_HEADERS

3. Changed options :
   * BADEHLO_CHECKS - Regex check added. Valid values are a 
     comma separated list of checks to do on EHLO parameter :
       InvalidChar, ForgedIP, NonBracketedIP, NonFQDN,
       IdentityTheft, Regex, All, None
     "All" means all possible checks and "None" means no check
     at all.
     Remarks : 
     * these checks are done only on unknown SMTP clients
     * scores evaluated on EHLO checks are no more added to
       message content score
   
4. Removing existing headers (Scores or general headers) code was 
   rewritten.
   Options REMOVED_OLD_SCORES and REMOVE_OLD_HEADERS were renamed 
   to REMOVE_SCORES and REMOVE_HEADERS, with a slightly different 
   behaviour.

5. Adding handling times measurement per callback and reporting 
   with "j-ndc stats htimes". Useful for debugging and tuning.

6. Added SMTP server address and listening port number to daemon
   field at j-chkmail log lines. This helpful in a cluster (multiple
   SMTP servers) environnement using a shared filter, to associate 
   log lines and SMTP servers.

7. Added some configure/compile options to port to MacOS X server.
   Based on feedback from Dennis Peterson.

* Bugs
******

1. A race condition when expiring/cleaning up greylisting databases
   both in the j-chkmail and j-greyd.

2. A recursion problem when self referenced "GreyEquivDomain" records
   where found inside j-policy database. This bug could appear in
   some particular situations causing an endless loop ending with
   a filter crash.

      
