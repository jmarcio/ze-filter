

j-chkmail 1.12.0
****************

Some main changes in this release are :

1. j-greyd was more extensively validated
   * j-greyd shares its main configuration file with j-chkmail
   * j-greyd shares policy database with j-chkmail

2. Modules - EXPERIMENTAL
   * An interface was added to j-chkmail to be able to handle some task
     to external modules.
   * This is experimental and disabled by default. To enable it, you shall
     define "-D_FFR_MODULES=1" at CPPFLAGS
   * In this version, modules shall use only system libraries. This restriction
     will be removed in the future
   * New configuration options :
     MODULES_CF (/etc/mail/jchkmail/j-modules) - modules configuration file
     MODDIR (/usr/lib/j-chkmail) - directory where modules are installed

3. Global message content evaluation.
   * Global score evaluation function was modified. New function is explained
     at the documentation
   * The following configuration options were removed or replaced
      SPAM_REGEX_SCORE
      LO_SCORE_ACTION
      HI_SCORE_ACTION
      SCORE_ON_SUBJECT_THRESHOLD
      XSTATUS_HEADER_HI_REGEX
      XSTATUS_HEADER_LO_REGEX
      XSTATUS_REJECT_REGEX
      XSTATUS_QUARANTINE_REGEX
   * New configuration options
      REGEX_MAX_SCORE
      XSTATUS_HEADER
      XSTATUS_HEADER_HI_CONDITION
      XSTATUS_HEADER_LO_CONDITION
      XSTATUS_HEADER_UNSURE_CONDITION
      XSTATUS_HEADER_HAM_CONDITION
      XSTATUS_REJECT_CONDITION
      XSTATUS_QUARANTINE_CONDITION
          score
          bayes
          urlbl
          regex
          urlbl+regex
          regex+urlbl
          oracle

4. Bayes filter can now read/write hash coded (md5 or sha1) tokens. This
   is useful to be able to distribute bayes databases without disclosing
   some private information.

5. The way individual filters are combined to create the global score was
   modified to improve filter accuracy

6. Heuristic filter now uses odds and logits to evaluate scores instead
   of additive scores.

7. j-chkmail was ported and test with postfix release 2.4 and newers.

8. Removed configuration options
   * These configuration options are done now at j-policy database
	NETS_FILE
	CONN_RATE_FROM_DOMAIN
	CONN_RATE_FROM_LOCAL
	CONN_RATE_FROM_FRIEND
	CONN_RATE_FROM_UNKNOWN
	RCPT_RATE_FROM_DOMAIN
	RCPT_RATE_FROM_LOCAL
	RCPT_RATE_FROM_FRIEND
	RCPT_RATE_FROM_UNKNOWN
	OPEN_CONN_FROM_DOMAIN
	OPEN_CONN_FROM_LOCAL
	OPEN_CONN_FROM_FRIEND
	OPEN_CONN_FROM_UNKNOWN
    * All these are now included in SPAM_REGEX
	CHECK_HEADERS_CONTENT
	CHECK_HELO_CONTENT
	CHECK_ENVFROM_CONTENT


