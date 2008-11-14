#include "gwlib/gwlib.h"
#ifdef HAVE_SQLITE
#include "gw/msg.h"
#include "sqlbox_sql.h"
void sql_save_msg(Msg *msg, Octstr *momt );
Msg *sqlite_fetch_msg();
void sql_shutdown();
struct server_type *sql_init_sqlite(Cfg *cfg);
#ifndef sqlbox_sqlite_c
extern
#endif
Octstr *sqlbox_id;
#endif
