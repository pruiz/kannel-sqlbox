#include "gwlib/gwlib.h"
#ifdef HAVE_PGSQL
#include "msg.h"
#include "sqlbox_sql.h"
#define sql_fetch_msg pgsql_fetch_msg
#define sql_save_msg pgsql_save_msg
#define sql_leave pgsql_leave
void sql_save_msg(Msg *msg, Octstr *momt /*, Octstr smsbox_id */);
Msg *pgsql_fetch_msg();
void sql_shutdown();
struct server_type *sql_init_pgsql(Cfg *cfg);
void sqlbox_configure_pgsql(Cfg* cfg);
#ifndef sqlbox_pgsql_c
extern
#endif
Octstr *sqlbox_id;
#endif
