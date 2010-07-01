/* files used in db.c */
#ifndef _DB_H_
#define _DB_H_

extern bool fBootDb;
extern int newmobs;
extern int newobjs;
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern int top_mob_index;
extern int top_obj_index;
extern int top_affect;
extern int top_newaffect;
extern int top_ed;

#endif
