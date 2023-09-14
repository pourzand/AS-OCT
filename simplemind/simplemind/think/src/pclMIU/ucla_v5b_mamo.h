#ifndef _ucla_v5b_mamo_h_
#define _ucla_v5b_mamo_h_

#define MAX_ACCESS 5
#define NAME_LEN 80
#define DATE_LEN 9
#define ID_LEN 11
#define CODE_LEN 6
#define MAX_MARK 50
#define MAXFILENAME 80
#define ASSOC_FILE 3

/* Marker location */
typedef struct MARK {
  unsigned int x;
  unsigned int y;
} MARK;

/*  Accociated files */
typedef struct ASSOC_FILES {
  char name[ASSOC_FILE][MAXFILENAME];
} ASSOC_FILES;


/*  Transcription code */
typedef struct TRANSCIPT_CODE {

  // image filename (database key)
  char file[MAXFILENAME];

  // database base name 
  char dbname[MAXFILENAME];

  // database table name 
  char dbtable[MAXFILENAME];

} TRANSCRIPT_CODE;

typedef struct PHY_ACCESS {
  // Access date
  char date[DATE_LEN]; // YYYYMMDD

  // Physician who access this file
  char phy_name[NAME_LEN];

  // Physician ID 
  char phy_id[ID_LEN];
  
  // BIRAD code given in this access
  char birad_code[CODE_LEN];  

  // MARKER LOCATIONS for this access
  MARK marks[MAX_MARK];

  // Count how many marker is used.
  unsigned marks_count;

  // Info. regarding to obtain the transcript code for this access
  TRANSCRIPT_CODE t_code;

} PHY_ACCESS;

typedef struct PACS_SUBHEADER_MAMO
{
  // PHY_ACCESS
  PHY_ACCESS access[MAX_ACCESS];

  // Associated files
  ASSOC_FILES assoc_file;
 
}PACS_SUBHEADER_MAMO;



#endif
