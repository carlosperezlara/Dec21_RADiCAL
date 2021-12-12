#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>

#include "CAENDigitizer.h"

int fHandle;
CAEN_DGTZ_ErrorCode fLastErrorCode;


int setup();
void print_info();

//==========================================================================================
int main(int argc, char **argv) {
  int nevts = 1000;
  bool keep_saving_while_in_spill = false;

  if(argc>1) {
    std::cout << "testbeam mode engaged" << std::endl;
    nevts = atoi(argv[1]);
    keep_saving_while_in_spill = true;
  }
  
  // ======================= INIT =====================
  std::cout << std::endl;
  std::cout << "********************************************" << std::endl;
  std::cout << "********************************************" << std::endl;
  std::cout << "*   Digitizer aim " << nevts << " events   *" << std::endl;
  std::cout << "********************************************" << std::endl;
  std::cout << "********************************************" << std::endl;
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
  setup();
  print_info();

  // ======================= MAIN LOOP  =====================
  CAEN_DGTZ_EventInfo_t eventInfo;
  char *EventPtr;

  CAEN_DGTZ_X742_EVENT_t *Event742;
  char *buffer = NULL;
  uint32_t size;
  fLastErrorCode = CAEN_DGTZ_AllocateEvent(fHandle, (void**)&Event742);
  fLastErrorCode = CAEN_DGTZ_MallocReadoutBuffer(fHandle,&buffer,&size);
  //----- open fstream
  std::ofstream fout;
  fout.open("output.dat", std::fstream::out | std::fstream::binary | std::fstream::trunc); 
  //----- open communication channel
  fLastErrorCode = CAEN_DGTZ_SWStartAcquisition(fHandle);
  if (fLastErrorCode != CAEN_DGTZ_Success) {
    std::cerr << "Could not start acquisition: " << fLastErrorCode << std::endl;
    exit(1);
  }
  //----- run statistics
  int nRecEvents = 0;
  int nTimesNoReading = 0;
  int nTimesAtFull = 0;
  int nIterations = 0;
  bool InSpill = false;
  auto eventStart = std::chrono::steady_clock::now();
  //----- steering loop
  for(;(nRecEvents<nevts) || InSpill;++nIterations) {
    if((nIterations%1000)==0)
      std::cout << "Number of events in disk: " << nRecEvents << std::endl;
    uint32_t reg; 
    fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x8104, &reg); //AcquisitionStatus
    int running = (reg>>2) & 0b1; // 3rd bit => 1=running | 0=stopped
    int ready   = (reg>>3) & 0b1; // 4th bit => 1=ev ready | 0=no event
    int buffull = (reg>>4) & 0x1; // 5th bit => 1=full | 0=not full
    if((buffull==1)) {
      nTimesAtFull++;
    }
    if(ready==1) {
      fLastErrorCode = CAEN_DGTZ_ReadData(fHandle,
					  CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
					  buffer,
					  &size);
      if (fLastErrorCode)
	std::cout << "Error " << fLastErrorCode << std::endl;
      fout.write(buffer, size);
      nRecEvents++;
      eventStart = std::chrono::steady_clock::now();
    } else {
      nTimesNoReading++;
    }
    auto eventStop = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = eventStop-eventStart;
    if (elapsed.count() > 1) {
      InSpill = keep_saving_while_in_spill;
    }
    else {
      InSpill = false;
    }

  }
  std::cout << "=================" << std::endl;
  std::cout << "===== STATS =====" << std::endl;
  std::cout << "Number of events acquired: " << nRecEvents << std::endl;
  std::cout << "Number of cycles with no reading: " << nTimesNoReading << std::endl;
  std::cout << "Number of cycles with buffer full: " << nTimesAtFull << std::endl;
  std::cout << "Number of total read attempts: " <<  nIterations << std::endl;
  if(nIterations != (nRecEvents + nTimesNoReading)) {
    std::cout << "  ** WARNING: check stats  ** " << std::endl;
  } else {
    std::cout << "  ** not a single missed event at this cpp program speed ** " << std::endl;
  }
  fout.close();
  fLastErrorCode = CAEN_DGTZ_SWStopAcquisition(fHandle); 
  if (fLastErrorCode != CAEN_DGTZ_Success)
    std::cout << "Could not stop acquisition: " << fLastErrorCode << std::endl;

  // ======================= CLOSE =====================
  fLastErrorCode = CAEN_DGTZ_CloseDigitizer(fHandle); 
  std::cout << "DAQ closed. Goodbye!" << std::endl;
  return fLastErrorCode;
}

