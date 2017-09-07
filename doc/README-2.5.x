
j-chkmail 2.5.0
***************

* Bugs
******

- Corrected bug about increasing the maximum size of messages to be
  handled by the linear discriminator classifier

- Corrected a bug on j-easy-install, which removed some system files when
  setting the installation prefix to a value different from the default one.

* Changes
*********

* Ported to OpenIndiana

* Updated BerkeleyDB Library to 5.3.1

* Corrected a bug in the interface to BerkeleyDB preventing useless BDB
  log files to be autoremoved (bug appearedn when updating BDB library).

* Updated PCRE library to 8.31   

* added startup script "jchkmail.init.rhel". This script is supposed
  to work under RedHad Linux and compatibles (Fedora and CentOS).
  For the moment, it's always the original generic startup script
  which will be installed. In a future version, the best suited
  script to the operating system or distribution will be installed.
