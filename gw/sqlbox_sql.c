#define sqlbox_sql_c
#include "sqlbox_sql.h"

struct server_type *sqlbox_init_sql(Cfg *cfg)
{
	struct server_type *res = NULL;

#ifdef HAVE_MYSQL
	res = (struct server_type *)sqlbox_init_mysql(cfg);
	if (res) {
		return res;
	}
#endif
#ifdef HAVE_PGSQL
	res = (struct server_type *)sqlbox_init_pgsql(cfg);
	if (res) {
		return res;
	}
#endif
	return res;
}
