/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2008, Eduardo Silva P.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <err.h>

#include "config.h"
#include "plugin.h"

void *mk_plugin_load(char *path)
{
        void *handle;

        handle = dlopen(path, RTLD_LAZY);
        if(!handle){
                fprintf(stderr, "Error during dlopen(): %s\n", dlerror());
                exit(1);
        }
        return handle;
}

void *mk_plugin_load_symbol(void *handler, const char *symbol)
{
        char *err;
        void *s;

        dlerror();
        s = dlsym(handler, symbol);
        if((err = dlerror()) != NULL){
                return NULL;
        }

        return s;
}

void *mk_plugin_register(void *handler)
{
        struct plugin *p;

        p = mk_mem_malloc_z(sizeof(struct plugin));
        p->handler = handler;
        p->name = mk_plugin_load_symbol(handler, "_name");
        p->version = mk_plugin_load_symbol(handler, "_version");
        p->stages = (mk_plugin_stage_t *) mk_plugin_load_symbol(handler, "_stages");

        if(!p->name || !p->version || !p->stages){
                mk_mem_free(p);
                return NULL;
        }

        return p;
}

void mk_plugin_get_list()
{
        int len;
        char *path;
        char *key, *value, *last;
        char buffer[255];
        FILE *fconf;
        void *handle;
        
        path = mk_mem_malloc_z(1024);
        snprintf(path, 1024, "%s/%s", config->serverconf, MK_PLUGIN_LOAD);

	if((fconf=fopen(path, "r"))==NULL) {
		fprintf(stderr, "Error: I can't open %s file.\n", path);
		exit(1);
	}

        while(fgets(buffer, 255, fconf)) {
		len = strlen(buffer);
		if(buffer[len-1] == '\n') {
			buffer[--len] = 0;
			if(len && buffer[len-1] == '\r')
				buffer[--len] = 0;
		}
		
		if(!buffer[0] || buffer[0] == '#')
			continue;

		key = strtok_r(buffer, "\"\t ", &last);
		value = strtok_r(NULL, "\"\t ", &last);

		if (!key || !value) continue;

                if(strcasecmp(key, "LoadPlugin")==0){
                        handle = mk_plugin_load(value);
                        if(!mk_plugin_register(handle)){
                                fprintf(stderr, "Plugin error: %s", value);
                                dlclose(handle);
                        }
                }
        }

        fclose(fconf);
}

void mk_plugin_stage_run(mk_plugin_stage_t stage)
{

}
