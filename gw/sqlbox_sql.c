#define sqlbox_sql_c
#include "sqlbox_sql.h"

struct server_type *sqlbox_init_sql(Cfg *cfg)
{
    CfgGroup *grp;
    Octstr *engine;
    struct server_type *res = NULL;

    grp = cfg_get_single_group(cfg, octstr_imm("sqlbox"));
    engine = cfg_get(grp, octstr_imm("sql-engine"));

    if (engine == NULL || octstr_compare(engine, octstr_imm("")) == 0)
        panic(0, "No sql engine defined.");

#ifdef HAVE_MSSQL
    if (octstr_compare(engine, octstr_imm("mssql")) == 0)
        res = (struct server_type *)sqlbox_init_mssql(cfg);

    if (res) {
        return res;
    }
#endif
#ifdef HAVE_MYSQL
    if (octstr_compare(engine, octstr_imm("mysql")) == 0)
         res = (struct server_type *)sqlbox_init_mysql(cfg);

    if (res) {
        return res;
    }
#endif
#ifdef HAVE_ORACLE
    if (octstr_compare(engine, octstr_imm("oracle")) == 0)
         res = (struct server_type *)sqlbox_init_oracle(cfg);

    if (res) {
        return res;
    }
#endif
#ifdef HAVE_PGSQL
    if (octstr_compare(engine, octstr_imm("pgsql")) == 0)
         res = (struct server_type *)sqlbox_init_pgsql(cfg);

    if (res) {
        return res;
    }
#endif
#ifdef HAVE_SDB
    if (octstr_compare(engine, octstr_imm("sdb")) == 0)
         res = (struct server_type *)sqlbox_init_sdb(cfg);

    if (res) {
        return res;
    }
#endif
#ifdef HAVE_SQLITE
    if (octstr_compare(engine, octstr_imm("sqlite")) == 0)
         res = (struct server_type *)sqlbox_init_sqlite(cfg);

    if (res) {
        return res;
    }
#endif
#ifdef HAVE_SQLITE3
    if (octstr_compare(engine, octstr_imm("sqlite")) == 0)
         res = (struct server_type *)sqlbox_init_sqlite3(cfg);

    if (res) {
        return res;
    }
#endif
    return res;
}
