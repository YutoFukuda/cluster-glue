/*
 * Message  Functions  For Local Resource Manager
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * By Huang Zhen <zhenh@cn.ibm.com> 2004/2/13
 *
 */
#include <portability.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <clplumbing/cl_log.h>
#include <ha_msg.h>
#include <heartbeat.h>
#include <lrm/lrm_api.h>
#include <lrm/lrm_msg.h>

static void pair_to_string(gpointer key, gpointer value, gpointer user_data);
static gboolean free_pair(gpointer key, gpointer value, gpointer user_data);
static void pair_to_msg(gpointer key, gpointer value, gpointer user_data);

int
ha_msg_add_int(struct ha_msg * msg, const char * name, int value)
{
	char buf[MAX_INT_LEN];
	snprintf(buf, MAX_INT_LEN, "%d", value);
	return (ha_msg_nadd(msg, name, strlen(name), buf, strlen(buf)));
}

int
ha_msg_add_uuid(struct ha_msg * msg, const char * name, const uuid_t id)
{
	char buf[UUID_SLEN];
	uuid_unparse(id, buf);
	return (ha_msg_nadd(msg, name, strlen(name), buf, strlen(buf)));
}

int
ha_msg_value_uuid(struct ha_msg * msg, const char * name, uuid_t id)
{
	const char* value = ha_msg_value(msg, name);
	char buf[UUID_SLEN];

	if (NULL == value) {
		return HA_FAIL;
	}
	strncpy(buf,value,UUID_SLEN);
	if( 0 != uuid_parse(value, id)) {
		return HA_FAIL;
	}

	return HA_OK;
}

int
ha_msg_value_int(struct ha_msg * msg, const char * name, int* value)
{
	const char* svalue = ha_msg_value(msg, name);
	if(NULL == svalue) {
		return HA_FAIL;
	}
	*value = atoi(svalue);

	return HA_OK;
}

GList* 
ha_msg_value_list(struct ha_msg * msg, const char * name)
{
	
	int i = 1;
	char paramname[MAX_NAME_LEN+MAX_INT_LEN];
	const char* value;
	char* element;
	GList* list = NULL;
	
	if (NULL == msg) {
		return NULL;
	}
	if (NULL == name) {
		return NULL;
	}
	if (strlen(name) > MAX_NAME_LEN) {
		return NULL;
	}
	while (TRUE) {
		
		snprintf(paramname, MAX_NAME_LEN+MAX_INT_LEN, "%s%d", name, i);
		value = ha_msg_value(msg,paramname);
		if (NULL == value) {
			break;
		}
		element = g_strdup(value);
		list = g_list_append(list, element);
		i++;
	}
	return list;
}

int
ha_msg_add_list(struct ha_msg * msg, const char * name, GList* list)
{
	int i = 1;
	char paramname[MAX_NAME_LEN+MAX_INT_LEN];
	if (NULL == msg) {
		return HA_FAIL;
	}
	if (NULL == name) {
		return HA_FAIL;
	}
	if (strlen(name) > MAX_NAME_LEN) {
		return HA_FAIL;
	}
	
	if (NULL != list) {
		GList* element = g_list_first(list);
		while (NULL != element) {
			char* value = (char*)element->data;
			snprintf(paramname, MAX_NAME_LEN+MAX_INT_LEN, "%s%d", name, i);
			ha_msg_add(msg,paramname, value);
			element = g_list_next(element);
			i++;
		}
	}
	return HA_OK;
}
void
pair_to_msg(gpointer key, gpointer value, gpointer user_data)
{
	struct ha_msg* msg = g_new(struct ha_msg, 1);
	ha_msg_add(msg, key, value);
}

GHashTable*
ha_msg_value_hash_table(struct ha_msg * msg, const char * name)
{
	struct ha_msg* hash_msg = cl_get_struct(msg, name);
	if (NULL == hash_msg) {
		return NULL;
	}
	GHashTable* hash_table = msg_to_hash_table(hash_msg);
	return hash_table;
}

int
ha_msg_add_hash_table(struct ha_msg * msg, const char * name,
							GHashTable* hash_table)
{
	if (NULL == msg || NULL == name || NULL == hash_table) {
		return HA_FAIL;
	}
	struct ha_msg* hash_msg = hash_table_to_msg(hash_table);
	ha_msg_addstruct(msg, name, hash_msg);
	return HA_OK;
}
GHashTable*
msg_to_hash_table(struct ha_msg * msg)
{
	int i;
	GHashTable* hash_table = g_hash_table_new(g_str_hash, g_str_equal);
	for (i = 0; i < msg->nfields; i++) {
		g_hash_table_insert(hash_table, msg->names[i],msg->values[i]);
	}
	return hash_table;
}
struct ha_msg*
hash_table_to_msg(GHashTable* hash_table)
{
	struct ha_msg* hash_msg = g_new(struct ha_msg, 1);
	g_hash_table_foreach(hash_table, pair_to_msg, hash_msg);
	return hash_msg;
}

void
pair_to_string(gpointer key, gpointer value, gpointer user_data)
{
	char* param = (char*)user_data;
	strncat(param,(char*)key,MAX_PARAM_LEN);
	strncat(param,"=",MAX_PARAM_LEN);
	strncat(param,(char*)value,MAX_PARAM_LEN);
	strncat(param,":",MAX_PARAM_LEN);
}

char*
hash_table_to_string(GHashTable* hash_table)
{
	char* param = g_new(char, MAX_PARAM_LEN);
	param[0]=EOS;
	g_hash_table_foreach(hash_table, pair_to_string, param);
	char* ret = g_strdup(param);
	g_free(param);
	return ret;
}

