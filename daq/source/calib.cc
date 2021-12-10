#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <string>


#include <CAENDigitizer.h>

#define MAX_READ_CHAR               1000
#define MAX_BASE_INPUT_FILE_LENGTH  1000

int SaveCorrectionTables(uint32_t groupMask, CAEN_DGTZ_DRS4Correction_t *tables);


//==========================================================================================
int main(int argc, char **argv) {
  // ======================= INIT =====================
  std::cout << std::endl;
  std::cout << "*** Retrieving calibration tables: ";
  int fHandle;
  CAEN_DGTZ_ErrorCode fLastErrorCode;
  fLastErrorCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, // link type
					   0, // link num
					   0, // conetNode
					   0, // VMEBaseAddress
					   &fHandle);
  if (fLastErrorCode != CAEN_DGTZ_Success) { 
    std::cout << "[CANNOT OPEN DIGITIZER]" << std::endl;
    exit(1);
  }
  std::cout << "[CONNECTION ESTABLISHED]" << std::endl;

  // ======================= MAIN LOOP  =====================
  // Load the Correction Tables from the Digitizer flash
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];

  fLastErrorCode = CAEN_DGTZ_GetCorrectionTables(fHandle, CAEN_DGTZ_DRS4_5GHz, (void*)X742Tables);
  if( fLastErrorCode != CAEN_DGTZ_Success) {
    std::cout << "Error while retrieving tables... exit 1" << std::endl;
    return 1;
  }
  SaveCorrectionTables( 0x3, X742Tables);

  // ======================= CLOSE =====================
  fLastErrorCode = CAEN_DGTZ_CloseDigitizer(fHandle); 
  std::cout << "DAQ closed. Goodbye!" << std::endl;
  return fLastErrorCode;
}

int SaveCorrectionTables( uint32_t groupMask, CAEN_DGTZ_DRS4Correction_t *tables) {
  char fnStr[MAX_BASE_INPUT_FILE_LENGTH + 1];
  int ch,i,j, gr;
  FILE *outputfile;
  
  for(gr = 0; gr < MAX_X742_GROUP_SIZE; gr++) {
    CAEN_DGTZ_DRS4Correction_t *tb;
    
    if(!((groupMask>>gr)&0x1))
      continue;
    tb = &tables[gr];

    // cell offset
    sprintf(fnStr, "%s_gr%d_cell.txt", "x742_calib/Tables", gr);
    printf("Saving correction table cell values to %s\n", fnStr);
    if((outputfile = fopen(fnStr, "w")) == NULL)
      return -2;
    for(ch=0; ch<MAX_X742_CHANNEL_SIZE; ch++) {
      for(i=0; i!=1024; ++i) {
	fprintf(outputfile, "%d\t%d\t%d\n", ch, i, tb->cell[ch][i]);
      }
    }
    fclose(outputfile);

    // nsample offset
    sprintf(fnStr, "%s_gr%d_nsample.txt", "x742_calib/Tables", gr);
    printf("Saving correction table nsamples values to %s\n", fnStr);
    if((outputfile = fopen(fnStr, "w")) == NULL)
      return -3;
    for(ch=0; ch<MAX_X742_CHANNEL_SIZE; ch++) {
      for(i=0; i!=1024; ++i) {
	fprintf(outputfile, "%d\t%d\t%d\n", ch, i, tb->nsample[ch][i]);
      }
    }
    fclose(outputfile);
    
    // time offset
    sprintf(fnStr, "%s_gr%d_time.txt", "x742_calib/Tables", gr);
    printf("Saving correction table time values to %s\n", fnStr);
    if((outputfile = fopen(fnStr, "w")) == NULL)
      return -4;
    for(i=0; i!=1024; ++i) {
      fprintf(outputfile, "%d\t%09.3f\n", i, tb->time[i]);
    }
    fclose(outputfile);
  }
  return 0;
}
