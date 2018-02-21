/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Sun Jun  1 19:42:35 CEST 2008
 *
 * This program is free software, but with restricted license :
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#ifndef __ZE_DOXYGEN_H

/**
 * @mainpage ze-filter : source code documentation
 *
 * This documentation is just a help for the filter programmer. 
 * Maybe I should say : more than just a help...
 *
 * A little bit of history : this filter beginning was around at the end of 
 * year 2001. Most of its development happened during years 2001-2010. So,
 * most of its code were built upon its base, trying to remain coherent.
 *
 * A little bit of history : the filter beginning was around the end of 
 * year 2001. Most of its development happened during years 2001-2010.
 *
 * The first feature of the filter was filtering based on the file extension 
 * of attached files (.exe, .com, ...). This was immediately followed by 
 * evaluating connection rate of SMTP clients, after a DoS attack against 
 * our mail servers. Other filtering ideas were tested and implemented during 
 * these 10 years.
 *
 * So, I can't talk "bottom-up" programming but "incremental" programming. 
 * This isn't always a good programming practice, but it's what should be done 
 * because, at the beginning, there were no system specifications and I didn't 
 * knew what this filter could be.
 *
 * At the same time, mail filtering is a moving domain, new spam and malware 
 * tricks appears all the time.
 *
 * Within this context, it's not unusual that some incoherence appear in the 
 * source code : variable names, code redundancy, ...
 *
 * Other than the usual goal of documenting this code, this tool is intended 
 * to help a task of code revision. And Doxygen is a great tool to do this.
 *
 */

/** 
 * @defgroup RateLimit    Filter : SMTP Rate Limit
 * @defgroup Behaviour    Filter : Behaviour checking
 * @defgroup Greylisting  Filter : Greylisting
 * @defgroup XFiles       Filter : XFiles 
 * @defgroup Bayes        Filter : Bayesian filter
 * @defgroup Logreg       Filter : Neural Linear Discriminator
 * @defgroup Regex        Filter : Regular expressions
 * @defgroup AntiVirus    Filter : Antivirus
 * @defgroup RBL          Filter : DNS - (IP / URL) Blacklists
 * @defgroup Heuristics   Filter : Oracle and other heuristics
 * @defgroup Actions      Filter : Actions after filtering
 * @defgroup Systools     Tools : System related tools
 * @defgroup Network      Tools : Network functions
 * @defgroup MsgTools     Tools : Message Tools
 * @defgroup Strings      Tools : Strings and Text
 * @defgroup Logging      Tools : ze-filter logging
 * @defgroup APIs         Base : Interface with to other APIs (PCRE, BerkeleyDB)
 * @defgroup DataStruct   Base : Basic Data Structures Handling
 * @defgroup DBConf       Configuration : Static Databases Handling
 * @defgroup TxtConf      Configuration : Text files
 */

/** @} */

# define __ZE_DOXYGEN_H    1
#endif /* __ZE_DOXYGEN_H */

