#include "gwlib/gwlib.h"
#ifdef HAVE_PGSQL
#include "gwlib/dbpool.h"
#define sqlbox_pgsql_c
#include "sqlbox_pgsql.h"
#include <libpq-fe.h>

#define sql_update pgsql_update
#define sql_select pgsql_select
#define exit_nicely(conn) do { PQfinish(conn); } while(0)

static Octstr *sqlbox_logtable;
static Octstr *sqlbox_insert_table;

/*
 * Our connection pool to pgsql.
 */

static DBPool *pool = NULL;

/*
 *-------------------------------------------------
 * Postgres SQL thingies
 *-------------------------------------------------
*/

static void* pgsql_open_conn(const DBConf *db_conf)
{
    PGconn *conn = NULL;
    PgSQLConf *conf = db_conf->pgsql; /* make compiler happy */
    char tmp[1024];
    char cs[1024];

    cs[0]='\0';

    /* sanity check */
    if (conf == NULL)
        return NULL;
     
    sprintf(tmp, " host=%s", octstr_get_cstr(conf->host));    
    if (strlen(tmp)) strcat(cs, tmp);
    
    sprintf(tmp, " user=%s", octstr_get_cstr(conf->username));
    if (strlen(tmp)) strcat(cs, tmp);
    
    sprintf(tmp, " password=%s", octstr_get_cstr(conf->password));
    if (strlen(tmp)) strcat(cs, tmp);
    
    sprintf(tmp, " dbname=%s", octstr_get_cstr(conf->database));
    if (strlen(tmp)) strcat(cs, tmp);

  /*
    if (conf->pgport) octstr_append(connstr, octstr_format("port=%s ", conf->pgport));
    if (conf->pgoptions) octstr_append(connstr, octstr_format("options=%s ", conf->pgoptions));
    if (conf->pgtty) octstr_append(connstr, octstr_format("tty=%s ", conf->pgtty));
    */

    info(0, "PGSQL: Using connection string: %s.", cs);

    conn = PQconnectdb(cs);

    gw_assert (conn != NULL);

    if (PQstatus(conn) == CONNECTION_BAD) {
        error(0, "PGSQL: connection to database %s failed!", octstr_get_cstr(conf->database)); 
	error(0, "PGSQL: %s", PQerrorMessage(conn));
       goto failed;
    }
    

    info(0, "PGSQL: Connected to server at %s.", octstr_get_cstr(conf->host));

    return conn;

failed:
    exit_nicely(conn);
    if (conn != NULL) gw_free(conn);
    return NULL;
}

static void pgsql_update(const Octstr *sql)
{
    DBPoolConn *pc;
    PGresult *res;
    ExecStatusType status;

#if defined(DLR_TRACE)
     debug("dlr.pgsql", 0, "sql: %s", octstr_get_cstr(sql));
#endif

    pc = dbpool_conn_consume(pool);
    if (pc == NULL) {
        error(0, "PGSQL: Database pool got no connection! DB update failed!");
        return;
    }

    res = PQexec(pc->conn, octstr_get_cstr(sql));
    status = PQresultStatus(res);
    switch(status) {
	case PGRES_BAD_RESPONSE:
	case PGRES_NONFATAL_ERROR:
	case PGRES_FATAL_ERROR:
	    error (0, "PGSQL: %s", PQresultErrorMessage(res));
	    break;
    }
	    
    dbpool_conn_produce(pc);
}

static PGresult *pgsql_select(const Octstr *sql)
{
    int	state;
    PGresult *res = NULL;
    DBPoolConn *pc;

#if defined(DLR_TRACE)
    debug("dlr.pgsql", 0, "sql: %s", octstr_get_cstr(sql));
#endif

    pc = dbpool_conn_consume(pool);
    if (pc == NULL) {
        error(0, "PGSQL: Database pool got no connection! DB operation failed!");
        return NULL;
    }

    res = PQexec(pc->conn, octstr_get_cstr(sql));
    switch(PQresultStatus(res)) {
	    case PGRES_EMPTY_QUERY:
	    case PGRES_BAD_RESPONSE:
	    case PGRES_NONFATAL_ERROR:
	    case PGRES_FATAL_ERROR:
		    error(0, "PGSQL: %s", PQresultErrorMessage(res));
		    break;
    }
    dbpool_conn_produce(pc);
    return res;
}

