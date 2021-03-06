/* ATSHA204 Library Simple Example
 by: Jim Lindblom
 SparkFun Electronics
 date: November 8, 2012
 
 This code shows how to wake up and verify that an SHA204 is
 connected and operational. And how to obtain an SHA204's unique serial
 number, and send it a MAC challenge.
 
 The ATSHA204's SDA pin can be connected to any of the Arduino's digital pins.
 When constructing your atsha204Class, pass the constructor the pin you want to use.
 In this example we'll attach SDA to pin 7.
 
 The ATSHA204 can be powered between 3.3V and 5V.
 */
#include <sha204_library.h>

#define SHA204_KEY_CHILD 10
#define SHA204_KEY_PARENT 13
#define MAC_MODE_CHALLENGE              ((uint8_t) 0x00)

const int sha204Pin = 7;

atsha204Class sha204(sha204Pin);
atsha204Class sha204_1(9);

void setup()
{
  Serial.begin(9600);
  Serial.println("Sending a Wakup Command. Response should be:\r\n4 11 33 43:");
  Serial.println("Response is:");
  wakeupExample();
  Serial.println();
  Serial.println("Asking the SHA204's serial number. Response should be:");
  Serial.println("1 23 x x x x x x x EE");
  Serial.println("Response is:");
  serialNumberExample();
  Serial.println();
  Serial.println("Sending a MAC Challenge. Response should be:");
  Serial.println("23 6 67 0 4F 28 4D 6E 98 62 4 F4 60 A3 E8 75 8A 59 85 A6 79 96 C4 8A 88 46 43 4E B3 DB 58 A4 FB E5 73");
  Serial.println("Response is:");
  macChallengeExample();
}

void loop()
{
}

