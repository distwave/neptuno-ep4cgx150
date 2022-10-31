//
// Multicore 2
//
// Copyright (c) 2017-2020 - Victor Trucco
//
// Additional code, debug and fixes: Diogo Patrão
//
// All rights reserved
//
// Redistribution and use in source and synthezised forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// Redistributions in synthesized form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// Neither the name of the author nor the names of other contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// You are responsible for any legal issues arising from your use of this code.
//


// codes for option_nums
#define OPTION_LOAD_NEW_CORE 98
#define OPTION_LOAD_FILE 99
#define OPTION_TOGGLE 1


// increment ptr until (1) the end of sd_buffer or (2) the next occurence of delimiter
// return the pointer to the next occurence of delimiter PLUS ONE, or NULL if end of string was reached
char* get_next_line( char delimiter, char* ptr ) {
  while( *ptr != '\0' && *ptr != delimiter ) {
    ptr++;
  }
  if ( *ptr == '\0' ) {
    return NULL;
  } else {
    return ptr+1;
  }
}

// get total number of lines
int get_line_count() {
  char* ptr = sd_buffer;
  int size = 0;
  while( ptr != NULL ) {
    size ++;
    ptr = get_next_line( ';', ptr );
  }
  return size;
}

// get total number of showable lines
int get_showable_line_count() {
  char* ptr = sd_buffer;
  int size = 0;
  while( ptr != NULL ) {
    if ( *ptr == 'T' || *ptr == 'R' || *ptr == 'O' || *ptr == 'S' ) {
      size ++;
    }
    ptr = get_next_line( ';', ptr );
  }
  return size;
}

// get total of options on current place
int get_option_count( char* ptr ) {
  if ( *ptr == '\0' ) {
    return 0;
  }
  int options = 1;
  while( *ptr != '\0' && *ptr != ';' ) {
    if ( *ptr == ',' ) {
      options++;
    }
    ptr++;
  }
  return options;
}

// copy src to dst until a delimiter or '\0' is found
void copy_till_delimiter( char delimiter, char* src, char* dst, bool end_string=1 ) {
  while( *src != '\0' && *src != delimiter ) {
    *dst = *src;
    dst ++;
    src ++;
  }
  if ( end_string ) {
    *dst = '\0';
  }
}
/**
 * search and copy to dst the extensions specified by the LOAD option
 * with s_index (0-9)
 * returns 1 if LOAD option with specified index is found, 0 otherwise
 */
int get_extension_for_load_option( int s_index, char* dst ) {

  char* ptr = sd_buffer;

  dst[0] = '\0';
  while( ptr != NULL ) {

    // only load is relevant here
    if ( ptr[ 0 ] != 'S' ) {
      ptr = get_next_line( ';', ptr );
      continue;
    }
    mc_info( ">> found S\n" );
    int index;

    if ( ptr[1] == ',' ) {
      index = 1;
    } else {
      index = (int)( ptr[1] - '0' );
    }

    // only load with specified index is relevant
    if ( s_index != index ) {
      mc_info( ">> skip - wanted " );
      mc_info( s_index );
      mc_info( "found " );
      mc_info( index );
      mc_info( "\n" );
      ptr = get_next_line( ';', ptr );
      continue;
    }

    // get extensions
    ptr = get_next_line( ',', ptr );
    mc_info( ">> current ptr:" );
    mc_info( ptr );
    mc_info( "\n" );
    copy_till_delimiter( ',', ptr, dst );

    return 1;
  }
  return 0;

}


void clear_option_data(  char line_option[ MENU_MAX_LINES ][ OPTION_LEN ], char option_values[ MENU_MAX_LINES ][ OPTION_LEN ], unsigned int option_map[ MAX_OPTIONS ], int option_load_slot[ MAX_OPTIONS ] ){

    mc_info("Entering clear_option_data\n");
    for( int i=0;i<MENU_MAX_LINES;i++ ) {
        line_option[i][0] = '\0';
        option_values[i][0] = '\0';
    }
    for( int i=0;i<MAX_OPTIONS;i++ ) {
        option_map[i] = 0;
        option_load_slot[i] = 0;
    
        // globals
        option_num[i] = 0;
        //ption_sel[i] = 0;
    }
    mc_info("Finishing clear_option_data\n");
    
}


/**
 * parse options stored in sd_buffer.
 * Titles are stored in "line_option"
 * Option Values are stored in "option_values"
 * In the "load" option, extensions are stored in exts (not a good place, i guess, should bug if more than one Load option is available)
 * parse only from "start" entry, up to "length" entries (entries are whole lines)
 * 
 */