void sqlbox_configure_pgsql(Cfg* cfg)
{
	CfgGroup *grp;
	Octstr *sql;

	if (!(grp = cfg_get_single_group(cfg, octstr_imm("sqlbox"))))
		panic(0, "SQLBOX: PGSQL: group 'sqlbox' is not specified!");

	sqlbox_logtable = cfg_get(grp, octstr_imm("sql-log-table"));
	if (sqlbox_logtable == NULL) {
		panic(0, "No 'sql-log-table' not configured.");
	}
	sqlbox_insert_table = cfg_get(grp, octstr_imm("sql-insert-table"));
	if (sqlbox_insert_table == NULL) {
		panic(0, "No 'sql-insert-table' not configured.");
	}

	/* create send_sms && sent_sms tables if they do not exist */
	sql = octstr_format("CREATE TABLE %S (sql_id SERIAL PRIMARY KEY, momt VARCHAR(3) CHECK(momt IN ('MO', 'MT', 'DLR', NULL)) DEFAULT NULL, sender VARCHAR(20) NULL, receiver VARCHAR(20) NULL, udhdata VARCHAR(255) NULL, msgdata VARCHAR(255) NULL, time BIGINT NULL, smsc_id VARCHAR(255) NULL, service VARCHAR(255) NULL, account VARCHAR(255) NULL, id BIGINT NULL, sms_type BIGINT NULL, mclass BIGINT NULL, mwi BIGINT NULL, coding BIGINT NULL, compress BIGINT NULL, validity BIGINT NULL, deferred BIGINT NULL, dlr_mask BIGINT NULL, dlr_url VARCHAR(255) NULL, pid BIGINT NULL, alt_dcs BIGINT NULL, rpi BIGINT NULL, charset VARCHAR(255) NULL, boxc_id VARCHAR(255) NULL, binfo VARCHAR(255) NULL)", sqlbox_logtable);
	sql_update(sql);
	octstr_destroy(sql);
	sql = octstr_format("CREATE TABLE %S (sql_id SERIAL PRIMARY KEY, momt VARCHAR(3) CHECK(momt IN ('MO', 'MT', NULL)) DEFAULT NULL, sender VARCHAR(20) NULL, receiver VARCHAR(20) NULL, udhdata VARCHAR(255) NULL, msgdata VARCHAR(255) NULL, time BIGINT NULL, smsc_id VARCHAR(255) NULL, service VARCHAR(255) NULL, account VARCHAR(255) NULL, id BIGINT NULL, sms_type BIGINT NULL, mclass BIGINT NULL, mwi BIGINT NULL, coding BIGINT NULL, compress BIGINT NULL, validity BIGINT NULL, deferred BIGINT NULL, dlr_mask BIGINT NULL, dlr_url VARCHAR(255) NULL, pid BIGINT NULL, alt_dcs BIGINT NULL, rpi BIGINT NULL, charset VARCHAR(255) NULL, boxc_id VARCHAR(255) NULL, binfo VARCHAR(255) NULL)", sqlbox_insert_table);
	sql_update(sql);
	octstr_destroy(sql);
	/* end table creation */
}

static Octstr *get_numeric_value_or_return_null(long int num)
{
	if (num == -1) {
		return octstr_create("NULL");
	}
	return octstr_format("%ld", num);
}

static Octstr *get_string_value_or_return_null(Octstr *str)
{
	if (str == NULL) {
		return octstr_create("NULL");
	}
	if (octstr_compare(str, octstr_imm("")) == 0) {
		return octstr_create("NULL");
	}
	octstr_replace(str, octstr_imm("\\"), octstr_imm("\\\\"));
	octstr_replace(str, octstr_imm("\'"), octstr_imm("\\\'"));
	return octstr_format("\'%S\'", str);
}

#define st_num(x) (stuffer[stuffcount++] = get_numeric_value_or_return_null(x))
#define st_str(x) (stuffer[stuffcount++] = get_string_value_or_return_null(x))

