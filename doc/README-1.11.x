

j-chkmail 1.11.0
****************

Some main changes in this release are :

1. FILE_EXT and FILE_REGEX configuration options don't exist anymore
   You shall move configuration done with these options to the j-xfiles 
   configuration file.

2. Oracle checks were cleaned up. Some tests generating false
   positives of detecting few spams were removed

3. j-nets configuration file is considered deprecated (even if it's still
   there for some time - it will be removed in the future).
   You shall use j-policy NetClass prefix to define known networks :

      NetClass:10	     	LOCAL
      NetClass:199.201      	DOMAIN
      NetClass:1.2.3.4      	FRIEND
      NetClass:domain.com   	DOMAIN

   As you can note, NetClass prefix can also be used with domain names.

   j-chkmail predefined classes are LOCAL, DOMAIN and FRIEND, but you
   can define others, if you want: E.g.

      NetClass:10.1             DEPMATH
      NetClass:10.2             DEPPHYS
      NetClass:10.3             DEPCHEM
      ...

   If you want to remain, for some time, with the old behaviour, you
   shall configure j-chkmail to do that :

       ./configure CPPFLAGS="-DPRE_110_CONF=1"

4. The following configuration options from file j-chkmail.cf are being
   considered deprecated :

   CONN_RATE_FROM_DOMAIN      200
   CONN_RATE_FROM_LOCAL       300
   CONN_RATE_FROM_FRIEND      30
   CONN_RATE_FROM_UNKNOWN     15

   RCPT_RATE_FROM_DOMAIN      200
   RCPT_RATE_FROM_LOCAL       300
   RCPT_RATE_FROM_FRIEND      100
   RCPT_RATE_FROM_UNKNOWN     25

   OPEN_CONN_FROM_DOMAIN           30
   OPEN_CONN_FROM_LOCAL            30
   OPEN_CONN_FROM_FRIEND           15
   OPEN_CONN_FROM_UNKNOWN          10


  This values can be configured at j-policy database as (e.g.) :

   ConnRate:LOCAL                300
   ConnRate:DOMAIN               200
   ConnRate:FRIEND               30
   ConnRate:DEFAULT              15
   #
   ConnRate:10.3                 400
   ConnRate:domain.com           2
   ConnRate:DEPMATH              400

   RcptRate:LOCAL                300
   RcptRate:DOMAIN               200
   RcptRate:FRIEND               100
   RcptRate:DEFAULT              25
   #
   RcptRate:10.3                 400
   RcptRate:domain.com           2
   RcptRate:DEPMATH              400

   ConnOpen:LOCAL                30
   ConnOpen:DOMAIN               30
   ConnOpen:FRIEND               15
   ConnOpen:DEFAULT              10
   #
   ConnOpen:10.3                 40
   ConnOpen:domain.com           2
   ConnOpen:DEPMATH              35

     .....
   The only options to be used at j-chkmail.cf file are the one used to
   enable these features and the default values. Default values option
   is being added to release 1.11

   CHECK_CONN_RATE            NO
   MAX_CONN_RATE              15 (New option)
   CHECK_RCPT_RATE            NO
   MAX_RCPT_RATE              25 (New option)
   CHECK_OPEN_CONNECTIONS     NO
   MAX_CONN_OPEN              15 (New option)

   If you want to remain, for some time, with the old behaviour, you
   shall configure j-chkmail to do that :

       ./configure CPPFLAGS="-DPRE_110_CONF=1"

5. j-local-users configuration file is considered deprecated (even if it's
   still there for some time - it will be removed in the future).
   You shall use j-rcpt database to define recipient access. Take a look at 
   the FAQ 5.24 to 5.27 paragraphs.

   http://j-chkmail.ensmp.fr/faq.dir/html/SECTION005.html#ITEM0024

6. As long as the code to check SMTP client address resolution was rewritten,
   the following options were removed.

	RESOLVE_FAIL
	RESOLVE_FORGED
	RESOLVE_ACCEPT_06H
	RESOLVE_ACCEPT_12H
	RESOLVE_ACCEPT_18H
	RESOLVE_ACCEPT_24H

   and replaced by :

	CHECK_RESOLVE_FAIL         YES
	CHECK_RESOLVE_FORGED       YES
	MAX_BAD_RESOLVE            10

   Instead of limiting the number of *connections* accepted on "ugly" 
   windows of size 6 hours, the limit now is set on the number of *messages* 
   accepted over the last 4 j-hours. (As you know, 1 j-minute equals 64 secs
   and 1 j-hour has 64 j-minutes or 4096 seconds).

7. Greylisting databases clean-up is now distributed over time instead of 
   completely doing it over time. GREY_CLEANUP_INTERVAL configuration option
   remain valid to define minimal interval between two clean-up operations. 
   Also, handling of data during cleaning up was optimized to reduce global
   handling time. You can eventually increase this value from previous 
   10 minutes default interval.

8. Using the file j-local-users is deprecated and code handling this was
   removed. Checking recipient access shall be done with j-rcpt database.

9. The list of macros imported from sendmail by j-chkmail grew. Please
   update your sendmail.cf file. Take a look at smconfig/* files.

10. Changes needed to make use of features provided by sendmail 8.14
    - Changing the enveloppe sender when sending notifications
    - Make use of RCPT results sent by sendmail to the filter

11. Four new configuration options were added, which are used when
    checking the content of messages.

    These options are used to define regular expressions to be matched
    against just inserted X-j-chkmail-Score header. If matched, some
    actions are triggered :

    # Insertion of a "X-j-chkmail-Status: HI" header
    XSTATUS_HEADER_HI_REGEX    (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
    # Insertion of a "X-j-chkmail-Status: LO" header
    XSTATUS_HEADER_LO_REGEX    (XXX)
    # Message is rejected
    XSTATUS_REJECT_REGEX       
    # Message is quarantined
    XSTATUS_QUARANTINE_REGEX   

12. j-chkmail now uses environnement and transactions to improve reliability
    of live databases and avoid database corruption in case of system 
    exceptions.

13. As a consequence of above, the layout of data files used by jchkmail was 
    changed to :

    /var/jchkmail
                 /files         -> all files before which where placed
                                   before inside /var/jchkmail, except
                                   databases

                 /cdb           -> constant databases :
                                   - j-policy.db
                                   - j-rcpt.db
                                   - j-urlbl.db
                                   - j-bayes.db

                 /wdb           -> live databases :
                                   - j-greypend.db
                                   - j-greyvalid.db
                                   - j-greywhitelist.db
                                   - j-greyblacklist.db
                                   - j-res-cache.db

                 /bayes-toolbox -> a directory where to create database
                                   of tokens needed by the bayesian 
                                   filter. This directory isn't (for the
                                   moment) created by install. To create 
                                   if, you shall launch :
                                          make install
                                          make install-learn
                                   This directory has a sample of spam
                                   and ham mailbox. To create the sample
                                   database of tokens, simply get into this
                                   directory and type
                                          cd /var/jchkmail/bayes-toolbox
                                          make

                 /jgreydb       -> live databases created by j-greyd

-------------------------------------------------------------------------------
