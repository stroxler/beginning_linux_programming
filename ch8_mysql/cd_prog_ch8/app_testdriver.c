#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "app_mysql.h"

int main() {
     struct current_cd_st cd;
     struct cd_search_st cd_res;
     struct current_tracks_st ct;
     int cd_id;
     int res, i;

     database_start("trox", "trox");

     // add a new cd. We get the cd_id back from the data code.
     res = add_cd("Mahler", "Symphony No 1", "4596102", &cd_id);  
     printf("Result of adding a cd was %d, cd_id is %d\n", res, cd_id);

     // add tracks for the new cd. Note how we need to zero out bytes first.
     memset(&ct, 0, sizeof(ct));
     ct.cd_id = cd_id;
     strcpy(ct.track[0], "Lansam Schleppend");
     strcpy(ct.track[1], "Kraftig bewegt");
     strcpy(ct.track[2], "Feierlich und gemesen");
     strcpy(ct.track[3], "Sturmisch bewegt");
     add_tracks(&ct);

     // search for the cd we just added. There's a struct to hold search data,
     // which could include many cds. It returns the ids.
     res = find_cds("Symphony", &cd_res);
     printf("Found %d cds, first has ID %d\n", res, cd_res.cd_id[0]);

     // to get full data on any of the results, look it up by id
     res = get_cd(cd_res.cd_id[0], &cd);
     printf("get_cd returned %d\n", res);

     // ... you can also look up the tracks once you have the id
     // note that we need to zero out the bytes again
     memset(&ct, 0, sizeof(ct));
     res = get_cd_tracks(cd_res.cd_id[0], &ct);  // returns num tracks in `res`
     printf("get_cd_tracks returned %d\n", res);
     for (i = 0; i < res; i++) {
         printf("\ttrack %d is %s\n", i, ct.track[i]);
     }

     // delete the cd we just added
     res = delete_cd(cd_res.cd_id[0]);
     printf("Delete_cd returned %d\n", res);

     // cleanup
     database_end();
     return 0;
}