void parseOptions( char line_option[ MENU_MAX_LINES ][ OPTION_LEN ], char option_values[ MENU_MAX_LINES ][ OPTION_LEN ], unsigned int option_map[ MAX_OPTIONS ], int option_load_slot[ MAX_OPTIONS ], int start, int length ) {
  // this while block parses the INI file (in the global sd_buffer)

  // parsing pointer
  char* ptr = sd_buffer;

  // ini file parse position
  int ini = 0;
  
  // ini file total length (to be set)
  int fim = 0;

  // general iterators
  int i,j,k;

  //
  int unrecognized;

  mc_info("Entering parseOptions\n");
  mc_info( "start: " );
  mc_info( start );
  mc_info( " length: " );
  mc_info( length);
  mc_info( "\n" );

  // index of currently shown options(used in option_values and line_option)
  int i_line = 0;

  // index of overall options   
  int cur_line = 0;
  
  // index of valid options (used in option_num and option_sel)
  int opt_sel  = 0;
  
  // parse the required lines
  while ( ptr != NULL ) {

    // parse specific options from the line
    char* subptr;

    mc_info( "Parsing line " );
    mc_info( cur_line );
    mc_info( " opt_sel=" );
    mc_info( opt_sel );
    mc_info( "\n" );

    // get first token, that specifies the option type
    subptr = ptr;

    mc_info( ">>> subptr:" );
    mc_info( subptr );
    mc_info( "\n" );

    unrecognized = 0; // flag when an option is not showable

    if ( subptr[ 0 ] == 'S') {
      //it's a LOAD option
      
      unsigned char s_index;
      
      if ( subptr[ 1 ] != ',' ) {
        s_index = codeToNumber( subptr[ 1 ] );
      }
      else {
        //default to "drive 1"
        s_index = 1; 
      }

      // set behaviour
      option_num[ opt_sel ] = OPTION_LOAD_FILE;
      option_sel[ opt_sel ] = 0; 
      option_load_slot[ opt_sel ] = s_index;

      // next token is the extensions - ignore for now
      subptr = get_next_line( ',', subptr );

      // TODO migrate to navigateOptions!!!!
      // if ( cur_select == cur_line ) {
      //  copy_till_delimiter( ',', subptr, exts );
      // }

      if ( start <= opt_sel && i_line < length) {

        mc_info( "Will be added to shown items" );

        // copy the title to line_option, padded with spaces
        subptr = get_next_line( ',', subptr );
        memset( line_option[ i_line ], ' ', SCREEN_WIDTH_IN_CHARS );
        copy_till_delimiter( ';', subptr, line_option[ i_line ], 0 );  
        line_option[ i_line ][ SCREEN_WIDTH_IN_CHARS ] = '\0';
              
        // clear options
        option_values[ i_line ][ 0 ] = '\0';

        // map shown option to overall options
        option_map[ i_line ] = opt_sel;

        i_line ++;
      }
                  
    }   
    //it's an toggle or "load another core" option
    else if ( subptr[ 0 ] == 'T' || subptr[ 0 ] == 'R' ) {

      // get identifier
      int num_op_token = codeToNumber( subptr[ 1 ] );

      mc_info( "Toggle or load another core option - ");
      mc_info( num_op_token );
      mc_info( "\n" );

      // set behaviour
      if ( subptr[ 0 ] == 'R' ) {
        option_num[ opt_sel ] = OPTION_LOAD_NEW_CORE;
      } else {
        option_num[ opt_sel ] = OPTION_TOGGLE; // hm 1;
        mc_info("Option_num [");
        mc_info( cur_line );
        mc_info("] = ");
        mc_info( option_num[ opt_sel ] );
        mc_info( "\n" );
      }
      option_sel[ opt_sel ] = ( num_op_token << 3 ); 
      option_load_slot[ opt_sel ] = 0;
      
      mc_info( "Num op token" );
      mc_info( num_op_token );
      mc_info( "\n" );

      if ( start <= opt_sel && i_line < length ) {

        mc_info("Will be added to shown items\n");

        // copy the title to line_option, padded with spaces
        subptr = get_next_line( ',', subptr );
        memset( line_option[ i_line ], ' ', SCREEN_WIDTH_IN_CHARS );
        copy_till_delimiter( ';', subptr, line_option[ i_line ], 0 );  
        line_option[ i_line ][ SCREEN_WIDTH_IN_CHARS ] = '\0';
        
        // clear options
        option_values[ i_line ][0] = '\0';

        // map shown option to overall options
        option_map[ i_line ] = opt_sel;

        i_line ++;
      }
    }            
    // it's an Option for menu
    else if ( subptr[ 0 ] == 'O' ) {

      // get identifier
      int num_op_token = codeToNumber( subptr[ 1 ] );

      if ( start <= opt_sel && i_line < length ) {
        
        mc_info("Will be added to shown items\n");

        // copy the title to line_option, padded with spaces
        // TODO put ":" after title
        subptr = get_next_line( ',', subptr );
        memset( line_option[ i_line ], ' ', SCREEN_WIDTH_IN_CHARS );
        copy_till_delimiter( ',', subptr, line_option[ i_line ], 0 );  
        line_option[ i_line ][ SCREEN_WIDTH_IN_CHARS ] = '\0';
      
        // get each option value to the array
        // TODO produce warning when copied string is longer than OPTION_LEN
        subptr = get_next_line( ',', subptr );
        copy_till_delimiter( ';', subptr, option_values[ i_line ], 1 );

        // map shown option to overall options
        option_map[ i_line ] = opt_sel;

        // set number of options
        option_num[ opt_sel ] = get_option_count( option_values[ i_line ] );
      
        i_line ++;

      } else {
        // discard title
        subptr = get_next_line( ',', subptr );

        // set number of options
        option_num[ opt_sel ] = get_option_count( get_next_line( ',', subptr ) );
      }

      // set behaviour
      option_sel[ opt_sel ] = ( num_op_token << 3) | (option_sel[ opt_sel ] & 0x07); 
      option_load_slot[ opt_sel ] = 0;
      
    } else { // end if 'O'
      mc_info( "Skipping option type '");
      mc_info( subptr[0] );
      mc_info( "'\n" );
      unrecognized = 1;
    }

    if ( ! unrecognized ) {
      opt_sel ++;
    }

    // go to next line
    ptr = get_next_line( ';', ptr );
    cur_line ++;
  } // end for

  mc_info( "Finished parsing. Result:" );
  for( int i=0;i<length;i++ ) {
    mc_info("linha: ");
    mc_info( i );
    mc_info(" titulo: ");
    mc_info( line_option[ i ] );
    mc_info(" option_values: ");
    mc_info( option_values[ i ] );
    mc_info(" option_map: ");
    mc_info( option_map[ i ] );
    mc_info(" option_num: ");
    mc_info( option_num[ option_map [ i ] ] );
    mc_info(" option_sel: ");
    mc_info( option_sel[ option_map[ i ] ] );
    mc_info(" load_slot: ");
    mc_info( option_load_slot[ i ] );
    mc_info( "\n" );
  }

}

