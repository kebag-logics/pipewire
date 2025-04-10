#ifndef __AVB_AECP_AEM_LOCK_H__
#define __AVB_AECP_AEM_LOCK_H__

#define AECP_AEM_LOCK_ENTITY_EXPIRE_TIMEOUT (60UL)
#define AECP_AEM_LOCK_ENTITY_FLAG_LOCK		(1)

#include "../aecp-aem.h"

/**
 * @brief Command handling will generate the response for the lock command
 */
int handle_cmd_lock_entity(struct aecp *aecp, int64_t now, const void *m, int len);

/**
 * @brief this is the unsolicted notificiation
 **/
int handle_unsol_lock_entity(struct aecp *aecp, int64_t now);

#endif //__AVB_AECP_AEM_LOCK_H__