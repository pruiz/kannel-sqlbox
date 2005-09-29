#include "gwlib/gwlib.h"
#ifdef HAVE_MYSQL
#include "msg.h"
#include "sqlbox_sql.h"
void sql_save_msg(Msg *msg, Octstr *momt /*, Octstr smsbox_id */);
Msg *mysql_fetch_msg();
void sql_shutdown();
struct server_type *sql_init_mysql(Cfg *cfg);
#ifndef sqlbox_mysql_c
extern
#endif
Octstr *sqlbox_id;
#endif
