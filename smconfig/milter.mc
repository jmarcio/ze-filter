dnl MILTER
dnl
dnl
dnl
dnl INPUT_MAIL_FILTER(`ze-filter',`S=inet:2000@localhost, T=C:2m;S:20s;R:40s;E:5m')
dnl
INPUT_MAIL_FILTER(`ze-filter',`S=local:/var/run/ze-filter/ze-filter.sock, T=C:2m;S:20s;R:40s;E:5m')
dnl
define(`confINPUT_MAIL_FILTERS',`ze-filter')
dnl
define(`confMILTER_MACROS_CONNECT',`_, j, v, {client_addr}, {client_name}, {client_ptr}, {client_resolve}, {daemon_addr}, {daemon_name}, {daemon_port}, {if_addr}, {if_name}')
define(`confMILTER_MACROS_DATA',`')
define(`confMILTER_MACROS_ENVFROM',`i, {auth_authen}, {auth_author}, {auth_ssf}, {auth_type}, {mail_addr}, {mail_host}, {mail_mailer}')
define(`confMILTER_MACROS_ENVRCPT',`{nrcpts}, {rcpt_addr}, {rcpt_host}, {rcpt_mailer}')
define(`confMILTER_MACROS_EOH',`')
define(`confMILTER_MACROS_EOM',`{msg_id}')
define(`confMILTER_MACROS_HELO',`{cert_issuer}, {cert_subject}, {cipher_bits}, {cipher}, {tls_version}')
define(`confMILTER_MACROS_UNKNOWN',`')
dnl
