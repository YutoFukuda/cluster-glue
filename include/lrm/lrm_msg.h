/*
 * Message Define For Local Resource Manager
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
 * By Huang Zhen <zhenh@cn.ibm.com> 2004/2/23
 *
 */
/*
 * Notice:
 *"status" indicates the exit status code of "status" operation
 * its value is defined in LSB
 *"state" indicates the state of resource, maybe LRM_RSC_BUSY, LRM_RSC_IDLE
 *"opstate" indicates how the op exit.like LRM_OP_DONE,LRM_OP_CANCELLED,
 * LRM_OP_TIMEOUT,LRM_OP_NOTSUPPORTED.
 */	
#ifndef __LRM_MSG_H
#define __LRM_MSG_H 1

#include <lrm/lrm_api.h>

#define LRM_CMDPATH 		"/tmp/.lrmdcmd"
#define LRM_CALLBACKPATH 	"/tmp/.lrmdcallback"

/*define the field type used by lrm*/
#define F_LRM_TYPE		"lrm_t"
#define F_LRM_APP		"lrm_app"
#define F_LRM_PID		"lrm_pid"
#define	F_LRM_UID		"lrm_uid"
#define F_LRM_GID		"lrm_gid"
#define F_LRM_RID		"lrm_rid"
#define F_LRM_RNAME		"lrm_rname"
#define F_LRM_RTYPE		"lrm_rtype"
#define F_LRM_PARAM		"lrm_param"
#define F_LRM_TIMEOUT		"lrm_timeout"
#define F_LRM_OP		"lrm_op"
#define F_LRM_OPCNT		"lrm_opcount"
#define F_LRM_OPSTATUS		"lrm_opstatus"
#define F_LRM_RC		"lrm_rc"
#define F_LRM_CALLID		"lrm_callid"		
#define F_LRM_RCOUNT		"lrm_rcount"
#define F_LRM_RIDS		"lrm_rids"
#define F_LRM_DATALEN		"lrm_datalen"
#define F_LRM_DATA		"lrm_data"
#define F_LRM_STATE		"lrm_state"
#define F_LRM_MONMODE		"lrm_monmode"
#define F_LRM_MONINTVL		"lrm_monintvl"
#define F_LRM_MONTGT		"lrm_montgt"
#define F_LRM_STATUS		"lrm_status"

#define	PRINT 	printf("file:%s,line:%d\n",__FILE__,__LINE__);


/*define the message typs between lrmd and client lib*/
#define REGISTER		"reg"
#define UNREGISTER		"unreg"
#define GETRATYPES		"types"
#define GETALLRCSES		"getall"
#define GETRSC			"getrsc"
#define GETRSCSTATE		"getstate"
#define	SETMONITOR		"setmon"
#define	GETMONITORS		"getmons"
#define FLUSHRSC		"flush"
#define ADDRSC			"addrsc"
#define DELRSC			"delrsc"
#define PERFORMOP		"op"
#define ISOPSUPPORT		"opspt"
#define OPDONE			"opdone"
#define MONITOR			"monitor"
#define RETURN			"return"
#define FLUSHOPS		"flushops"

#define UUID_SLEN 		37
#define MAX_INT_LEN 		10
#define MAX_NAME_LEN 		255
#define MAX_VALUE_LEN 		255
#define MAX_PARAM_LEN 		1024


/* temporarily add for build and test
 * will be replaced when ha_msg provide such funcs
 */
int ha_msg_add_int(struct ha_msg * msg, const char * name, int value);
int ha_msg_value_int(struct ha_msg * msg, const char * name, int* value);

int ha_msg_add_uuid(struct ha_msg * msg, const char * name, const uuid_t id);
int ha_msg_value_uuid(struct ha_msg * msg, const char * name, uuid_t id);

GList* ha_msg_value_list(struct ha_msg * msg, const char * name);
int ha_msg_add_list(struct ha_msg * msg, const char * name, GList* list);

GHashTable* ha_msg_value_hash_table(struct ha_msg * msg, const char * name);
int ha_msg_add_hash_table(struct ha_msg * msg, const char * name,
							GHashTable* hash_table);
							
GHashTable* msg_to_hash_table(struct ha_msg * msg);
struct ha_msg* hash_table_to_msg(GHashTable* hash_table);
							
char* hash_table_to_string(GHashTable* hash_table);
GHashTable* string_to_hash_table(const char* data);
void free_hash_table(GHashTable* hash_table);

 /*  
 * message for no parameter, like unreg,types,getall 
 * they do not include any paramters 
 */ 
struct ha_msg* create_lrm_msg(const char*  msg);

/*
 * message for only one parameter - resource id,
 * like getrsc,delrsc,flush,getstate,getmons
 */
struct ha_msg* create_lrm_rsc_msg(rsc_id_t rid, const char* msg);

/* register client message */ 
struct ha_msg* create_lrm_reg_msg(const char* app_name);

/* 	
 * add new resource
 * according to the opinion of Lars, it is awkward that we combine all
 * parameters in to one string. I think so too. So this call may changed soon
 */ 
struct ha_msg* create_lrm_addrsc_msg(rsc_id_t rid, const char* rname,
				const char* rtype, GHashTable* parameter);

/*  
 *
 *the return message from lrmd for reg,unreg,addrsc,delrsc,isopsupport. 
 *these return messages only include return code. 
 * 
 */ 
struct ha_msg* create_lrm_ret(int rc, int fields);
 

/*  
 * the return message for a status change monitoring. 
 */ 

struct ha_msg* create_rsc_perform_op_msg (rsc_id_t rid, lrm_op_t* op);

struct ha_msg* create_rsc_set_monitor_msg(rsc_id_t rid,	lrm_mon_t* monitor);
				
#endif /* __LRM_MSG_H */