// perform tests (not yet automated) for parseOptions and get_extensions
void testParseOptions(  char line_option[ MENU_MAX_LINES ][ OPTION_LEN ], char option_values[ MENU_MAX_LINES ][ OPTION_LEN ], unsigned int option_map[ MAX_OPTIONS ], int option_load_slot[ MAX_OPTIONS ], int start, int length  ) {
#if MC_DEBUG > 0
  char tmp[1024];
  char exts[32];
  memcpy( tmp, sd_buffer, 1024 );
  strcpy( sd_buffer, "S0,NES,Load NES Game...;S1,FDS,Load FDS Game...;S2,NSF,Load NSF Sound...;S3,BIN,Load FDS BIOS..." );    
  Serial.println("--- initiating testParseOptions #1" );
  parseOptions( line_option, option_values, option_map, option_load_slot, 0, MENU_MAX_LINES );
  Serial.println("--- end of testParseOptions #1" );
  Serial.println("--- initiating testGetExtensions #1" );
  for( int s_index = 0; s_index<4; s_index++ ) {
    get_extension_for_load_option( s_index, exts );
    Serial.print("extensions for load[ ");
    Serial.print( s_index );
    Serial.print(" ] = " );
    Serial.println( exts );
  }
  Serial.println("--- end of testGetExtensions #1" );

  strcpy( sd_buffer, "OHK,Palette,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15;" );    
  Serial.println("--- initiating testParseOptions #2" );
  parseOptions( line_option, option_values, option_map, option_load_slot, 0, MENU_MAX_LINES );
  Serial.println("--- end of testParseOptions #2" );


  strcpy( sd_buffer, "P,Galaga.ini;O78,Screen Rotation,0,90,180,270;O34,Scanlines,Off,25%,50%,75%;O5,Blend,Off,On;O9,Scandoubler,On,Off;T6,Reset;V,v1.21." );
  Serial.println("--- initiating testParseOptions #3" );
  parseOptions( line_option, option_values, option_map, option_load_slot, 0, MENU_MAX_LINES );
  Serial.println("--- end of testParseOptions #3" );

  memcpy( sd_buffer, tmp, 1024 );
#endif
}

