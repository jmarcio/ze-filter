
j-chkmail 2.3.0
***************

* Changes
*********
  
Some main changes in this release are :

1. Included third part libraries upgraded
   PCRE       library updated to 8.12
   BerkeleyDB library updated to 5.1.25

2. Corrected bug on checking EHLO command against localhost name.

3. Log level of EHLO command checks was tied to the level of 
   log of the oracle checks. It's independent now.

4. Improved error checking on the dialog between j-chkmail and
   j-greyd server

5. Default data update :
   j-policy.z-iana.txt as all IPv4 prefix are now allocated by
   IANA


j-chkmail 2.3.1
***************

* Changes
*********

1. Some compile time errors/warnings (11) pointed out by Thomas Spahni


j-chkmail 2.3.2
***************

