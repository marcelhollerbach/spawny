#include "main.h"
#include <string.h>

typedef struct List_Elem_ List_Elem;

struct List_Elem_ {
    List_Elem *next;
    void *data;
} ;

typedef struct {
    char *id;
    char *name;
    char *icon;
    Template_Fire_Up cb;
    void *data;
} Template;

static List_Elem *templates = NULL;
static int count = 0;

static Template*
find_temp(const char *id) {
    List_Elem *walker = templates;

    while(walker) {
        Template *temp = walker->data;

        if (!strcmp(id, temp->id)) {
            return temp;
        }
        walker = walker->next;
    }
    return NULL;
}

static List_Elem*
last_template(void) {
    List_Elem *walker = templates;

    if (!walker)
      return NULL;

    while(walker->next) {
        walker = walker->next;
    }

    return walker;
}

static List_Elem*
_insert_item(void) {
    List_Elem *elem;
    List_Elem *last;

    elem = calloc(1, sizeof(List_Elem));

    last = last_template();

    if (!last) {
        templates = elem;
    } else {
        last->next = elem;
    }
    return elem;
}

const char*
template_register(char *name, char *icon, Template_Fire_Up fire_up, void *data) {
    Template *temp;
    List_Elem *elem;
    char buf[PATH_MAX];

    if (!name) return NULL;
    if (!fire_up) return NULL;

    elem = _insert_item();

    snprintf(buf, sizeof(buf), "t%d", count);
    count ++;

    temp = calloc(1, sizeof(Template));

    temp->id = strdup(buf);
    temp->name = strdup(name);
    temp->icon = icon ? strdup(icon) : NULL;
    temp->cb = fire_up;
    temp->data = data;

    elem->data = temp;

    return temp->id;
}

int
template_unregister(const char *id) {
    Template *temp = find_temp(id);

    if (!temp) return 0;

    //clean out our stack
    {
        List_Elem *walker = templates;

        if (walker->data == temp) {
            free(walker);
            templates = NULL;
            walker = NULL;
        }

        while (walker) {
            List_Elem *next = walker->next;
            if (next && next->data == temp) {
                //we need to cut out next
                walker->next = next->next;
                free(next);
            }
            walker = next;
        }
    }

    //free template
    free(temp->id);
    free(temp->name);
    free(temp->icon);
    free(temp);

    return 1;
}

int
template_details_get(const char *id, const char **name, const char **icon) {
    Template *temp = find_temp(id);

    if (!temp) return 0;

    *name = temp->name;
    *icon = temp->icon;

    return 1;
}

int
template_run(const char *id) {
    Template *temp = find_temp(id);

    if (!temp) return 0;

    temp->cb(temp->data);

    return 1;
}

int
template_get(char ***template_ids, unsigned int *num) {
    List_Elem *walker = templates;
    char **temps = NULL;

    *num = 0;

    while(walker) {
        (*num) ++;

        temps = realloc(temps, (*num) * sizeof(char*));
        temps[*num - 1] = ((Template*)walker->data)->id;
        walker = walker->next;
    }
    *template_ids = temps;

    return 1;
}