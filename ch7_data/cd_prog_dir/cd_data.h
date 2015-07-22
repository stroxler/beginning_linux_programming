/* define the dbm 'schema' */
#define CAT_LEN 30
#define TITLE_LEN 70
#define TYPE_LEN 30
#define ARTIST_LEN 70
#define TTEXT_LEN 70

// the catalog 'table' datatype
typedef struct {
    char catalog[CAT_LEN + 1];
    char title[TITLE_LEN + 1];
    char type[TYPE_LEN + 1];
    char artist[ARTIST_LEN + 1];
} cdc_entry;

// the tracks 'table' datatype
typedef struct {
    char catalog[CAT_LEN + 1];
    int track_no;
    char track_txt[TTEXT_LEN + 1];
} cdt_entry;

/* Initialization and termination functions */
int database_initialize(const int new_database);
void database_close(void);

/* simple data retrieval */
cdc_entry get_cdc_entry(const char *cd_catalog_ptr);
cdt_entry get_cdt_entry(const char *cd_catalog_ptr, const int track_no);

/* data addition */
int add_cdc_entry(const cdc_entry entry_to_add);
int add_cdt_entry(const cdt_entry entry_to_add);

/* data deletion */
int del_cdc_entry(const char *cd_catalog_ptr);
int del_cdt_entry(const char *cd_catalog_ptr, const int track_no);

/* search */
cdc_entry search_cdc_entry(const char *cd_catalog_ptr, int *first_call_ptr);
