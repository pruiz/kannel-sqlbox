/* ==================================================================== 
 * The Kannel Software License, Version 1.0 
 * 
 * Copyright (c) 2001-2004 Kannel Group  
 * Copyright (c) 1998-2001 WapIT Ltd.   
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in 
 *    the documentation and/or other materials provided with the 
 *    distribution. 
 * 
 * 3. The end-user documentation included with the redistribution, 
 *    if any, must include the following acknowledgment: 
 *       "This product includes software developed by the 
 *        Kannel Group (http://www.kannel.org/)." 
 *    Alternately, this acknowledgment may appear in the software itself, 
 *    if and wherever such third-party acknowledgments normally appear. 
 * 
 * 4. The names "Kannel" and "Kannel Group" must not be used to 
 *    endorse or promote products derived from this software without 
 *    prior written permission. For written permission, please  
 *    contact org@kannel.org. 
 * 
 * 5. Products derived from this software may not be called "Kannel", 
 *    nor may "Kannel" appear in their name, without prior written 
 *    permission of the Kannel Group. 
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED.  IN NO EVENT SHALL THE KANNEL GROUP OR ITS CONTRIBUTORS 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR  
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE  
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 * ==================================================================== 
 * 
 * This software consists of voluntary contributions made by many 
 * individuals on behalf of the Kannel Group.  For more information on  
 * the Kannel Group, please see <http://www.kannel.org/>. 
 * 
 * Portions of this software are based upon software originally written at  
 * WapIT Ltd., Helsinki, Finland for the Kannel project.  
 */ 

/*
 * dbpool_mssql.c - implement MSSQL operations for generic database connection pool
 *
 * Stipe Tolj <tolj@wapme-systems.de>
 *      2003 Initial version.
 * Alexander Malysh <a.malysh@centrium.de>
 *      2003 Made dbpool more generic.
 */

#ifdef HAVE_MSSQL
#include <sybfront.h>
#include <sybdb.h>

#define DATELEN     26
#define TYPELEN      2

static void* mssql_open_conn(const DBConf *db_conf)
{
	DBPROCESS *dbproc = NULL;
	MSSQLConf *conf = db_conf->mssql; /* make compiler happy */
	BOOL success;
	LOGINREC      *login;        /* Our login information. */
	

	/* sanity check */
	if (conf == NULL)
        	return NULL;

	/*
	** Get a LOGINREC structure and fill it with the necessary
	** login information.
	*/

	login = dblogin();
	DBSETLUSER(login, octstr_get_cstr(conf->username));
	DBSETLPWD(login, octstr_get_cstr(conf->password));
	DBSETLAPP(login, "kannel");

	/*
	** Get a DBPROCESS structure for communicating with SQL Server.
	** A NULL servername defaults to the server specified by DSQUERY.
	*/

	dbproc = dbopen(login, octstr_get_cstr(conf->database));
	success = (dbproc && (dbproc != FAIL));

	if (!success) {
		dbproc = NULL;
		return NULL;
	}

	return dbproc;
}

static void mssql_close_conn(void *conn)
{
    if (conn == NULL)
        return;

    dbclose((DBPROCESS *) conn);
}

static int mssql_check_conn(void *conn)
{
    if (conn == NULL)
        return -1;

    /* todo: implement alive check */

    return 0;
}

static void mssql_conf_destroy(DBConf *db_conf)
{
    MSSQLConf *conf = db_conf->mssql;

    octstr_destroy(conf->host);
    octstr_destroy(conf->username);
    octstr_destroy(conf->password);
    octstr_destroy(conf->database);

    gw_free(conf);
    gw_free(db_conf);
}

static struct db_ops mssql_ops = {
    .open = mssql_open_conn,
    .close = mssql_close_conn,
    .check = mssql_check_conn,
    .conf_destroy = mssql_conf_destroy
};

/* sybase related funcs */

// for sybase & freetds
#define ERR_CH stderr
// for sybase:
// #define BOOL int
// for freetds:
#define DBFAR
#define CS_PUBLIC

int CS_PUBLIC err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr, char *dberrstr, char *oserrstr)
{
	if ((dbproc == NULL) || (DBDEAD(dbproc)))
		return(INT_EXIT);
	else 
	{
		fprintf (ERR_CH, "DB-Library error:\n\t%s\n", dberrstr);

		if (oserr != DBNOERR)
			fprintf (ERR_CH, "Operating-system error:\n\t%s\n", oserrstr);

		return(INT_CANCEL);
	}
}

int CS_PUBLIC msg_handler(DBPROCESS dbproc, DBINT msgno, int msgstate, int severity, char *msgtext, 
                char *srvname, char *procname, int line)
{
	fprintf (ERR_CH, "Msg %d, Level %d, State %d\n", 
	        msgno, severity, msgstate);

	if (strlen(srvname) > 0)
		fprintf (ERR_CH, "Server '%s', ", srvname);
	if (strlen(procname) > 0)
		fprintf (ERR_CH, "Procedure '%s', ", procname);
	if (line > 0)
		fprintf (ERR_CH, "Line %d", line);

	fprintf (ERR_CH, "\n\t%s\n", msgtext);

	return(0);
}
#endif /* HAVE_MSSQL */