bool navigateOptions() {
  //char sd_buffer[1024] {
  // "S,PRG,Load *.PRG;S,TAP/TZX,Load *.TAP;TC,Play/Stop TAP;OG,Menos sound,Off,On;OH,Mais sound,Off,On;OD,Tape sound,Off,On;O3,Video,PAL,NTSC;O2,CRT with load address,Yes,No;OAB,Scanlines,Off,25%,50%,75%;O45,Enable 8K+ Expansion,Off,8K,16K,24K;O6,Enable 3K Expansion,Off,On;O78,Enable 8k ROM,Off,RO,RW;O9,Audio Filter,On,Off;T0,Reset;T1,Reset with cart unload;." 
  // };


  mc_info( "Entering navigateOptions()" );
  
  int cur_line = 0;
  int page = 0;
  int i,j,k;
  int opt_sel = 0;
  int total_options = 0; 
  int total_show = 0; // DEPRECATE
  int last_page = 0; // DEPRECATE

  int show_debug = 1;
  
  // keyboard events
  int event; 

  // extensions for loading files
  char exts[ 32 ];

  OsdClear( );

  //show the menu to navigate
  OSDVisible( true );

  cur_select = 0;

  // menu items are called "options"
  // 8 = one per line
  char line_option[ MENU_MAX_LINES ][ OPTION_LEN ];

  // possible responses to options are called "values"
  char option_values[ MENU_MAX_LINES ][ OPTION_LEN ];

  // possible responses to options are called "values"
  int option_load_slot[ MAX_OPTIONS ];

  // map 0..MENU_MAX_LINE to the option raw index (i.e., the ordinal in which
  // the option actually appears. that maps option_vaues and line_option
  // to globals option_num and option_sel, in which i dare not mess yet)
  unsigned int option_map[ MENU_MAX_LINES ];

  // clear globals and local variables
  clear_option_data( line_option, option_values, option_map, option_load_slot );

  // retrieve total number of options
  total_options = get_showable_line_count( );

  mc_info("total_options: ");
  mc_info( total_options );
  mc_info( "\n" );

  // parse options from current page
  //testParseOptions( line_option, option_values, option_map, option_load_slot, page * MENU_MAX_LINES, MENU_MAX_LINES );

  //strcpy( sd_buffer, "S,*,Load *.*;T0,Reset;" );
  parseOptions( line_option, option_values, option_map, option_load_slot, page * MENU_MAX_LINES, MENU_MAX_LINES );

  // the options navigation loop
  while( true ) {

    // display options on screen
    char line[128];
    char tmp[128];
    bool selected;
    for ( int j=0; j < MENU_MAX_LINES; j++ ) {

      int i_opt = j + page * MENU_MAX_LINES;

      if ( show_debug == 1 ) {
        mc_info( "Line: " );
        mc_info( j );
        mc_info( " - " );
        mc_info( i_opt );
        mc_info( "\n" );
      }
      
      if ( i_opt < total_options ) {

        // copy title to line
        strcpy( line, line_option[ j ] );

        // copy option (if any) to end of line
        if ( option_num[ option_map[ j ] ] != OPTION_LOAD_NEW_CORE && option_num[ option_map[ j ] ] != OPTION_LOAD_FILE && option_num[ option_map[ j ] ] > 1 ) {

          int option_selected = option_sel[ option_map[ j ] ] & 0x07;  
          strcpy( tmp, option_values[ j ] );

          if ( show_debug == 1 ) {
            mc_info( "Option[ ");
            mc_info( j );
            mc_info( " ] = ");
            mc_info( line_option[j] );
            mc_info( " num=");
            mc_info( option_num[ option_map[ j ] ] );
            mc_info( " sel=");
            mc_info( option_sel[ option_map[ j ] ] );
            mc_info( " val=");
            mc_info( option_selected );
            mc_info( "\n" );
          }
          
          char *token = strtok( tmp, "," );          
          while( option_selected > 0 ) {
             token = strtok( NULL, "," ); 
             option_selected --;
          }
          if ( show_debug == 1 ) {
            mc_info( "    selected:");
            mc_info( token );
            mc_info( "\n" );
          }

          strcpy( line + SCREEN_WIDTH_IN_CHARS - 1 - strlen( token ), token );
        }
        
        if ( show_debug == 1 ) {
          mc_info( "Line: " );
          mc_info( j );
          mc_info( " - " );
          mc_info( line );
          mc_info( "\n" );
        }
        // toggle color inversion if it's the selected line
        selected = ( cur_select == ( j + page * MENU_MAX_LINES ) );
      } else {
        strcpy( line, "                                " );
        selected = false;
      }
      OsdWriteOffset( j, line, selected, 0, 0, 0);
    }

    show_debug = 0;
    // set second SPI on PA12-PA15
    SPI.setModule( SPI_FPGA );
    
    // ensure SS stays high for now
    SPI_DESELECTED();
    //NSS_SET; 
    SPI_SELECTED();
    //NSS_CLEAR;
    
    // read keyboard and act
    event = readKeyboard( &key, &cmd );

    if ( event == EVENT_KEYPRESS ) {
//      Serial.println("Key pressed");

      if ( key == KEY_UP ) {
        if ( cur_select > 0 ) {
          cur_select--;
        } else
        {
          cur_select = total_options - 1;
        }
      }
      else if ( key == KEY_DOW ) {
        if ( cur_select < total_options - 1 ) {
          cur_select++; 
        } else {
          cur_select = 0;
        }
      }
      else if ( key == KEY_LFT ) { 
        // TODO probably won't work due to "recalculate page" below
        if ( page > 0 ) {
          page--;  
        }
      }
      else if ( key == KEY_RGT ) {
        // TODO probably won't work due to "recalculate page" below
        if ( page < ( ( total_options - 1 ) / MENU_MAX_LINES ) ) {
          page++; 
        }
      }
      else if ( key == KEY_RET ) {
        mc_info( "Option selected: " );
        mc_info( option_num[ cur_select ] );
        mc_info( "\n" );
        if ( option_num[ cur_select ] == OPTION_LOAD_FILE ) {  

          int s_index = option_load_slot[ cur_select  ];

          mc_info( "Option_load_slot[ ");
          mc_info( cur_select  );
          mc_info( "] = ");
          mc_info( s_index );
          mc_info( "\n" );

          get_extension_for_load_option( s_index, exts );
          mc_info( "Extensions for filtering: " );
          mc_info( exts );
          mc_info( "\n" );

          transfer_index = s_index ;
          OsdClear();
  
          //Serial.print(ext);Serial.println("-----------------------------");
          if ( navigateMenu( exts, true, false, false, true ) ) {

            SPI_DESELECTED();
            SPI_SELECTED();
            
            mc_info(" we will execute a data pump 3!\n ");
            dataPump();
            
            OSDVisible(false);
            last_cmd = 1;
            last_key = 31;
            return(true);
          }
        }
        // load a new core
        else if ( option_num[ cur_select ] == OPTION_LOAD_NEW_CORE ) {
          mc_info( "Loadn a new core\n" );
          menuLoadNewCore( false );
          return false;
        }
        // toggle
        else if ( option_num[ cur_select ] == OPTION_TOGGLE ) {
          int num_op_token = option_sel[ cur_select ] & 0xf8; // 11111000b
                 
          option_sel[cur_select] = num_op_token | 0x01;

          mc_info("Press reset\n");

          SendStatusWord();
          delay( 300 );

          mc_info("Release reset\n");
          option_sel[cur_select] = num_op_token;
          SendStatusWord();
        }
        // option
        else {
          int num_op_token = option_sel[ cur_select ] & 0xf8; // 11111000b
          
          mc_info( line_option[ cur_select - j *  MENU_MAX_LINES ] );
          mc_info( " ( " );
          mc_info( num_op_token );
          mc_info( " ): " );
          mc_info( option_sel[ cur_select ] );
          mc_info( " => " );

          option_sel[ cur_select ]++;
          if ( ( option_sel[ cur_select ] & 0x07 ) == ( option_num[ cur_select ] ) && option_num[ cur_select ] > 1 ) {
              option_sel[ cur_select ] = num_op_token;
          }

          mc_info( option_sel[ cur_select ] );
          mc_info( "\n" );
          SendStatusWord();

        }
      } 
      //F12 - abort
      else if ( ( cmd == 0x01 || cmd == 0x02 || cmd == 0x03 ) ) { 
  
        if ( menu_opened + 1000 < millis( ) )
            return true;  
      }

      // recalculate page
      if ( page != cur_select / MENU_MAX_LINES ) {
        page = cur_select / MENU_MAX_LINES;
        parseOptions( line_option, option_values, option_map, option_load_slot, page * MENU_MAX_LINES, MENU_MAX_LINES );
        OsdClear( );
      }

    } // end if keypress

              
  } // end while     
}
