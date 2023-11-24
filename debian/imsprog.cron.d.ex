#
# Regular cron jobs for the imsprog package
#
0 4	* * *	root	[ -x /usr/bin/imsprog_maintenance ] && /usr/bin/imsprog_maintenance
