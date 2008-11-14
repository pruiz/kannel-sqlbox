#include "gwlib/gwlib.h"
#ifdef HAVE_SQLITE3
#include "gw/msg.h"
#include "sqlbox_sql.h"
void sql_save_msg(Msg *msg, Octstr *momt );
Msg *sqlite3_fetch_msg();
void sql_shutdown();
struct server_type *sql_init_sqlite3(Cfg *cfg);
#ifndef sqlbox_sqlite3_c
extern
#endif
Octstr *sqlbox_id;
#endif