void pgsql_save_msg(Msg *msg, Octstr *momt /*, Octstr smsbox_id */)
{
	Octstr *sql, *values;
	Octstr *stuffer[30];
	int stuffcount = 0;

	values = octstr_format("%S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S, %S", st_str(momt), st_str(msg->sms.sender), st_str(msg->sms.receiver), st_str(msg->sms.udhdata), st_str(msg->sms.msgdata), st_num(msg->sms.time), st_str(msg->sms.smsc_id), st_str(msg->sms.service), st_str(msg->sms.account), st_num(msg->sms.sms_type), st_num(msg->sms.mclass), st_num(msg->sms.mwi), st_num(msg->sms.coding), st_num(msg->sms.compress), st_num(msg->sms.validity), st_num(msg->sms.deferred), st_num(msg->sms.dlr_mask), st_str(msg->sms.dlr_url), st_num(msg->sms.pid), st_num(msg->sms.alt_dcs), st_num(msg->sms.rpi), st_str(msg->sms.charset), st_str(msg->sms.boxc_id), st_str(msg->sms.binfo));
	sql = octstr_format("INSERT INTO %S (momt, sender, receiver, udhdata, msgdata, time, smsc_id, service, account, sms_type, mclass, mwi, coding, compress, validity, deferred, dlr_mask, dlr_url, pid, alt_dcs, rpi, charset, boxc_id, binfo) VALUES (%S)", sqlbox_logtable, values);
	sql_update(sql);
        //debug("sqlbox", 0, "sql_save_msg: %s", octstr_get_cstr(sql));
	while (stuffcount > 0) {
		octstr_destroy(stuffer[--stuffcount]);
	}
	octstr_destroy(values);
	octstr_destroy(sql);
}

void pgsql_leave()
{
	dbpool_destroy(pool);
}

#define octstr_null_create(x) ((PQgetvalue(res, 0, x) != NULL) ? octstr_create(PQgetvalue(res, 0, x)) : octstr_create(""))
#define atol_null(x) ((PQgetvalue(res, 0, x) != NULL) ? atol(PQgetvalue(res, 0, x)) : -1)
Msg *pgsql_fetch_msg()
{
	Msg *msg = NULL;
	Octstr *sql, *delet, *id;
	PGresult *res;

	sql = octstr_format("SELECT sql_id, momt, sender, receiver, udhdata, msgdata, time, smsc_id, service, account, id, sms_type, mclass, mwi, coding, compress, validity, deferred, dlr_mask, dlr_url, pid, alt_dcs, rpi, charset, boxc_id, binfo FROM %S LIMIT 1 OFFSET 0", sqlbox_insert_table);
	res = pgsql_select(sql);
	if (res == NULL) {
		debug("sqlbox", 0, "SQL statement failed: %s", octstr_get_cstr(sql));
	}
	else {
		if (PQntuples(res) >= 1) {
			id			= octstr_null_create(0);
			/* save fields in this row as msg struct */
			msg = msg_create(sms);
			msg->sms.sender		= octstr_null_create(2);
			msg->sms.receiver	= octstr_null_create(3);
			msg->sms.udhdata	= octstr_null_create(4);
			msg->sms.msgdata	= octstr_null_create(5);
			msg->sms.time		= atol_null(6);
			msg->sms.smsc_id	= octstr_null_create(7);
			msg->sms.service	= octstr_null_create(8);
			msg->sms.account	= octstr_null_create(9);
			/* msg->sms.id		= atol_null(row[10]); */
			msg->sms.sms_type	= atol_null(11);
			msg->sms.mclass		= atol_null(12);
			msg->sms.mwi		= atol_null(13);
			msg->sms.coding		= atol_null(14);
			msg->sms.compress	= atol_null(15);
			msg->sms.validity	= atol_null(16);
			msg->sms.deferred	= atol_null(17);
			msg->sms.dlr_mask	= atol_null(18);
			msg->sms.dlr_url	= octstr_null_create(19);
			msg->sms.pid		= atol_null(20);
			msg->sms.alt_dcs	= atol_null(21);
			msg->sms.rpi		= atol_null(22);
			msg->sms.charset	= octstr_null_create(23);
			msg->sms.binfo		= octstr_null_create(25);
			if ((PQgetvalue(res, 0, 24)) == NULL) {
				msg->sms.boxc_id= octstr_duplicate(sqlbox_id);
			}
			else {
				msg->sms.boxc_id= octstr_null_create(24);
			}
			/* delete current row */
			delet = octstr_format("DELETE FROM %S WHERE sql_id = %S", sqlbox_insert_table, id);
        		//debug("sqlbox", 0, "sql_fetch_msg: %s", octstr_get_cstr(delet));
			pgsql_update(delet);
			octstr_destroy(id);
			octstr_destroy(delet);
		}
		PQclear(res);
	}
	octstr_destroy(sql);
        //debug("sqlbox", 0, "sql_fetch_msg: %s", octstr_get_cstr(sql));
	return msg;
}

