#include "gwlib/gwlib.h"
#ifdef HAVE_ORACLE
#include "gw/msg.h"
#include "sqlbox_sql.h"
void sql_save_msg(Msg *msg, Octstr *momt /*, Octstr smsbox_id */);
Msg *oracle_fetch_msg();
void sql_shutdown();
struct server_type *sql_init_oracle(Cfg *cfg);
#ifndef sqlbox_oracle_c
extern
#endif
Octstr *sqlbox_id;
#endif
