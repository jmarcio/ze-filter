

j-chkmail 1.13.0
****************

Some main changes in this release are :


1. DNS RBWLs code was rewritten to provide better handling and 
   configuration or Black/White lists of both IP addresses and URL 
   blacklists, which are now configured at j-tables data file

2. Added support to POP before SMTP. Based on tests done by
   Steve Hsieh

3. Default configuration is no more a null filter but have some 
   features enabled.
	 SPAM_URLBL                         YES
	 SPAM_REGEX                         YES
         SPAM_ORACLE                        YES
         CHECK_CONN_RATE                    YES
         CHECK_OPEN_CONNECTIONS             YES
         CHECK_BADRCPTS                     YES  
         CHECK_RCPT_ACCESS                  YES
         CHECK_NB_RCPT                      YES
         CHECK_MSG_RATE                     YES
         CHECK_NB_MSGS                      YES
         CHECK_BAD_SENDER_MX                YES
         CHECK_DATE_IN_FUTURE               YES
         GREY_CHECK                         YES

4. j-chkmail -M option added allowing to create
	configuration files :
	j-chkmail -M null    : create null filter
	j-chkmail -M default : default configuration with some options 
	    enabled
	j-chkmail -M running : a clean configuration file with current 
	    options
	This option replaces old -m and -n options, which become deprecated

5. In default configuration, URLBL data comes from DNS, instead of BerkeleyDB
   databases. BerkDB databases are faster than DNS queries, but the latter
   is much easier to set up and don't require maintenance. Probably a good
   choice for small mail servers.

6. IPv6 support was added to j-chkmail.

7. To upgrade your installation, even if you're not using IPv6, you shall
   upgrade your live databases (inside /var/jchkmail/wdb). There is a 
   set of scripts to do that inside 
	contrib/migrate/migrate_to_1.13
   To accomodate IPv6 addresses, some separators inside records were changed.
   Take a look at the README file inside that directory. 

8. Basic message archiving capabilities were added to j-chkmail. A new 
   configuration option was added to enable it (ARCHIVE), and the
   corresponding policy database prefixes were added :
        ArchiveConnect, ArchiveFrom and ArchiveTo

9. Inside tools directory, there are some scripts useful to upgrade
   j-chkmail installations :
      * check-mta-conf - this script tells you if the MTA configuration
        file (sendmail.cf or main.cf) contains all needed macros, and
        can propose you the lines to add to those files
      * check-conf-diffs - this script, launched AFTER the new version
        was compiled and BEFORE being installed, will present the changes
        in configuratin options : new or removed options.
      * update-wdb-databases - this script is useful to upgrade bdb
        databases. It can be called during installation, but alone, 
        **BEFORE** installing a new j-chkmail version, but **AFTER**
        compilation of the new version, it can show you if the included
        Berkeley DB distribution was changed and upgrade databases.

10. A new Makefile target was defined : upgrade. Launching 
	make upgrade
    instead of 
	make install
    will export live database data before upgrading the filter programs
    and recreate databases after installation. This is useful for two
    reasons : changes in BerkeleyDB API or defragmentation of database.