byte wakeupExample()
{
  uint8_t response[SHA204_RSP_SIZE_MIN];
  byte returnValue;

  returnValue = sha204.sha204c_wakeup(&response[0]);
  for (int i=0; i<SHA204_RSP_SIZE_MIN; i++)
  {
    Serial.print(response[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  memset(response, 0, sizeof(response));
  returnValue = sha204_1.sha204c_wakeup(&response[0]);
  for (int i=0; i<SHA204_RSP_SIZE_MIN; i++)
  {
    Serial.print(response[i], HEX);
    Serial.print(" ");
  }
  Serial.println();


  return returnValue;
}

byte serialNumberExample()
{
  uint8_t serialNumber[9];
  byte returnValue;

  returnValue = sha204.getSerialNumber(serialNumber);
  for (int i=0; i<9; i++)
  {
    Serial.print(serialNumber[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); 

  memset(serialNumber, 0, sizeof(serialNumber));
  returnValue = sha204_1.getSerialNumber(serialNumber);
  for (int i=0; i<9; i++)
  {
    Serial.print(serialNumber[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); 

  return returnValue;
}

byte macChallengeExample()
{
  uint8_t command[MAC_COUNT_LONG];
  //uint8_t response[35];
  uint8_t response_status[SHA204_RSP_SIZE_MIN];
//  uint8_t randomnumber[35];
  uint8_t response_random[RANDOM_RSP_SIZE];
  
  memset(response_random, 0, sizeof(response_random));
  uint8_t *randomnumber = &response_random[SHA204_BUFFER_POS_DATA];

  uint8_t command_derive_key[GENDIG_OTHER_DATA_SIZE];
  uint8_t command_mac[CHECKMAC_CLIENT_COMMAND_SIZE];
  // MAC response buffer
  uint8_t response_mac[SHA204_RSP_SIZE_MAX];
  
  //check mac
  uint8_t checkmac_other_data[CHECKMAC_OTHER_DATA_SIZE];

  //serial number
  uint8_t serial_numb[9];
  
//uint8_t ret_code = sha204.sha204e_configure_key();
  //ret_code = sha204_1.sha204e_configure_diversify_key();
  //Serial.println(ret_code);
  /*for (int i=0; i<SHA204_RSP_SIZE_MAX; i++)
  {
    Serial.print(response[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
    */
   //Read Config Zone
   uint8_t config_data[88];
   Serial.println("Config Zone: Host");  
   sha204.sha204e_read_config_zone(config_data);
   for (int i=0; i<88; i++)
   {
   Serial.print(config_data[i], HEX);
   Serial.print(' ');
   }
   Serial.println();

  Serial.println("Config Zone: Client");  
   sha204_1.sha204e_read_config_zone(config_data);
   for (int i=0; i<88; i++)
   {
   Serial.print(config_data[i], HEX);
   Serial.print(' ');
   }
   Serial.println();
   
   uint8_t derive_key_command[] = {0x1C, 0x04, 0x0A, 0x00};
   //---------------------------------------------------------------------//
   //---------------- Configure the derive key -------------------------- //
   // ------------------------------------------------------------------- //
   
   //-------------------------SERIAL NUMBER--------------------------------//
   //------------get the serial number and pad it with zeros ------------- //
   
   
  sha204_1.getSerialNumber(serial_numb);
  
  //uint8_t *temp = (uint8_t *)realloc(serial_numb, 32*sizeof(uint8_t));
  uint8_t temp[NONCE_NUMIN_SIZE_PASSTHROUGH];
  
  memset(temp, 0x65 , sizeof(temp));
  for (int i=0; i<9; i++)
  {
    temp[i] = serial_numb[i];
  }
  
  Serial.println("Serial Number:");
  for (int i=0; i<32; i++)
  {
    Serial.print(temp[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); 
  
   
   // ----------------------- DIVERSIFIED KEY -----------------------------//
   // ---------- create a diversified key on the client device ----------- //
  
  //  Put padded serial number into TempKey (fixed Nonce).
  uint8_t ret_code_t = sha204_1.sha204m_nonce(command, response_status, NONCE_MODE_PASSTHROUGH, temp);
  if (ret_code_t != SHA204_SUCCESS) {
  	//sha204p_sleep();
  	return ret_code_t;
  }
  	
  //  Send DeriveKey command.
  ret_code_t = sha204_1.sha204m_derive_key(command, response_status, DERIVE_KEY_RANDOM_FLAG, SHA204_KEY_CHILD, NULL);
  
  //----------------------------------------------------------------------// 
  // Lock Config Zone : Danger make sure you configure everything before--// 
  // uint8_t ret_code_lock =  sha204.sha204e_lock_config_zone();----------//
  //----------------------------------------------------------------------// 

  // --------------------------------------------------------------------- //
  // Generate Random Number ---------------------------------------------- //
  // generate a random number on the host -------------------------------- //
  // --------------------------------------------------------------------- //
  memset(command, 0, sizeof(command));
  uint8_t ret_code_random_number = sha204.sha204m_random(command, response_random, RANDOM_NO_SEED_UPDATE);

  // output
  Serial.println("Generating Random Number");
  for (int i=0; i<NONCE_NUMIN_SIZE_PASSTHROUGH; i++)
  {
    Serial.print(randomnumber[i], HEX);
    Serial.print(' ');
    //randomnumber[i-1] = response[i];
  }
  Serial.println();
  
  
  //-----------------------------------------------------------------//
  // generate nonce on the client ---------------------------------- //
  // generate the nonce -------------------------------------------- //
  //-----------------------------------------------------------------//
 /* memset(command, 0, sizeof(command));
  memset(response_status, 0, sizeof(response_status));
  uint8_t ret_code_nonce = sha204_1.sha204m_nonce(command, response_status, NONCE_MODE_PASSTHROUGH, temp);

  //alternative way to send the nonce command
  //sha204m_execute(SHA204_NONCE, NONCE_MODE_SEED_UPDATE, 0, NONCE_NUMIN_SIZE,
  //    (uint8_t *) randomnumber, 0, NULL, 0, NULL, sizeof(command), &command[0], 
  //    sizeof(response), &response[0]);

  //output
  Serial.println("Performing nonce");
  for (int i=0; i<SHA204_RSP_SIZE_MIN; i++)
  {
      Serial.print(response_status[i], HEX);
      Serial.print(' ');
  }
  Serial.println();
  /*************nonce done*********************/

  // --------------------------------------------------------------- //
  // --------------- Derive Key ------------------------------------ //
  // --------------------------------------------------------------- // 
  /*
  memset(command, 0, sizeof(command));
  memset(response_status, 0, sizeof(response_status)); 
  uint8_t ret_code_der_key = sha204_1.sha204m_derive_key(command, response_status, DERIVE_KEY_RANDOM_FLAG, SHA204_KEY_CHILD, NULL);

  //output
  Serial.println("Derive Key");
  for (int i=0; i<SHA204_RSP_SIZE_MIN; i++)
  {
    Serial.print(response[i], HEX);
    Serial.print(' ');
  }
  Serial.println(); 

  // copy opcode and param which will be used later
  memcpy(command_derive_key, &command[SHA204_OPCODE_IDX], sizeof(command_derive_key));


  /****************************MAC Command and prep********************************/
  // Send Nonce command in preparation for MAC command. 
  /*
  memset(command, 0, sizeof(command));
  memset(response, 0, sizeof(response)); 
  ret_code_nonce = sha204_1.sha204m_nonce(command, response, NONCE_MODE_PASSTHROUGH, randomnumber);
  if (ret_code_nonce != SHA204_SUCCESS) {
    //sha204e_sleep();
    return ret_code_nonce;
  }

  //output
  Serial.println("Nonce Calculated");
  for (int i=0; i<SHA204_RSP_SIZE_MAX; i++)
  {
    Serial.print(response[i], HEX);
    Serial.print(' ');
  }
  Serial.println();

  // Send MAC command.
  // MAC = sha256(child key[32], TempKey[32] = random, MAC command[4], 0[11], sn8[1], 0[4], sn0_1[2], 0[2])
  // mode: first 32 bytes data slot (= child key), second 32 bytes TempKey (= random), TempKey.SourceFlag = Input
  memset(command, 0, sizeof(command));
  memset(response_mac, 0, sizeof(response_mac)); 
  uint8_t ret_code_mac = sha204_1.sha204m_mac(command, response_mac, MAC_MODE_BLOCK2_TEMPKEY | MAC_MODE_SOURCE_FLAG_MATCH, SHA204_KEY_CHILD, NULL);
  if (ret_code_mac != SHA204_SUCCESS) {
    return ret_code_mac;
  }
  
  //output
  //Serial.println("MAC");
   Serial.println("MAC");
  for (int i=SHA204_BUFFER_POS_DATA; i<SHA204_RSP_SIZE_MAX; i++)
  {
    Serial.print(response_mac[i], HEX);
    Serial.print(' ');
  }
  Serial.println(); 

  // Save op-code and parameters to be used in the CheckMac command for the host.
  memcpy(command_mac, &command[SHA204_OPCODE_IDX], sizeof(command_mac));
  */
  
  // ---------------------------------------------------------------- //
  // --------------------- Generate Digest -------------------------- //
  // ---------------------------------------------------------------- //
  memset(command, 0, sizeof(command));
  memset(response_status, 0, sizeof(response_status)); 
  uint8_t ret_code_nonce = sha204.sha204m_nonce(command, response_status, NONCE_MODE_PASSTHROUGH, temp);
  if (ret_code_nonce != SHA204_SUCCESS) {
    //sha204p_sleep();
    return ret_code_nonce;
  }

  memset(command, 0, sizeof(command));
  memset(response_status, 0, sizeof(response_status));
  uint8_t ret_code_gen_dig = sha204.sha204m_gen_dig(command, response_status, GENDIG_ZONE_DATA, SHA204_KEY_PARENT, derive_key_command);
  if (ret_code_gen_dig != SHA204_SUCCESS) {
    //sha204p_sleep();
    return ret_code_gen_dig;
  }
  
  // -------------------------------------------------------------- //
  // ------------------- MAC -------------------------------------- //
  // ----- issue a mac command on the client ---------------------- //
  // -------------------------------------------------------------- //
  
  memset(command, 0, sizeof(command));
  memset(response_mac, 0, sizeof(response_mac));
  uint8_t ret_code_mac = sha204_1.sha204m_mac(command, response_mac, MAC_MODE_CHALLENGE, SHA204_KEY_CHILD, randomnumber);
  if (ret_code_mac != SHA204_SUCCESS) {
  	//sha204e_sleep();
  	return ret_code_mac;
  }
  
   //output
  Serial.println("MAC");
  for (int i=SHA204_BUFFER_POS_DATA; i<SHA204_RSP_SIZE_MAX; i++)
  {
    Serial.print(response_mac[i], HEX);
    Serial.print(' ');
  }
  Serial.println(); 
  
  /*
  memset(command, 0, sizeof(command));
  memset(response_mac, 0, sizeof(response_mac));
   ret_code_mac = sha204.sha204m_mac(command, response_mac, MAC_MODE_CHALLENGE, SHA204_KEY_CHILD, randomnumber);
  if (ret_code_mac != SHA204_SUCCESS) {
  	//sha204e_sleep();
  	return ret_code_mac;
  }
  
   //output
  Serial.println("MAC Host");
  for (int i=SHA204_BUFFER_POS_DATA; i<SHA204_RSP_SIZE_MAX; i++)
  {
    Serial.print(response_mac[i], HEX);
    Serial.print(' ');
  }
  Serial.println(); 
  */
  // -------------------------------------------------------------- //
  // ------------------- Check MAC -------------------------------- //
  // -------------------------------------------------------------- //
   Serial.println("MAC");
  for (int i=0; i<SHA204_RSP_SIZE_MIN; i++)
  {
    Serial.print(response_status[i], HEX);
    Serial.print(' ');
  }
  Serial.println(); 

 // memset(command, 0, sizeof(command));
  memset(response_status, 0, sizeof(response_status));
  memset(checkmac_other_data, 0, sizeof(checkmac_other_data));
  memcpy(checkmac_other_data, &command[SHA204_OPCODE_IDX], CHECKMAC_CLIENT_COMMAND_SIZE);
  
  
  Serial.println(ret_code_mac);
  
  Serial.println("checkmac_other_data");
  for (int i=0; i<CHECKMAC_OTHER_DATA_SIZE; i++)
  {
    Serial.print(checkmac_other_data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  
  ret_code_mac = sha204.sha204m_check_mac(command, response_status, CHECKMAC_MODE_BLOCK1_TEMPKEY | CHECKMAC_MODE_SOURCE_FLAG_MATCH, 
				        0, randomnumber, &response_mac[SHA204_BUFFER_POS_DATA], checkmac_other_data);


  ///ret_code_mac = response_status[SHA204_BUFFER_POS_STATUS];  

  Serial.println(ret_code_mac);
  
  Serial.println("MAC");
  for (int i=0; i<SHA204_RSP_SIZE_MIN; i++)
  {
    Serial.print(response_status[i], HEX);
    Serial.print(' ');
  }
  Serial.println(); 

 // ret_code = sha204e_check_response_status(ret_code, response_status);

  /*
  for (int i=0; i<SHA204_RSP_SIZE_MAX; i++)
   {
   Serial.print(response[i], HEX);
   Serial.print(' ');
   }
   Serial.println();
   //not using this because it creates a small random number for some reason (need to
   //investigate that)
   //sha204.sha204m_execute(SHA204_RANDOM, RANDOM_SEED_UPDATE, 0, 0, 
   //   NULL, 0, NULL, 0, NULL, sizeof(command), &command[0], 
   //  32*sizeof(uint8_t), &response[0]);
   
   
   // generate the gendig
   
   
   // get the mac
   uint8_t ret_code = sha204.sha204m_execute(SHA204_MAC, 0x01, 0x0000, 0, 
   NULL, 0, NULL, 0, NULL, sizeof(command), &command[0], 
   sizeof(response), &response[0]);
   
   //
   Serial.print(ret_code);
   Serial.print(' ');
   for (int i=0; i<SHA204_RSP_SIZE_MAX; i++)
   {
   Serial.print(response[i], HEX);
   Serial.print(' ');
   }
   Serial.println("Data zone");
   
   
   //uint8_t returnCode1 = sha204.sha204e_configure_derive_key();
   //try a read
   uint8_t readCommand[READ_COUNT];
   uint8_t readResponse[READ_32_RSP_SIZE];	
   
   memset(readResponse, 0, sizeof(readResponse));
   uint8_t returnCode = sha204.sha204m_read(readCommand, readResponse, SHA204_ZONE_COUNT_FLAG | SHA204_ZONE_DATA , 1);
   Serial.println();
   for (int i=0; i<READ_32_RSP_SIZE; i++)
   {
   Serial.print(readResponse[i], HEX);
   Serial.print(' ');
   }
   Serial.println();
   
   uint8_t config_data[88];
   sha204.sha204e_read_config_zone(config_data);
   for (int i=0; i<88; i++)
   {
   Serial.print(config_data[i], HEX);
   Serial.print(' ');
   }
   Serial.println();
   
  /*
   uint8_t derivecommand[96] = { 0x1C, 0x4, 0x1 , 0x0 , 0x80 , 0x4F , 0x4D , 0x12 , 0xA3 , 0x4C , 0x2B , 0xDF , 0x8C , 0x70 , 0x30 , 0x23 , 0x96 , 0xB1 , 0x4A , 0x7C , 0x3D , 0x1B , 0x47 , 0xC7 , 0x2 , 0x15 , 0x0 , 0x0 , 0x0 , 0x80 , 0x2,
   0x1C, 0x04, 0x01, 0x00, 
   0xEE,
   0x01, 0x23, 
   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
   0x1, 0x23 , 0x39 , 0x66 , 0xAB , 0x5B , 0xA5 , 0xCE , 0xEE, 
   0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05};  
   */
  //uint8_t
 

  delay(1000);
  return 0;
}



/* read from bytes 0->3 of config zone 
 //uint8_t returnCode = sha204.sha204m_read(readCommand, readResponse, SHA204_ZONE_COUNT_FLAG | SHA204_ZONE_DATA , 0x0140);
 //Serial.println();
 //Serial.println(returnCode);
 // for (int i=0; i<READ_32_RSP_SIZE; i++)
 // {
 //   Serial.print(readResponse[i], HEX);
 //   Serial.print(' ');
 // }
 // Serial.println();
 
 uint8_t data_load[SHA204_ZONE_ACCESS_32];
 
 // Write client configuration.
 memcpy(data_load, &readResponse[SHA204_BUFFER_POS_DATA], sizeof(data_load));
 
 for (int i=0; i<SHA204_ZONE_ACCESS_32; i++)
 {
 Serial.print(data_load[i], HEX);
 Serial.print(' ');
 }
 Serial.println();
 
 */

/*
 // data_load[0] = 0x8F;
 
 data_load[1] = 0x8F;
 data_load[2] = 0x8F;
 data_load[3] = 0x8F;
 */
/*
  //write the config
 uint8_t writeCommand[WRITE_COUNT_LONG];
 uint8_t writeResponse[WRITE_RSP_SIZE];
 
 const uint8_t write_field[4] = {0x8F, 0x8F, 0x9F, 0x8F};
 
 //get the mac
 //uint8_t ret_code_write = sha204.sha204m_execute(SHA204_WRITE, 0x00, 20, 2,
 //  (uint8_t *)write_field, 0, NULL, 0, NULL, sizeof(writeCommand), &writeCommand[0], 
 //  sizeof(writeResponse), &writeResponse[0]);
 
 uint8_t ret_code_write = sha204.sha204m_write(writeCommand, writeResponse,  SHA204_ZONE_CONFIG, 24, (uint8_t *)write_field, NULL);
 Serial.println();  
 Serial.println(ret_code_write);
 Serial.println();
 for (int i=0; i<WRITE_RSP_SIZE; i++)
 {
 Serial.print(writeResponse[i], HEX);
 Serial.print(' ');
 }
 Serial.println();
 */
/* read from bytes 0->3 of config zone */
// returnCode = sha204.sha204m_read(readCommand, readResponse, SHA204_ZONE_COUNT_FLAG | SHA204_ZONE_CONFIG , 20);
// Serial.println();
//  for (int i=0; i<READ_4_RSP_SIZE; i++)
// {
//   Serial.print(readResponse[i], HEX);
//   Serial.print(' ');
// }
// Serial.println();



