
j-chkmail 2.6.0
***************

* FEATURE - New prefix added to j-policy database :

     NetClassEquiv:oneclass    anotherclass

  This allows for two things :
  a. If j-chkmail looks for a feature (limit, access, ...) for class
     "oneclass", and don't find it, then it looks if there is something
     defined for "anotherclass"
  b. If "anotherclass" is one of LOCAL, DOMAIN, FRIEND, OTHER, AUTH, then
     it becomes a "known" class. And, remember, some spam filtering rules
     are lighter when applied to known classes. E.g. no greylisting, ...

* FEATURE - rate limits based on the Enveloppe From email address. This is 
  intended to limit on submission servers. Probably less useful on MTAs.

     MsgRateFrom:*                        0
     RcptRateFrom:*                       0 
     MaxRcptFrom:*                        0
     MsgRateFrom:user@domain-one.com      1
     RcptRateFrom:user@domain-one.com     2 
     MaxRcptFrom:user@domain-one.com      3
     MsgRateFrom:*@domain-two.com        10
     RcptRateFrom:*@domain-two.com       20 
     MaxRcptFrom:*@domain-two.com        30

* FEATURE - Changing in the way email address are looked up in policy databases.
  In previous versions, there were some confusion, on the sequence of queries
  and how to define not fully qualified email on policy database.

  From now, if one want to check the policy to an email, say :
  
    user@dept.example.com

  with some prefix, say "Prf", the policy database will be sequentially queried
  with the keys :

    Prf:user@dept.domain.com                 
    Prf:*@dept.domain.com                    
    Prf:dept.domain.com                      
    Prf:user@*.domain.com                    
    Prf:*@*.domain.com                       
    Prf:*.domain.com                         
    Prf:user@*.com                           
    Prf:*@*.com                              
    Prf:*.com                                
    Prf:user@*                               
    Prf:*@*                                  
    Prf:*                                                                     
    Prf:default  
 
  Note : any of "*@*", "*" and "default" can be used to define default values.

  WARNING : this is "work in progress". So, in release 2.6.0, it's active only
    for the new features introduced in this release. For the other features,
    this will be done in the next official release, after a global review of
    the code.
 
* BEHAVIOR CHANGE - the content of messages coming from 127.0.0.1 wasn't, 
  before this version, checked. From this version and if you want to preserve
  previous behaviour, you shall add a line of this kind at j-policy database:
      ContentCheckConnect:127.0.0.1    NO-QUICK 


j-chkmail 2.6.2
***************

* BUG - get-bayes doesn't get the good j-lr.txt file from j-chkmail source server
  Problem pointed out by Dudi Goldenberg

* BUG - j-chkmail 2.6.1 says the version is 2.6.0. This is a problem when upgrading
  j-chkmail with j-easy-install

