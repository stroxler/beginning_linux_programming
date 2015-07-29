// a struct to represent a cd. Note it corresponds almost exactly to the sql
// the only exception is that the artist_name, which has its own sql table,
// is placed in the struct.
struct current_cd_st {
    int artist_id;
    int cd_id;
    char artist_name[100];
    char title[100];
    char catalogue[100];
};

// a struct to hold track information. We allow 19 tracks at most, because the
// code would be a lot more complex if this were dynamic.
//    (19, not 20, because an entry with '\0' in the 1st char is used to
//     indicate the end of the data)
struct current_tracks_st {
    int cd_id;
    char track[20][100];
};


// a struct to hold the ids from a find_cd command. We don't allow more than
// 10 -- again, making this dynamic would lead to much more complex code.
#define MAX_CD_RESULT 10
struct cd_search_st {
    int cd_id[MAX_CD_RESULT];
};

// database handling
int  database_start(char *name, char *password);
void database_end();

// adding a cd
int add_cd(char *artist, char *title, char *catalogue, int *cd_id);
int add_tracks(struct current_tracks_st *tracks);

// finding and retrieving a cd
int find_cds(char *search_str, struct cd_search_st *results);
int get_cd(int cd_id, struct current_cd_st *dest);
int get_cd_tracks(int cd_id, struct current_tracks_st *dest);

// delete a cd
int delete_cd(int cd_id);
