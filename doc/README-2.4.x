
j-chkmail 2.4.0
***************

* Changes
*********

- Configuration file j-access definitely deprecated. 

- Linear discriminating filter added. For the while there isn't yet a
  distributable learning tool. Will be added soon. For the while, people
  can grab a learning data from us :
     cd /var/jchkmail/cdb
     rsync -av rsync://foss.jose-marcio.org:1873/j-toolbox/j-lr.txt .
     j-ndc reload lrdata
 

- Added the possibility to limit the message size, based on the triplet :

     SMTP client / enveloppe from / enveloppe to

  Limits are configured at j-policy database in the usual way. Something 
  like :

    MaxMsgSizeConnect:1.2.3.4       40000000
    MaxMsgSizeFrom:joe@domain.com      30000
    MaxMsgSizeTo:tim@domain.com        10000

  Unless defined, the default value is 0 (no limit). But no limit for 
  j-chkmail means the message size limite is that one imposed by the 
  MTA (sendmail or postfix).

- A new header (X-j-chkmail-Auth) is added when the SMTP client 
  authenticates. The header contents is the client login.
   
