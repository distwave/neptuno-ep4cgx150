

// show currentPage of results (MENU_MAX_LINES)
void show_files(  SPI_Directory orderedFiles[MAX_SORTED_FILES], char extensions[ MAX_PARSED_EXTENSIONS ][ MAX_LENGTH_EXTENSION ], int totalExtensions, int showExtensions, int currentPage, int currentLine, int totalFiles, int *rolling_offset, int *idleTime ) {
  int j;

  for( j = 0; j < MENU_MAX_LINES; j ++ ) {

    // erase lines without files
    if ( ( j + currentPage * MENU_MAX_LINES ) >= totalFiles ) {
      OsdWriteOffset( j, "", 0, 0, 0, 0 ); 
      continue;
    }
    
    int k;
    char temp_filename[64];
    SPI_Directory *d = &( orderedFiles[ j + currentPage * MENU_MAX_LINES ] );
    
    // skips one byte if its a dir so I can put <> around the dirname
    if ( d->entry_type == SPI_Directory::dir ) {
      strcpy( temp_filename+1, d->filename ); 
      temp_filename[0] = '<';
      temp_filename[strlen(temp_filename)] = '>';
      k = strlen( d->filename )+2;
    } else {
      strcpy( temp_filename, d->filename ); 
      k = strlen( d->filename );
    
      // remove filename extension (WARNING: WORKS ONLY WITH ONE EXTENSION)
      if ( endsWith( temp_filename, extensions[0] ) && !showExtensions && totalExtensions==1) {
        k -= strlen(extensions[0]);
      }
    }
    
    // pad rest of string with spaces (in order to erase garbage from previous screen)
    while( k < 31 ) {
      temp_filename[ k++ ] = ' ';
    }
    temp_filename[ k ] = '\0';

    // display unselected file
    if ( j != ( currentLine % MENU_MAX_LINES ) ) {
      OsdWriteOffset( j, temp_filename, 0, 0,0,0 );           
    } else {
      // display selected file
      
      // do not roll files that fit the screen 
      if ( strlen( temp_filename ) < SCREEN_WIDTH_IN_CHARS ) {
        *rolling_offset = 0;
      } else {


        // if rolled all the way to the end of string, stop, wait 2sec and start again
        if ( *rolling_offset + SCREEN_WIDTH_IN_CHARS > strlen( temp_filename )  ) {                
          if ( ( millis( ) - *idleTime ) > 2000 ) {
            *idleTime = millis( );
            *rolling_offset = 0;
          }
        } else
        // if at start of string, wait 2sec to start rolling, then go 1 char each 0.2sec
        if ( ( *rolling_offset == 0 && (millis( ) - *idleTime ) > 2000 ) || ( *rolling_offset > 0 && (millis() - *idleTime ) > 200 ) ) {                 
          *idleTime = millis( );
          (*rolling_offset)++;
        }
      }
      OsdWriteOffset( j, temp_filename + ( *rolling_offset), 1, 0, 0, 0 );
    }
  
  } // end for
  

}
