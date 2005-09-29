#ifndef SQLBOX_SQL_H
#define SQLBOX_SQL_H
#include "gwlib/gwlib.h"
#include "msg.h"
#include "sqlbox_mysql.h"
#include "sqlbox_pgsql.h"
//#include "sqlbox_mssql.h"

struct server_type {
	Octstr *type;
	void (*sql_enter) (Cfg *cfg);
	void (*sql_leave) ();
	Msg *(*sql_fetch_msg) ();
	void (*sql_save_msg) (Msg *, Octstr *);
};

struct server_type *sqlbox_init_sql(Cfg *cfg);

#ifndef sqlbox_sql_c
extern
#endif
struct server_type *sql_type;

#define gw_sql_fetch_msg sql_type->sql_fetch_msg
#define gw_sql_save_msg sql_type->sql_save_msg
#define gw_sql_enter sql_type->sql_enter
#define gw_sql_leave sql_type->sql_leave

#endif
