// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "sfiustore.hh"
#include "sfimemory.hh"
#include "bse/internal.hh"

/* --- strcutures --- */
static inline SfiUStore*
scast (GTree *tree)
{
  return (SfiUStore*) tree;
}
static inline GTree*
tcast (SfiUStore *store)
{
  return (GTree*) store;
}


/* --- unique ID store --- */
static gint
ustore_cmp (gconstpointer a,
	    gconstpointer b)
{
  gulong u1 = (gulong) a;
  gulong u2 = (gulong) b;
  return u1 < u2 ? -1 : u1 != u2;
}

SfiUStore*
sfi_ustore_new (void)
{
  SfiUStore *store = scast (g_tree_new (ustore_cmp));

  return store;
}

gpointer
sfi_ustore_lookup (SfiUStore *store,
		   gulong     unique_id)
{
  assert_return (store != NULL, NULL);

  return g_tree_lookup (tcast (store), (gpointer) unique_id);
}

void
sfi_ustore_insert (SfiUStore *store,
		   gulong     unique_id,
		   gpointer   value)
{
  assert_return (store != NULL);

  if (!value)
    g_tree_remove (tcast (store), (gpointer) unique_id);
  else
    g_tree_insert (tcast (store), (gpointer) unique_id, value);
}

void
sfi_ustore_remove (SfiUStore *store,
		   gulong     unique_id)
{
  assert_return (store != NULL);

  g_tree_remove (tcast (store), (gpointer) unique_id);
}

typedef struct {
  gpointer         data;
  SfiUStoreForeach foreach;
} FData;

static gboolean
foreach_wrapper (gpointer key,
		 gpointer value,
		 gpointer data)
{
  FData *fdata = (FData*) data;
  /* iterate as long as SfiUStoreForeach() returns TRUE */
  return !fdata->foreach (fdata->data, (gulong) key, value);
}

void
sfi_ustore_foreach (SfiUStore       *store,
		    SfiUStoreForeach foreach,
		    gpointer         data)
{
  FData fdata;

  assert_return (store != NULL);

  fdata.data = data;
  fdata.foreach = foreach;
  g_tree_foreach (tcast (store), foreach_wrapper, &fdata);
}

void
sfi_ustore_destroy (SfiUStore *store)
{
  assert_return (store != NULL);

  g_tree_destroy (tcast (store));
}

/* --- unique ID pool --- */
#define UPOOL_TAG ((gpointer) sfi_upool_new)

static inline SfiUPool*
upool_cast (SfiUStore *ustore)
{
  return (SfiUPool*) ustore;
}

static inline SfiUStore*
ustore_cast (SfiUPool *upool)
{
  return (SfiUStore*) upool;
}

SfiUPool*
sfi_upool_new (void)
{
  return upool_cast (sfi_ustore_new ());
}

gboolean
sfi_upool_lookup (SfiUPool *pool,
		  gulong    unique_id)
{
  return sfi_ustore_lookup (ustore_cast (pool), unique_id) != NULL;
}

void
sfi_upool_set (SfiUPool *pool,
	       gulong    unique_id)
{
  sfi_ustore_insert (ustore_cast (pool), unique_id, UPOOL_TAG);
}

void
sfi_upool_unset (SfiUPool *pool,
		 gulong    unique_id)
{
  sfi_ustore_remove (ustore_cast (pool), unique_id);
}

void
sfi_upool_foreach (SfiUPool        *pool,
		   SfiUPoolForeach  foreach,
		   gpointer         data)
{
  sfi_ustore_foreach (ustore_cast (pool), (SfiUStoreForeach) foreach, data);
}

typedef struct {
  guint   capacity;
  guint   n_ids;
  gulong *ids;
} UPoolList;

static gboolean
upool_enlist (gpointer        data,
              gulong          unique_id)
{
  UPoolList *list = (UPoolList*) data;
  guint i = list->n_ids++;
  if (list->n_ids > list->capacity)
    {
      list->capacity = sfi_alloc_upper_power2 (list->n_ids);
      list->ids = g_renew (gulong, list->ids, list->capacity);
    }
  list->ids[i] = unique_id;
  return TRUE;
}

gulong*
sfi_upool_list (SfiUPool        *pool,
                guint           *n_ids)
{
  UPoolList list = { 0 };
  sfi_upool_foreach (pool, upool_enlist, &list);
  if (n_ids)
    *n_ids = list.n_ids;
  return list.ids;
}

void
sfi_upool_destroy (SfiUPool *pool)
{
  sfi_ustore_destroy (ustore_cast (pool));
}


/* --- pointer pool --- */
#define PPOOL_TAG ((gpointer) sfi_ppool_new)

static inline SfiPPool*
ppool_cast (GTree *tree)
{
  return (SfiPPool*) tree;
}

static inline GTree*
ppool_tree (SfiPPool *pool)
{
  return (GTree*) pool;
}

static gint
ppool_cmp (gconstpointer a,
           gconstpointer b)
{
  const char *c1 = (const char*) a;
  const char *c2 = (const char*) b;
  return c1 < c2 ? -1 : c1 != c2;
}

SfiPPool*
sfi_ppool_new (void)
{
  return ppool_cast (g_tree_new (ppool_cmp));
}

gboolean
sfi_ppool_lookup (SfiPPool *pool,
		  gpointer  unique_ptr)
{
  assert_return (pool != NULL, FALSE);
  return g_tree_lookup (ppool_tree (pool), unique_ptr) != NULL;
}

void
sfi_ppool_set (SfiPPool *pool,
	       gpointer  unique_ptr)
{
  assert_return (pool != NULL);
  g_tree_insert (ppool_tree (pool), unique_ptr, PPOOL_TAG);
}

void
sfi_ppool_unset (SfiPPool *pool,
		 gpointer  unique_ptr)
{
  assert_return (pool != NULL);
  g_tree_remove (ppool_tree (pool), unique_ptr);
}

typedef struct {
  gpointer        data;
  SfiPPoolForeach foreach;
} PPoolData;

static gboolean
ppool_foreach_wrapper (gpointer key,
                       gpointer value,
                       gpointer data)
{
  PPoolData *pdata = (PPoolData*) data;
  /* iterate as long as SfiPPoolForeach() returns TRUE */
  return !pdata->foreach (pdata->data, key);
}

void
sfi_ppool_foreach (SfiPPool        *pool,
		   SfiPPoolForeach  foreach,
		   gpointer         data)
{
  PPoolData pdata;
  assert_return (pool != NULL);
  pdata.data = data;
  pdata.foreach = foreach;
  g_tree_foreach (ppool_tree (pool), ppool_foreach_wrapper, &pdata);
}

static gboolean
ppool_foreach_slist (gpointer key,
                     gpointer value,
                     gpointer data)
{
  GSList **slist_p = (GSList**) data;
  *slist_p = g_slist_prepend (*slist_p, key);
  return FALSE; /* always continue */
}

GSList*
sfi_ppool_slist (SfiPPool *pool)
{
  GSList *slist = NULL;
  assert_return (pool != NULL, NULL);
  g_tree_foreach (ppool_tree (pool), ppool_foreach_slist, &slist);
  return slist;
}

void
sfi_ppool_destroy (SfiPPool *pool)
{
  assert_return (pool != NULL);
  g_tree_destroy (ppool_tree (pool));
}