GHashTable*
string_to_hash_table(const char* data)
{
	int name_start=0;
	int name_end=0;
	int value_start=0;
	int value_end=0;
	char* temp;
	char* key;
	char* value;
	GHashTable* hash_table = g_hash_table_new(g_str_hash, g_str_equal);
	while(name_start<strlen(data)) {
		temp = strchr(data+name_start,'=');
		name_end = temp - data ;
		value_start = name_end+1;
		temp = strchr(data+value_start,':');
		value_end = temp - data;
		key = strndup(data+name_start,name_end-name_start);
		value =	strndup(data+value_start,value_end-value_start);
		g_hash_table_insert(hash_table,key, value);

		name_start = value_end+1;
	}
	return hash_table;
}

gboolean
free_pair(gpointer key, gpointer value, gpointer user_data)
{
	g_free(key);
	g_free(value);
	return TRUE;
}

void
free_hash_table(GHashTable* hash_table)
{
	g_hash_table_foreach_remove(hash_table, free_pair, NULL);
	g_hash_table_destroy(hash_table);
}	

struct ha_msg*
create_lrm_msg (const char* msg)
{
	if ((NULL == msg) || (0 == strlen(msg))) {
		return NULL;
	}

	struct ha_msg* ret = ha_msg_new(1);
	if (HA_FAIL == ha_msg_add(ret, F_LRM_TYPE, msg)) {
		return NULL;
	}

	return ret;
}

struct ha_msg*
create_lrm_reg_msg(const char* app_name)
{
	if ((NULL == app_name) || (0 == strlen(app_name))) {
		return NULL;
	}

	struct ha_msg* ret = ha_msg_new(5);

	if (HA_FAIL == ha_msg_add(ret, F_LRM_TYPE, REGISTER)) {
		return NULL;
	}
	
	if (HA_FAIL == ha_msg_add(ret, F_LRM_APP, app_name)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(ret, F_LRM_PID, getpid())) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(ret, F_LRM_GID, getgid())) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(ret, F_LRM_UID, getuid())) {
		return NULL;
	}
	
	return ret;
}

struct ha_msg*
create_lrm_addrsc_msg(rsc_id_t rid, const char* rname,
					  const char* rtype, GHashTable* params)
{
	struct ha_msg* msg = ha_msg_new(5);
	if (HA_FAIL == ha_msg_add(msg, F_LRM_TYPE, ADDRSC)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_uuid(msg, F_LRM_RID, rid)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add(msg, F_LRM_RNAME, rname)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add(msg, F_LRM_RTYPE, rtype))	{
		return NULL;
	}
	if (NULL != params) {
		char* param_str = hash_table_to_string(params);
		if (HA_FAIL == ha_msg_add(msg, F_LRM_PARAM, param_str))	{
			return NULL;
		}
		g_free(param_str);
/*		ha_msg_add_hash_table(msg,F_LRM_PARAM,params);*/
	}
	
	return msg;
}

struct ha_msg*
create_lrm_rsc_msg(rsc_id_t rid, const char* msg)
{
	if ((NULL == msg) || (0 == strlen(msg))) {
		return NULL;
	}

	struct ha_msg* ret = ha_msg_new(1);
	if (HA_FAIL == ha_msg_add(ret, F_LRM_TYPE, msg)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_uuid(ret, F_LRM_RID, rid)) {
		return NULL;
	}
	return ret;
}



struct ha_msg*
create_lrm_ret(int rc, int fields)
{
	struct ha_msg* ret = ha_msg_new(fields);
	if (HA_FAIL == ha_msg_add(ret, F_LRM_TYPE, RETURN)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(ret, F_LRM_RC, rc)) {
		return NULL;
	}
	return ret;
}

struct ha_msg*
create_rsc_perform_op_msg (rsc_id_t rid, lrm_op_t* op)
{
	struct ha_msg* msg = ha_msg_new(5);
	if (HA_FAIL == ha_msg_add(msg, F_LRM_TYPE, PERFORMOP)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_uuid(msg, F_LRM_RID, rid)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add(msg, F_LRM_OP, op->op_type)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(msg, F_LRM_TIMEOUT, op->timeout))	{
		return NULL;
	}
	if (NULL != op->params) {
		char* param_str = hash_table_to_string(op->params);
		if (HA_FAIL == ha_msg_add(msg, F_LRM_PARAM, param_str))	{
			return NULL;
		}
	}
	return msg;
}

struct ha_msg*
create_rsc_set_monitor_msg (rsc_id_t rid, lrm_mon_t* monitor)
{
	struct ha_msg* msg = ha_msg_new(5);
	if (HA_FAIL == ha_msg_add(msg, F_LRM_TYPE, SETMONITOR)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(msg, F_LRM_MONMODE, monitor->mode)) {
		return NULL;
	}
	if (HA_FAIL == ha_msg_add_int(msg, F_LRM_MONINTVL, monitor->interval)) {
		return NULL;
	}
	if (HA_FAIL == ha_msg_add_int(msg, F_LRM_MONTGT, monitor->target)) {
		return NULL;
	}
	if (HA_FAIL == ha_msg_add_uuid(msg, F_LRM_RID, rid)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add(msg, F_LRM_OP, monitor->op_type)) {
		return NULL;
	}

	if (HA_FAIL == ha_msg_add_int(msg, F_LRM_TIMEOUT, monitor->timeout))	{
		return NULL;
	}
	if (NULL != monitor->params) {
		char* param_str = hash_table_to_string(monitor->params);
		if (HA_FAIL == ha_msg_add(msg, F_LRM_PARAM, param_str))	{
			return NULL;
		}
	}
	return msg;
}