//==========================================================================================
//==========================================================================================
//==========================================================================================
int setup() {
  uint32_t reg;
  fLastErrorCode = CAEN_DGTZ_Reset(fHandle); // Reset everything 
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); // need to think of a better approach

  CAEN_DGTZ_SetRecordLength(fHandle,1024); // Set the record length to 1024 samples (5 GS / s)

  uint32_t gemask = 0x3; 
  CAEN_DGTZ_SetGroupEnableMask(fHandle, gemask); // groups "11"
  gemask = 0xffff;
  CAEN_DGTZ_SetChannelEnableMask(fHandle, gemask); // channels "1111 1111 1111 1111"

  //CAEN_DGTZ_SetGroupFastTriggerThreshold(fHandle, 0, 26214); //Set Trigger Threshold to ~500 mV
  //CAEN_DGTZ_SetGroupFastTriggerThreshold(fHandle, 1, 26214); //Set Trigger Threshold to ~500 mV
  CAEN_DGTZ_SetMaxNumAggregatesBLT(fHandle, 1); // 1000? // 1 is fast but is there a risk of lose?

  //CAEN_DGTZ_DRS4Frequency_t freq = CAEN_DGTZ_DRS4_1GHz;
  //CAEN_DGTZ_DRS4Frequency_t freq = CAEN_DGTZ_DRS4_2_5GHz;
  CAEN_DGTZ_DRS4Frequency_t freq = CAEN_DGTZ_DRS4_5GHz;
  CAEN_DGTZ_SetDRS4SamplingFrequency(fHandle, freq); // 0: 5Gs   2: 1Gs  3: 0.75 GHz


  
  
  /// TR0 Digitization
  CAEN_DGTZ_SetFastTriggerDigitizing(fHandle, CAEN_DGTZ_ENABLE);    // Enables digitization of TR0
  


  
  
  ///////////////////////////////////////////////////////////////////////////////////////////////
  // D C    O F F S E T
  ///////////////////


  // Channel DC Offset
  //reg = (0xf<<17)|(0x6ABC); // 0xf means channels 1111 will be affected
  //fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x1098, reg); // GROUP 0
  //reg = (0xf<<17)|(0x6ABC); // 0xf means channels 1111 will be affected
  //fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x1198, reg); // GROUP 1

  // TR DC Offset
  // approx mV = -(DEC - 33540)*0.0466
  //reg = 0x8304; // 0 mV
  //reg = 0x6000; // 420 mV
  //reg = 0x6ABC; // 300 mV
  //reg = 0x7000; // 220 mV
  //reg = 0x7FFF; // 0 mV
  //reg = 0x9000; // -150 mV
  //reg = 0xA000; // -350 mV
  //fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x10DC, reg);
  



  
  ///////////////////////////////////////////////////////////////////////////////////////////////
  // T R I G G E R
  ///////////////////

  //CAEN_DGTZ_SetTriggerPolarity(fHandle, 0, CAEN_DGTZ_TriggerOnRisingEdge);
  CAEN_DGTZ_SetTriggerPolarity(fHandle, 0, CAEN_DGTZ_TriggerOnFallingEdge);
  
  
  /// SOFTWARE TRIGGER
  //CAEN_DGTZ_SetAcquisitionMode(fHandle,CAEN_DGTZ_SW_CONTROLLED); // START/STOP SOFTWARE CONTROLLED
  //CAEN_DGTZ_SetSWTriggerMode(fHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);  // SOFTWARE TRIGGER?

  /// TR0 TRIGGER
  CAEN_DGTZ_SetFastTriggerMode(fHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);// Trigger with TR0
  // default -150mV
  // approx mV = (DEC - 25448)*0.0329
  //reg = 0x4000; // -300 mV
  //reg = 0x5000; // -150 mV
  reg = 0x6ABC; // 50 mV
  //reg = 0x7000; // 100 mV
  //reg = 0x8000; // 250 mV
  //reg = 0x9000; // 380 mV
  fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x10D4, reg); // Threshold
  

  // TIN Trigger
  // NIM:   "1" = -1V to -0.8V     "0" = 0V
  // TTL:   "1" = 1.5V to 5.0V     "0" = 0V to 0.7V
  //CAEN_DGTZ_SetIOLevel(fHandle, CAEN_DGTZ_IOLevel_NIM); // NIM trigger
  //CAEN_DGTZ_SetIOLevel(fHandle, CAEN_DGTZ_IOLevel_TTL); // TTL trigger
  //fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x811C, &reg);
  //uint32_t reg = (3<<18)|(1<<16);
  //fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x811C, reg);

  // TRIGGER ON CHANNEL COINCIDENCE (OR)
  // register  0x10A8 0x11A8 (p19) 0x1080 0x1180 (p13)
  //reg = 0x0f; // channels 00001111 (first four channels)
  //fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x10A8, reg); // group0
  //fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x11A8, reg); // group1
  

  // Post Trigger (time delay)
  CAEN_DGTZ_SetPostTriggerSize(fHandle,30); // percentage of record length

  //DIGITIZER IS NOW OPEN
  return fLastErrorCode;
}
//==========================================================================================
void print_info() {
  std::cout << "=== COMMUNICATION ====" << std::endl;

  CAEN_DGTZ_BoardInfo_t fBoardInfo;
  fLastErrorCode = CAEN_DGTZ_GetInfo(fHandle, &fBoardInfo);
  if (fLastErrorCode != CAEN_DGTZ_Success) {
    exit(1);
  }
  std::cout <<   "Model: " << fBoardInfo.ModelName << std::endl;
  std::cout <<   "Channels: " << fBoardInfo.Channels << std::endl;
  std::cout <<   "ADC_NBits: " << fBoardInfo.ADC_NBits << std::endl;
  std::cout <<   "ROC FPGA: " << fBoardInfo.ROC_FirmwareRel << std::endl;
  std::cout <<   "AMC FPGA: " << fBoardInfo.AMC_FirmwareRel << std::endl;
  std::cout <<   "Board Family: " << fBoardInfo.FamilyCode << std::endl; 

  std::cout << "=== TRIGGER ====" << std::endl;

  CAEN_DGTZ_TriggerMode_t tmode; 
  fLastErrorCode = CAEN_DGTZ_GetSWTriggerMode(fHandle, &tmode);
  std::cout << "Software Trigger Mode: "; 
  
  fLastErrorCode = CAEN_DGTZ_GetExtTriggerInputMode(fHandle, &tmode);
  std::cout << "External Trigger Mode: "; 

  CAEN_DGTZ_RunSyncMode_t runsyncmode;
  fLastErrorCode = CAEN_DGTZ_GetRunSynchronizationMode(fHandle, &runsyncmode);
  std::cout << "Run synchronization mode: ";
  std::cout << runsyncmode << std::endl;
  
  CAEN_DGTZ_IOLevel_t iolevel;
  fLastErrorCode = CAEN_DGTZ_GetIOLevel(fHandle, &iolevel);
  std::cout << "IO Level: ";
  if(iolevel==CAEN_DGTZ_IOLevel_NIM)
    std::cout << "[NIM] ";
  else
    std::cout << "[TTL] ";
  std::cout << std::endl;
  
  CAEN_DGTZ_TriggerPolarity_t polarity;
  fLastErrorCode = CAEN_DGTZ_GetTriggerPolarity(fHandle, 0, &polarity);
  std::cout << "Trigger Polarity: ";
  if(polarity==CAEN_DGTZ_TriggerOnRisingEdge)
    std::cout << "[On rising edge] ";
  else
    std::cout << "[On falling edge] ";
  std::cout << std::endl;

  uint32_t TriggerThreshold = 0;
  fLastErrorCode = CAEN_DGTZ_GetGroupFastTriggerThreshold(fHandle, 0, &TriggerThreshold);
  std::cout << "Trig Thresh Group 0:" << TriggerThreshold << std::endl; 
  fLastErrorCode = CAEN_DGTZ_GetGroupFastTriggerThreshold(fHandle, 1, &TriggerThreshold);
  std::cout << "Trig Thresh Group 1:" << TriggerThreshold << std::endl;

  uint32_t FastTrigDCOffset = 0;
  fLastErrorCode = CAEN_DGTZ_GetGroupFastTriggerDCOffset(fHandle, 0, &FastTrigDCOffset); 
  std::cout << "Fast Trig DC Offset:" << FastTrigDCOffset << std::endl; 
  fLastErrorCode = CAEN_DGTZ_GetGroupFastTriggerDCOffset(fHandle, 1, &FastTrigDCOffset); 
  std::cout << "Fast Trig DC Offset Group 2:" << FastTrigDCOffset << std::endl; 

  CAEN_DGTZ_EnaDis_t ft_digtz_enabled;
  fLastErrorCode = CAEN_DGTZ_GetFastTriggerDigitizing(fHandle, &ft_digtz_enabled);
  std::cout << "Fast trigger digitizing: ";
  if (ft_digtz_enabled==CAEN_DGTZ_ENABLE)
    std::cout << "[Enabled]" << std::endl;
  else
    std::cout << "[Disabled]" << std::endl;

  CAEN_DGTZ_TriggerMode_t fast_trig_mode;
  fLastErrorCode = CAEN_DGTZ_GetFastTriggerMode(fHandle, &fast_trig_mode);
  std::cout << "Fast trigger: ";
  if (fast_trig_mode==CAEN_DGTZ_TRGMODE_ACQ_ONLY) {
    std::cout << "[Acq only]" << std::endl; 
  }
  else {
    std::cout << "[Disabled]" << std::endl;
  }

  CAEN_DGTZ_DRS4Frequency_t freq;
  fLastErrorCode = CAEN_DGTZ_GetDRS4SamplingFrequency(fHandle, &freq);
  std::cerr << "DRS4 Sampling Frequency: ";

  std::cout << "=== ACQUISITION ====" << std::endl;
  uint32_t mask; 
  fLastErrorCode = CAEN_DGTZ_GetGroupEnableMask(fHandle, &mask); 
  std::cout << "Group Enable Mask: " << std::hex << "0x" <<  mask << std::dec << std::endl; 
  fLastErrorCode = CAEN_DGTZ_GetChannelEnableMask(fHandle, &mask); 
  std::cout << "Channel Enable Mask: " << std::hex << "0x" <<  mask << std::dec << std::endl; 
  uint32_t sz; 
  fLastErrorCode = CAEN_DGTZ_GetRecordLength(fHandle, &sz); 
  std::cout << "Record Length: " << sz << std::endl; 
  fLastErrorCode = CAEN_DGTZ_GetPostTriggerSize(fHandle, &sz); 
  std::cout << "Post Trigger Size: " << sz << std::endl; 
  CAEN_DGTZ_AcqMode_t mode;
  fLastErrorCode = CAEN_DGTZ_GetAcquisitionMode(fHandle,&mode);
  std::cout << "Acquisition Mode: ";
  if(mode==CAEN_DGTZ_SW_CONTROLLED)
    std::cout << "[Software controlled] ";
  else if(mode==CAEN_DGTZ_S_IN_CONTROLLED)
    std::cout << "[S_IN CONTROLLED] ";
  else
    std::cout << "[First Trigger Controlled] ";
  std::cout << std::endl;
  
  std::cout << "====================" << std::endl;

  uint32_t reg;
  fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x810C, &reg); // Global Trigger Mask [31:0]
  std::cout << "Global Trigger Mask => Reg 0x810C:" << std::endl;
  std::cout << " ex_trig: " << ((reg >> 30) & 0b1) << std::endl; // bit 30
  std::cout << " sw_trig: " << ((reg >> 31) & 0b1) << std::endl; // bit 31

  fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x811C, &reg); // Front Pannel IO Control [31:0]
  std::cout << "Front Pannel IO Control => Reg 0x811C:" << std::hex << reg << std::dec << std::endl;
  std::cout << " bit16: " << ((reg >> 16) & 0b1) << std::endl; // bit 18
  std::cout << " bit17: " << ((reg >> 17) & 0b1) << std::endl; // bit 18
  std::cout << " bit18: " << ((reg >> 18) & 0b1) << std::endl; // bit 18
  std::cout << " bit19: " << ((reg >> 19) & 0b1) << std::endl; // bit 19
  std::cout << " bit20 (busy out): " << ((reg >> 20) & 0b1) << std::endl; // bit 20

  std::cout << "CH-wise configuration: " << std::endl;
  for(int ich=0; ich!=8; ++ich) {
    reg = ich;
    fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x10A4, reg);
    fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x1080, &reg);
    std::cout << " channel " << ich << " => thr " << reg;
    fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x1098, &reg);
    std::cout << "  adc offset " << reg << " || ";
    //
    fLastErrorCode = CAEN_DGTZ_WriteRegister(fHandle, 0x11A4, reg);
    fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x1180, &reg);
    std::cout << " channel " << 8+ich << " => thr " << reg;
    fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x1198, &reg);
    std::cout << "  adc offset " << reg << std::endl;

  }
  std::cout << "TR-wise configuration: " << std::endl;
  fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x10D4, &reg);
  std::cout << " tr0  => thr " << reg;
  fLastErrorCode = CAEN_DGTZ_ReadRegister(fHandle, 0x10DC, &reg);
  std::cout << "  adc offset " << reg << std::endl;

  uint32_t numEvents;
  
  fLastErrorCode = CAEN_DGTZ_GetMaxNumAggregatesBLT(fHandle, &numEvents);
  if (fLastErrorCode != CAEN_DGTZ_Success) {
    std::cerr << "Could not read Max Aggregate Events  " << fLastErrorCode << std::endl;
  }
  std::cout << "MaxAggregatesBLT:" << numEvents << std::endl; 

}
