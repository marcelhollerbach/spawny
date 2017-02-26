#include "main.h"
#include <string.h>

typedef struct {
    char *id;
    char *name;
    char *icon;
    Template_Fire_Up cb;
    void *data;
} Template;

ARRAY_API(Template)

static Array *array = NULL;
static unsigned int counter = 0;

static void
template_init(void)
{
    if (!array)
      array = array_Template_new();
}


static Template*
find_temp(const char *id, unsigned int *array_id) {

    for (int i = 0; i < array_len_get(array); ++i)
    {
        Template *t;

        t = array_Template_get(array, i);

        if (!strcmp(id, t->id))
          {
             if (array_id) *array_id = i;
             return t;
          }
    }
    if (array_id) *array_id = 0;
    return NULL;
}

const char*
template_register(char *name, char *icon, Template_Fire_Up fire_up, void *data) {
    Template *temp;
    char buf[PATH_MAX];

    if (!name) return NULL;
    if (!fire_up) return NULL;

    snprintf(buf, sizeof(buf), "t%d", counter);
    counter ++;

    template_init();

    temp = array_Template_add(array);
    temp->id = strdup(buf);
    temp->name = strdup(name);
    temp->icon = icon ? strdup(icon) : NULL;
    temp->cb = fire_up;
    temp->data = data;

    return temp->id;
}

int
template_unregister(const char *id) {
    unsigned int i;
    Template *temp = find_temp(id, &i);

    if (!temp) return 0;

    //free template
    free(temp->id);
    free(temp->name);
    free(temp->icon);

    array_Template_del(array, i);

    return 1;
}

int
template_details_get(const char *id, const char **name, const char **icon) {
    Template *temp = find_temp(id, NULL);

    if (!temp) return 0;

    *name = temp->name;
    *icon = temp->icon;

    return 1;
}

int
template_run(const char *id) {
    Template *temp = find_temp(id, NULL);

    if (!temp) return 0;

    temp->cb(temp->data);

    return 1;
}

int
template_get(char ***template_ids, unsigned int *num) {
    int count = array_len_get(array);
    char **result;

    result = calloc(count, sizeof(char*));

    for (int i = 0; i < count; i++)
      {
         Template *t;

         t = array_Template_get(array, i);

         if (!t) continue;

         result[i] = t->id;
      }
    *template_ids = result;
    *num = count;

    return 1;
}
