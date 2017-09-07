dnl
dnl This line activates MILTER sendmail option
dnl
APPENDDEF(`conf_sendmail_ENVDEF',`-DMILTER')
dnl
APPENDDEF(`conf_libmilter_ENVDEF',`-D_FFR_WORKERS_POOL=1')
APPENDDEF(`conf_libmilter_ENVDEF',`-DSM_CONF_POLL=1')
dnl