struct server_type *sqlbox_init_pgsql(Cfg* cfg)
{
    CfgGroup *grp;
    List *grplist;
    Octstr *pgsql_host, *pgsql_user, *pgsql_pass, *pgsql_db, *pgsql_id;
    Octstr *p = NULL;
    long pool_size, pgsql_port;
    int have_port;
    DBConf *db_conf = NULL;
    struct server_type *res = NULL;

    /*
     * check for all mandatory directives that specify the field names
     * of the used PGSQL table
     */
    if (!(grp = cfg_get_single_group(cfg, octstr_imm("sqlbox"))))
        panic(0, "SQLBOX: PGSQL: group 'sqlbox' is not specified!");

    if (!(pgsql_id = cfg_get(grp, octstr_imm("id"))))
   	    panic(0, "SQLBOX: PGSQL: directive 'id' is not specified!");

    /*
     * now grap the required information from the 'pgsql-connection' group
     * with the pgsql-id we just obtained
     *
     * we have to loop through all available PGSQL connection definitions
     * and search for the one we are looking for
     */

     grplist = cfg_get_multi_group(cfg, octstr_imm("pgsql-connection"));
     while (grplist && (grp = (CfgGroup *)gwlist_extract_first(grplist)) != NULL) {
        p = cfg_get(grp, octstr_imm("id"));
        if (p != NULL && octstr_compare(p, pgsql_id) == 0) {
            goto found;
        }
        if (p != NULL) octstr_destroy(p);
     }
     panic(0, "SQLBOX: PGSQL: connection settings for id '%s' are not specified!",
           octstr_get_cstr(pgsql_id));

found:
    octstr_destroy(p);
    gwlist_destroy(grplist, NULL);

    if (cfg_get_integer(&pool_size, grp, octstr_imm("max-connections")) == -1 || pool_size == 0)
        pool_size = 1;

    if (!(pgsql_host = cfg_get(grp, octstr_imm("host"))))
   	    panic(0, "SQLBOX: PGSQL: directive 'host' is not specified!");
    if (!(pgsql_user = cfg_get(grp, octstr_imm("username"))))
   	    panic(0, "SQLBOX: PGSQL: directive 'username' is not specified!");
    if (!(pgsql_pass = cfg_get(grp, octstr_imm("password"))))
   	    panic(0, "SQLBOX: PGSQL: directive 'password' is not specified!");
    if (!(pgsql_db = cfg_get(grp, octstr_imm("database"))))
   	    panic(0, "SQLBOX: PGSQL: directive 'database' is not specified!");
    have_port = (cfg_get_integer(&pgsql_port, grp, octstr_imm("port")) != -1);

    /*
     * ok, ready to connect to PGSQL
     */
    db_conf = gw_malloc(sizeof(DBConf));
    gw_assert(db_conf != NULL);

    db_conf->pgsql = gw_malloc(sizeof(PgSQLConf));
    gw_assert(db_conf->pgsql != NULL);

    db_conf->pgsql->host = pgsql_host;
    db_conf->pgsql->username = pgsql_user;
    db_conf->pgsql->password = pgsql_pass;
    db_conf->pgsql->database = pgsql_db;
    if (have_port) {
	db_conf->pgsql->port = pgsql_port;
    }

    pool = dbpool_create(DBPOOL_PGSQL, db_conf, pool_size);
    gw_assert(pool != NULL);

    /*
     * XXX should a failing connect throw panic?!
     */
    if (dbpool_conn_count(pool) == 0)
        panic(0,"SQLBOX: PGSQL: database pool has no connections!");

    octstr_destroy(pgsql_id);

    res = gw_malloc(sizeof(struct server_type));
    gw_assert(res != NULL);

    res->type = octstr_create("PGSQL");
    res->sql_enter = sqlbox_configure_pgsql;
    res->sql_leave = pgsql_leave;
    res->sql_fetch_msg = pgsql_fetch_msg;
    res->sql_save_msg = pgsql_save_msg;
    return res;
}

#endif
