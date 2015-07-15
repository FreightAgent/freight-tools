/*********************************************************
 * *Copyright (C) 2015 Neil Horman
 * *This program is free software; you can redistribute it and\or modify
 * *it under the terms of the GNU General Public License as published 
 * *by the Free Software Foundation; either version 2 of the License,
 * *or  any later version.
 * *
 * *This program is distributed in the hope that it will be useful,
 * *but WITHOUT ANY WARRANTY; without even the implied warranty of
 * *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * *GNU General Public License for more details.
 * *
 * *File: freight-db.h
 * *
 * *Author:Neil Horman
 * *
 * *Date: April 16, 2015
 * *
 * *Description API for DB connections
 * *********************************************************/


#ifndef _FREIGHT_DB_H_
#define _FREIGHT_DB_H_
#include <stdio.h>
#include <errno.h>
#include <freight-config.h>


struct tbl {
	int rows;
	int cols;
	char *** value;
};

enum listen_channel {
	CHAN_CONTAINERS = 0,
	CHAN_NODES,
};

enum event_rc {
	EVENT_CONSUMED = 0,
	EVENT_NOCHAN,
};

struct db_api {

	/* setup and teardown functions */
	int (*init)(struct agent_config *acfg);
	void (*cleanup)(struct agent_config *acfg);
	int (*connect)(struct agent_config *acfg);
	int (*disconnect)(struct agent_config *acfg);

	/* operational methods */
	int (*send_raw_sql)(const char *values, const struct agent_config *acfg);

	struct tbl* (*get_table)(const char *tbl, const char *cols, const char *filter,
				 const struct agent_config *acfg);
	enum event_rc (*poll_notify)(const struct agent_config *acfg);
};

extern struct db_api postgres_db_api;
extern struct db_api nodb_api;

static inline struct db_api* get_db_api(struct agent_config *acfg)
{

	if (acfg->db.dbtype == DB_TYPE_NONE)
		return &nodb_api;

	switch (acfg->db.dbtype) {
	case DB_TYPE_POSTGRES:
		return &postgres_db_api;
	default:
		return NULL;
	}
}

static inline int db_init(struct db_api *api, struct agent_config *acfg)
{
	if (!api->init)
		return -EOPNOTSUPP;
	return api->init(acfg);	
}

static inline void db_cleanup(struct db_api *api, struct agent_config *acfg)
{
	if (!api->cleanup)
		return;
	return api->cleanup(acfg);	
}

static inline int db_connect(struct db_api *api, struct agent_config *acfg)
{
	if (!api->connect)
		return -EOPNOTSUPP;
	return api->connect(acfg);	
}

static inline int db_disconnect(struct db_api *api, struct agent_config *acfg)
{
	if (!api->disconnect)
		return -EOPNOTSUPP;
	return api->disconnect(acfg);	
}

static inline int wait_for_channel_notification(struct db_api *api, struct agent_config *acfg)
{
	if (!api->poll_notify)
		return -EOPNOTSUPP;
	return api->poll_notify(acfg);
}

extern int channel_subscribe(const struct db_api *api,
			     const struct agent_config *acfg,
			     const enum listen_channel chn,
			     enum event_rc (*hndl)(const enum listen_channel chnl, const char *data));

extern void channel_unsubscribe(const struct db_api *api,
				const struct agent_config *acfg,
				const enum listen_channel chn);

extern enum event_rc event_dispatch(const char *chn, const char *extra);

extern struct tbl *alloc_tbl(int rows, int cols);

extern void free_tbl(struct tbl *table);

extern int add_repo(const struct db_api *api,
		    const char *name,
		    const char *url,
		    const struct agent_config *acfg);

extern int del_repo(const struct db_api *api,
		    const char *name,
		    const struct agent_config *acfg);

extern int add_host(const struct db_api *api,
		    const char *hostname,
		    const struct agent_config *acfg);

extern int del_host(const struct db_api *api,
		    const char *hostname,
		    const struct agent_config *acfg);

extern int subscribe_host(const struct db_api *api,
			  const char *tennant,
			  const char *host,
			  const struct agent_config *acfg);

extern int unsubscribe_host(const struct db_api *api,
			  const char *tennant,
			  const char *host,
			  const struct agent_config *acfg);
extern int list_subscriptions(const struct db_api *api,
			     const char *tennant,
			     const struct agent_config *acfg);

extern struct tbl* get_tennants_for_host(const struct db_api *api,
				         const char *host,
				         const struct agent_config *acfg);

extern struct tbl* get_repos_for_tennant(const struct db_api *api,
					 const char *tennant,
					 const struct agent_config *acfg);

extern int request_create_container(const struct db_api *api,
				    const char *cname,
				    const char *iname,
				    const char *chost,
				    const struct agent_config *acfg);

#endif
