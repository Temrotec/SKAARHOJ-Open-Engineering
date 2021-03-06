/*****************
   Basis control for the SKAARHOJ C50 series devices
   This example is programmed for ATEM 1M/E versions

   This example also uses several custom libraries which you must install first.
   Search for "#include" in this file to find the libraries. Then download the libraries from http://skaarhoj.com/wiki/index.php/Libraries_for_Arduino

   Works with ethernet-enabled arduino devices (Arduino Ethernet or a model with Ethernet shield)
   Make sure to configure IP and addresses first using the sketch "ConfigEthernetAddresses"

   - kasper
*/



// Including libraries:
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EEPROM.h>      // For storing IP numbers
#include "WebServer.h"  // For web interface
#include <SkaarhojPgmspace.h>
#include <Streaming.h>
#include <SkaarhojTools.h>

SkaarhojTools sTools(1);    // 0=No runtime serial logging, 1=Moderate runtime serial logging, 2=more verbose... etc.

// Include ATEM library and make an instance:
#include <ATEMbase.h>
#include <ATEMext.h>
ATEMext AtemSwitcher;

#include <MemoryFree.h>

// Configure the IP addresses and MAC address with the sketch "ConfigEthernetAddresses":
static uint8_t default_ip[] = {     // IP for Configuration Mode (192.168.10.99)
  192, 168, 10, 99
};
uint8_t ip[4];        // Will hold the Arduino IP address
uint8_t atem_ip[4];  // Will hold the ATEM IP address
uint8_t mac[6];    // Will hold the Arduino Ethernet shield/board MAC address (loaded from EEPROM memory, set with ConfigEthernetAddresses example sketch)


// All related to library "SkaarhojBI8":
#include "Wire.h"
#include "MCP23017.h"
#include "PCA9685.h"
#include "SkaarhojBI8.h"
#include "SkaarhojUtils.h"

SkaarhojBI8 inputSelect1;
SkaarhojBI8 inputSelect2;
SkaarhojBI8 cmdSelect;
SkaarhojUtils utils;

int greenLED = 15;
int redLED = 13;
int blueLED = 14;







/*************************************************************


                       Webserver


 **********************************************************/

#define PREFIX ""
WebServer webserver(PREFIX, 80);

void logoCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  /* this data was taken from a PNG file that was converted to a C data structure
     by running it through the directfb-csource application.
     (Alternatively by PHPSH script "HeaderGraphicsWebInterfaceInUnitsPNG8.phpsh")
  */
  P(logoData) = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00,
    0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x02, 0xa2,
    0x00, 0x00, 0x00, 0x2b, 0x08, 0x03, 0x00, 0x00, 0x00, 0x94,
    0xff, 0x1a, 0xf8, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58,
    0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00,
    0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x49, 0x6d, 0x61, 0x67,
    0x65, 0x52, 0x65, 0x61, 0x64, 0x79, 0x71, 0xc9, 0x65, 0x3c,
    0x00, 0x00, 0x00, 0x30, 0x50, 0x4c, 0x54, 0x45, 0xe6, 0xe6,
    0xe6, 0x0d, 0x5c, 0xaf, 0x0c, 0x61, 0xbd, 0x57, 0x89, 0xcc,
    0xd7, 0xe2, 0xf2, 0xfa, 0xfa, 0xfa, 0x85, 0xa8, 0xd9, 0xab,
    0xc2, 0xe3, 0xc7, 0xd6, 0xec, 0x0b, 0x51, 0x9d, 0x36, 0x76,
    0xc4, 0x23, 0x68, 0xb9, 0x0a, 0x65, 0xc5, 0x0a, 0x65, 0xc6,
    0x0d, 0x5e, 0xb5, 0xff, 0xff, 0xff, 0xe6, 0xb3, 0x8a, 0x8c,
    0x00, 0x00, 0x07, 0x1e, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda,
    0xec, 0x5d, 0x0d, 0x7b, 0xa3, 0x20, 0x0c, 0x56, 0xa8, 0xc0,
    0xd4, 0xe2, 0xff, 0xff, 0xb7, 0x67, 0x05, 0x42, 0x12, 0x82,
    0xda, 0xae, 0xf7, 0xcc, 0x6e, 0xc4, 0x75, 0x53, 0xf2, 0x8d,
    0xaf, 0xaf, 0xca, 0xed, 0xee, 0xba, 0x25, 0xca, 0xe0, 0xc6,
    0xf9, 0xde, 0xa4, 0xc9, 0x45, 0x44, 0x8d, 0x6e, 0x08, 0xc8,
    0xec, 0x00, 0xa1, 0xaa, 0x4d, 0x4b, 0x93, 0x2b, 0xc9, 0x1c,
    0x31, 0x1a, 0x21, 0x3a, 0x0c, 0x63, 0x9b, 0x93, 0x26, 0x17,
    0xe3, 0x51, 0x06, 0xd1, 0x46, 0xa2, 0x4d, 0xae, 0x86, 0xd1,
    0x61, 0xc0, 0x37, 0xfa, 0xae, 0xcd, 0x48, 0x93, 0xab, 0x49,
    0x87, 0x21, 0x3a, 0x34, 0x88, 0x36, 0xb9, 0x9c, 0x10, 0x16,
    0x1d, 0x86, 0x36, 0x21, 0x4d, 0x2e, 0xce, 0xa2, 0xf3, 0xbc,
    0xbe, 0x43, 0xa5, 0x77, 0xa9, 0xf8, 0xb9, 0xa3, 0x65, 0xa8,
    0xa0, 0x9f, 0x83, 0x6a, 0xfd, 0x31, 0x0b, 0x23, 0xd9, 0x75,
    0x4e, 0xdb, 0x9c, 0x62, 0x04, 0xbb, 0xcd, 0x67, 0xfb, 0x1e,
    0x13, 0x91, 0x48, 0x31, 0x54, 0xb0, 0x03, 0x1b, 0xe4, 0x99,
    0x7d, 0xe6, 0x6c, 0x1f, 0x7d, 0x90, 0xd1, 0x7c, 0xa7, 0xda,
    0x58, 0xfd, 0x7c, 0xc7, 0x3d, 0xcd, 0x2c, 0x4b, 0x2a, 0x1c,
    0x77, 0x9e, 0x5b, 0x40, 0xe3, 0x78, 0x52, 0x50, 0x40, 0xc8,
    0x87, 0xcd, 0x52, 0xc5, 0x77, 0x16, 0x36, 0x55, 0x19, 0x1d,
    0x67, 0xe8, 0xfb, 0x8e, 0x6b, 0x7c, 0x29, 0xda, 0xcb, 0x15,
    0x3c, 0xe9, 0xfa, 0xe6, 0x66, 0xd1, 0x11, 0xc0, 0xa3, 0x23,
    0x8b, 0x4e, 0xdd, 0xfc, 0xa7, 0xe4, 0x7e, 0x46, 0x73, 0xff,
    0xd9, 0x4a, 0x7e, 0x7e, 0x2e, 0x7e, 0x3a, 0x15, 0x7f, 0x16,
    0x9d, 0x9b, 0x34, 0xb9, 0x96, 0x9c, 0x64, 0x51, 0x63, 0x6e,
    0xa5, 0x98, 0xf5, 0x82, 0xd0, 0xdb, 0x9e, 0x4e, 0x76, 0x3a,
    0xab, 0xc2, 0x05, 0x33, 0xd2, 0xe3, 0x2c, 0xa5, 0x42, 0xdf,
    0xa4, 0x1c, 0x63, 0x56, 0x59, 0xe4, 0xee, 0x68, 0xde, 0xd5,
    0x24, 0x97, 0x68, 0x8c, 0x11, 0x2f, 0x5e, 0x53, 0x44, 0x69,
    0xf2, 0x01, 0xb7, 0xba, 0x53, 0x2c, 0x2a, 0xe1, 0xf3, 0x01,
    0x90, 0x7b, 0x02, 0x4f, 0x80, 0x8a, 0x02, 0x94, 0xe9, 0x3b,
    0x77, 0x1d, 0xe7, 0x68, 0x52, 0x51, 0xd4, 0x20, 0x1a, 0x40,
    0x7c, 0x04, 0xd1, 0x89, 0x55, 0xe8, 0x6e, 0x46, 0xe3, 0x06,
    0x14, 0x4a, 0xf9, 0x46, 0x88, 0xaa, 0xff, 0x7e, 0x7e, 0xd4,
    0x5f, 0x02, 0xa3, 0x3a, 0xcb, 0xa2, 0x8a, 0x6d, 0xb3, 0xb2,
    0xb7, 0x0a, 0x78, 0x14, 0x86, 0x28, 0x1c, 0xdc, 0x9c, 0x86,
    0x30, 0x23, 0x02, 0x9a, 0x0a, 0x35, 0x94, 0x8a, 0x38, 0xa4,
    0xeb, 0x69, 0x12, 0x44, 0x73, 0x55, 0x09, 0xa2, 0xdb, 0x91,
    0xa9, 0xb8, 0xa9, 0x14, 0x3a, 0xfc, 0xb4, 0x45, 0x94, 0xdc,
    0x63, 0x65, 0xdb, 0xd3, 0xed, 0xaa, 0x9e, 0x51, 0x9e, 0xb1,
    0x7b, 0x25, 0xd5, 0x37, 0x2a, 0x78, 0x36, 0xdd, 0x7b, 0x9b,
    0x25, 0x87, 0xfc, 0x59, 0x34, 0xe9, 0xb1, 0x69, 0x82, 0x8e,
    0x1d, 0x85, 0x18, 0x09, 0xa2, 0x0a, 0xf6, 0x6f, 0x6e, 0xcc,
    0xc9, 0x32, 0x78, 0x98, 0xb3, 0xa0, 0xe0, 0x38, 0x34, 0x16,
    0x2c, 0xf6, 0x21, 0x6a, 0xc2, 0x81, 0x33, 0x5b, 0x28, 0x70,
    0x33, 0x38, 0x5f, 0x2e, 0xc6, 0xd2, 0xee, 0x4e, 0x6c, 0x6f,
    0x52, 0x3d, 0x99, 0xf5, 0xfb, 0xd1, 0x5e, 0xaf, 0xa0, 0x3c,
    0xf8, 0x6f, 0xa9, 0x8e, 0x37, 0xce, 0xa2, 0x21, 0x3e, 0xc9,
    0xe4, 0x6e, 0xe9, 0x06, 0x29, 0x14, 0xa2, 0x33, 0x54, 0x34,
    0x45, 0x32, 0xe1, 0xca, 0x88, 0x2f, 0xa8, 0x9d, 0x2b, 0x72,
    0x28, 0x8b, 0x2e, 0x01, 0x9b, 0x2c, 0x34, 0xb5, 0x5c, 0x37,
    0x04, 0x51, 0x93, 0x58, 0x13, 0xae, 0xa9, 0x58, 0xb1, 0x56,
    0xa4, 0x97, 0x04, 0x51, 0x85, 0x67, 0x90, 0xce, 0x66, 0x7d,
    0x9e, 0x77, 0xed, 0xe6, 0xea, 0x29, 0x55, 0x95, 0x93, 0xac,
    0x76, 0xd0, 0x50, 0x1f, 0xdf, 0x8f, 0x26, 0x17, 0x7f, 0x5c,
    0x41, 0xe9, 0x52, 0xda, 0xef, 0x17, 0xff, 0xce, 0x66, 0x8b,
    0xf9, 0xbe, 0x0b, 0x2c, 0xaa, 0x48, 0x5c, 0x80, 0xa8, 0x51,
    0x4a, 0x38, 0x07, 0x3a, 0x81, 0x01, 0x10, 0x3a, 0xdd, 0x55,
    0x4c, 0xa0, 0x22, 0x7a, 0x0c, 0x05, 0x8c, 0x02, 0x58, 0x61,
    0x05, 0x62, 0x51, 0x45, 0x79, 0xf5, 0xe6, 0x54, 0x46, 0x6f,
    0xca, 0x1d, 0x21, 0xba, 0xee, 0x4d, 0x0e, 0x41, 0x3b, 0x7c,
    0x8d, 0x37, 0x14, 0x6a, 0xcb, 0x58, 0xb0, 0x28, 0x9a, 0x09,
    0x05, 0x7e, 0xe4, 0x4b, 0x65, 0x0d, 0x0a, 0x2d, 0x1c, 0xce,
    0x6c, 0xbe, 0x58, 0xcc, 0x72, 0xe2, 0x71, 0xdc, 0xe2, 0x33,
    0x2b, 0xa9, 0x92, 0x33, 0xd1, 0xd2, 0x2e, 0xde, 0x29, 0xf0,
    0x26, 0x55, 0xc0, 0xf7, 0x59, 0x6f, 0x47, 0xc5, 0x3f, 0x93,
    0x6a, 0xbf, 0xd9, 0xb9, 0xec, 0xb4, 0x64, 0x51, 0x55, 0x4a,
    0xe6, 0x32, 0x41, 0x99, 0x21, 0x9a, 0x10, 0xaa, 0x90, 0xd9,
    0x18, 0x41, 0xeb, 0x22, 0x7b, 0xed, 0x2b, 0x12, 0x0e, 0xb9,
    0xd9, 0x7a, 0xa7, 0x2f, 0x55, 0x0e, 0xd0, 0x1d, 0x80, 0xe7,
    0xa6, 0x0c, 0x2a, 0x78, 0xbe, 0x98, 0x68, 0xad, 0xb6, 0x88,
    0xd2, 0xe4, 0xfa, 0x52, 0x3e, 0x8b, 0x96, 0x62, 0xd0, 0xb3,
    0x68, 0x15, 0xa2, 0x3a, 0x53, 0x2d, 0xf7, 0xd4, 0x33, 0xb0,
    0xe5, 0xae, 0xa2, 0x0e, 0xd1, 0x69, 0x07, 0xa2, 0x11, 0x78,
    0x86, 0xd4, 0x16, 0x1d, 0xf5, 0x2c, 0x94, 0xd3, 0x20, 0xfa,
    0x61, 0x72, 0xcc, 0xa2, 0x4a, 0x7c, 0xa3, 0xb7, 0x66, 0xc4,
    0x10, 0x15, 0x11, 0x3a, 0x26, 0x2e, 0x1b, 0xe3, 0xfd, 0x7a,
    0x5f, 0x51, 0xe2, 0xd0, 0xf2, 0x1b, 0xbd, 0x04, 0x51, 0x27,
    0x64, 0x16, 0x07, 0xe7, 0x06, 0xd1, 0xdf, 0xca, 0xa2, 0x4a,
    0x5e, 0xd4, 0x59, 0xef, 0xd3, 0x80, 0x2b, 0x73, 0xe3, 0x3c,
    0x99, 0xdd, 0x0c, 0x60, 0x0d, 0x48, 0x4d, 0x56, 0x70, 0x1c,
    0xc6, 0x17, 0xf5, 0x87, 0xba, 0xb6, 0x1e, 0x95, 0x21, 0xca,
    0x08, 0x33, 0x42, 0xd4, 0xd3, 0x7a, 0xba, 0x06, 0xd1, 0x5f,
    0xca, 0xa2, 0x4a, 0x4d, 0x46, 0x64, 0xd2, 0xcc, 0xa2, 0xae,
    0x4e, 0xa2, 0x2b, 0xdb, 0x7a, 0x4d, 0x68, 0xb4, 0xa2, 0xa8,
    0xaf, 0x8b, 0xaa, 0x63, 0x88, 0x4a, 0x2c, 0xea, 0x0c, 0x6b,
    0xa2, 0xb1, 0xe8, 0x47, 0x42, 0x94, 0xb2, 0x68, 0xcd, 0xcc,
    0x63, 0x3e, 0xd2, 0xb0, 0xf2, 0xe8, 0x13, 0x44, 0x47, 0x01,
    0x28, 0x18, 0x11, 0x0e, 0xb3, 0x6c, 0xa9, 0x58, 0x03, 0xf9,
    0x0a, 0x0e, 0xdd, 0x83, 0x0a, 0xab, 0x10, 0xf5, 0x40, 0xc4,
    0xd2, 0x8d, 0x9e, 0xb3, 0x7a, 0x63, 0xd1, 0x5f, 0xc0, 0xa2,
    0xeb, 0x29, 0xf7, 0xb0, 0xa5, 0x9d, 0x00, 0x4f, 0xaf, 0x60,
    0xc8, 0xa7, 0x97, 0xfc, 0x44, 0x83, 0xda, 0x8f, 0x69, 0xe9,
    0xc7, 0xab, 0x68, 0x34, 0x02, 0x8a, 0x32, 0xc6, 0xa6, 0x87,
    0x6e, 0x04, 0x06, 0x04, 0xc5, 0xe3, 0x7d, 0xdc, 0x4b, 0x38,
    0x34, 0xe6, 0x91, 0xdd, 0xc3, 0x33, 0x00, 0x14, 0x01, 0x18,
    0x8c, 0xb5, 0x18, 0x52, 0xf4, 0x94, 0x52, 0xa4, 0x6a, 0xb6,
    0x26, 0xe2, 0xa5, 0x81, 0x1b, 0x83, 0x7d, 0xd6, 0x72, 0x3e,
    0x0c, 0x3f, 0x49, 0xf7, 0x58, 0x81, 0xac, 0xb1, 0x0b, 0x8e,
    0x21, 0xc4, 0x22, 0xf1, 0x7d, 0x51, 0x42, 0x9c, 0x74, 0xea,
    0x57, 0x89, 0xe6, 0x85, 0x13, 0xf6, 0x8a, 0x4f, 0x32, 0x78,
    0xb2, 0xf8, 0x77, 0x36, 0xeb, 0xc5, 0xea, 0x4a, 0x16, 0x65,
    0x73, 0x5e, 0x04, 0xc5, 0x88, 0xbb, 0xc1, 0x8d, 0x1e, 0x80,
    0xb7, 0x62, 0xd4, 0x13, 0x18, 0x53, 0xc4, 0x3d, 0x74, 0xb2,
    0x22, 0x3f, 0x8b, 0x6e, 0x19, 0x52, 0xb8, 0x7e, 0xab, 0x00,
    0xab, 0xc2, 0x16, 0x21, 0xba, 0xea, 0xe2, 0xa2, 0x13, 0x29,
    0x3c, 0x8e, 0x85, 0x03, 0x68, 0xde, 0x42, 0x85, 0xb9, 0x2b,
    0x61, 0x47, 0xda, 0x3c, 0x36, 0x42, 0xc7, 0x82, 0xe5, 0xe9,
    0x61, 0x79, 0x68, 0xb7, 0x8c, 0xe7, 0xa2, 0x7d, 0xc7, 0xe7,
    0x5b, 0xe9, 0xde, 0xd4, 0x6c, 0xfc, 0xc1, 0x9f, 0x45, 0xb7,
    0xe9, 0xf7, 0xf9, 0xcc, 0x86, 0x4f, 0x1c, 0x4d, 0x43, 0x89,
    0xf0, 0x9c, 0xef, 0xd3, 0xa2, 0x93, 0x4f, 0xac, 0xb9, 0xbe,
    0x44, 0x6d, 0x36, 0xe2, 0xbd, 0xd9, 0x4d, 0x87, 0x0a, 0x1b,
    0x61, 0x90, 0xd6, 0x59, 0x7d, 0x3e, 0xb0, 0x3e, 0x57, 0xe7,
    0x20, 0x6f, 0x58, 0x5c, 0xed, 0x34, 0x14, 0xab, 0x7c, 0x9f,
    0x1e, 0x1f, 0x08, 0xa4, 0x26, 0x3c, 0x8a, 0x9a, 0x42, 0x3b,
    0xb9, 0x59, 0x55, 0x34, 0x5e, 0xda, 0xe0, 0x19, 0x52, 0x85,
    0x92, 0x84, 0x57, 0x2c, 0xa2, 0xaa, 0x79, 0xa8, 0x32, 0xdc,
    0xd9, 0x68, 0xd8, 0xee, 0xa4, 0x8f, 0x92, 0x2b, 0xc0, 0xf5,
    0x1d, 0x16, 0xff, 0x6a, 0xb3, 0xd2, 0xd4, 0xd1, 0x70, 0x71,
    0x97, 0xb3, 0x28, 0x3b, 0x13, 0x54, 0x60, 0xc8, 0x00, 0x7e,
    0x22, 0x44, 0x7b, 0x84, 0xdb, 0x69, 0x33, 0x91, 0x7f, 0xf7,
    0xc4, 0xec, 0x28, 0x00, 0x87, 0x41, 0x60, 0x1d, 0xcb, 0x73,
    0xd5, 0x63, 0xc4, 0xe5, 0xbc, 0x69, 0x39, 0x21, 0x6b, 0x6d,
    0xfc, 0x5d, 0x01, 0x5a, 0xbd, 0x05, 0x3b, 0xd2, 0xa5, 0x92,
    0xda, 0x15, 0x31, 0xcc, 0xa7, 0xba, 0x98, 0x1d, 0x55, 0xce,
    0x94, 0x92, 0x6c, 0x4b, 0x00, 0x4a, 0x57, 0x81, 0x90, 0x6d,
    0x37, 0x9a, 0x70, 0xb1, 0x1d, 0xfb, 0x88, 0xa5, 0x3c, 0x59,
    0xfc, 0x6b, 0xcd, 0x2a, 0x5f, 0xe3, 0x09, 0x36, 0x05, 0xec,
    0x6f, 0x80, 0xfa, 0x33, 0x32, 0x1a, 0x78, 0x5b, 0x49, 0x2c,
    0x8a, 0x41, 0x65, 0xc7, 0xbc, 0x9f, 0x51, 0x32, 0x92, 0x3f,
    0x28, 0x12, 0x14, 0x3d, 0x83, 0xe8, 0xa4, 0x01, 0xbc, 0x5c,
    0x95, 0x21, 0x8a, 0xb0, 0xd7, 0x59, 0x33, 0x86, 0xea, 0x1c,
    0xc5, 0x6c, 0xbc, 0xaa, 0xec, 0xad, 0x08, 0xd2, 0xe4, 0x13,
    0xa4, 0xce, 0xa2, 0x59, 0xaa, 0xab, 0x41, 0x9e, 0xb0, 0x28,
    0xf0, 0x99, 0x1b, 0x7b, 0x78, 0x89, 0x41, 0x18, 0x09, 0x23,
    0x55, 0x85, 0x2f, 0x70, 0x98, 0x7e, 0x3f, 0xa4, 0xf7, 0xfb,
    0x10, 0xf5, 0xd2, 0xba, 0xad, 0xd3, 0x22, 0x95, 0xdb, 0xa9,
    0x9d, 0xf3, 0x4f, 0x83, 0xe8, 0x72, 0xcc, 0xa2, 0xba, 0xb6,
    0xe2, 0x93, 0x21, 0xba, 0xd9, 0xf5, 0x3d, 0x60, 0x54, 0xe3,
    0x7b, 0x7e, 0x64, 0x45, 0x47, 0x1f, 0x3d, 0xb9, 0x62, 0x2a,
    0x71, 0x98, 0x30, 0x7a, 0x04, 0x51, 0xaf, 0x0b, 0x2c, 0x9a,
    0x49, 0x7a, 0xda, 0x70, 0xa6, 0x9d, 0xf1, 0x3f, 0xc3, 0xa2,
    0x36, 0x60, 0x80, 0xb2, 0xa8, 0x07, 0x8c, 0x8a, 0xb7, 0x5a,
    0x86, 0x6e, 0xa6, 0xb0, 0x25, 0x0e, 0x01, 0xa3, 0x63, 0x0d,
    0xa2, 0x29, 0xef, 0x7a, 0x75, 0xe0, 0xbf, 0x18, 0xa2, 0xd7,
    0x81, 0x02, 0xa2, 0xeb, 0xb0, 0xef, 0xdb, 0x19, 0xff, 0x74,
    0x16, 0x95, 0x4e, 0x61, 0xef, 0xe5, 0xd1, 0x88, 0x8d, 0xed,
    0x3b, 0x19, 0xed, 0x45, 0x97, 0x68, 0x24, 0xa8, 0x82, 0xa6,
    0xef, 0x45, 0xfc, 0xf4, 0x0f, 0x45, 0xe9, 0xd4, 0x7b, 0x92,
    0xd7, 0x07, 0x9b, 0x14, 0x48, 0x46, 0x62, 0xc3, 0xe7, 0x27,
    0x0a, 0x7f, 0xa3, 0x8f, 0x10, 0xea, 0x85, 0x8d, 0xed, 0x14,
    0x56, 0xdc, 0xb3, 0x74, 0x49, 0x9e, 0xa5, 0x42, 0x74, 0xad,
    0x55, 0x71, 0x30, 0x5a, 0xef, 0x60, 0x57, 0xc1, 0x5b, 0x94,
    0xac, 0x4b, 0xed, 0x5e, 0xd3, 0x72, 0x80, 0x9a, 0x07, 0x9f,
    0xa5, 0xe3, 0xea, 0x05, 0x8f, 0xe3, 0x86, 0x4f, 0x55, 0xf0,
    0x62, 0xba, 0x37, 0x35, 0xcb, 0x5d, 0xf8, 0xb3, 0x68, 0xa0,
    0xac, 0x00, 0xa4, 0xf8, 0x05, 0xc0, 0xea, 0x81, 0xd1, 0x7a,
    0x34, 0xee, 0xc1, 0x10, 0x9b, 0x64, 0x53, 0x50, 0x79, 0x7a,
    0x48, 0xdd, 0x7a, 0xec, 0xe9, 0x79, 0x78, 0x48, 0xeb, 0x69,
    0x9c, 0x22, 0x37, 0xfb, 0x06, 0xd5, 0xa2, 0xa2, 0x41, 0x23,
    0x54, 0x81, 0x83, 0x7a, 0xa2, 0xf5, 0x3d, 0x6d, 0xcd, 0xe7,
    0xc0, 0x24, 0x7b, 0x4e, 0x83, 0x2d, 0x89, 0x0f, 0x9f, 0x87,
    0x5c, 0x10, 0xcb, 0xeb, 0x9f, 0x8d, 0x26, 0xd4, 0xf2, 0x5a,
    0x05, 0xcf, 0xa4, 0x7b, 0x77, 0xb3, 0x6c, 0xaa, 0x8b, 0x67,
    0x51, 0x00, 0x44, 0x93, 0x26, 0x17, 0x11, 0xb6, 0x2e, 0xda,
    0x26, 0xa4, 0xc9, 0xd5, 0xa4, 0x60, 0xd1, 0x26, 0x4d, 0x2e,
    0x06, 0x51, 0xfa, 0x4f, 0xe0, 0x7e, 0xb5, 0x19, 0x69, 0x72,
    0x6d, 0x16, 0x9d, 0xda, 0x8c, 0x34, 0xb9, 0x96, 0x4c, 0x8c,
    0x45, 0xed, 0x57, 0xe3, 0xd1, 0x26, 0x17, 0x92, 0xaf, 0x2f,
    0x3b, 0xd0, 0xff, 0x31, 0xa4, 0xb3, 0xd3, 0x57, 0x93, 0x26,
    0x97, 0x91, 0xc9, 0x76, 0xf4, 0x8d, 0x7e, 0x18, 0xba, 0x26,
    0x4d, 0x2e, 0x25, 0x03, 0x65, 0xd1, 0x15, 0xa3, 0x0d, 0xa4,
    0x4d, 0x2e, 0x05, 0x50, 0xf6, 0x5f, 0x83, 0x3d, 0x40, 0xba,
    0xe1, 0xf4, 0xa1, 0xc9, 0x9f, 0x25, 0x1c, 0x2f, 0x69, 0x7c,
    0x49, 0x7a, 0xf4, 0x73, 0xc1, 0x7b, 0x5c, 0x03, 0x9f, 0x34,
    0xc2, 0x83, 0x0f, 0x52, 0xf0, 0xec, 0x20, 0x8e, 0x0c, 0xb2,
    0x0d, 0x3e, 0x58, 0xb0, 0xeb, 0x52, 0xf5, 0x5c, 0x68, 0x61,
    0xa1, 0x8a, 0x9d, 0x6e, 0x68, 0xc1, 0x03, 0xe9, 0x45, 0x98,
    0x9e, 0x45, 0xc8, 0xb9, 0x94, 0xbd, 0x2c, 0xbc, 0xf6, 0x85,
    0x54, 0xf4, 0x4c, 0xb4, 0xd7, 0x2a, 0x58, 0x4e, 0xba, 0x2e,
    0xc5, 0xdc, 0xbd, 0xad, 0xd9, 0x81, 0xfa, 0x2c, 0x09, 0xa0,
    0xcb, 0xf2, 0x4f, 0x80, 0x01, 0x00, 0x3d, 0x86, 0x58, 0xcf,
    0x39, 0xc2, 0xf5, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
  };

  if (type == WebServer::POST)
  {
    // ignore POST data
    server.httpFail();
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" but identify our data as a PNG file */
  server.httpSuccess("image/png");

  /* we don't output the body for a HEAD request */
  if (type == WebServer::GET)
  {
    server.writeP(logoData, sizeof(logoData));
  }
}

// commands are functions that get called by the webserver framework
// they can read any posted data from client, and they output to server
void webDefaultView(WebServer &server, WebServer::ConnectionType type)
{
  P(htmlHead) =
    "<html>"
    "<head>"
    "<title>SKAARHOJ Device Configuration</title>"
    "<style type=\"text/css\">"
    "BODY { font-family: sans-serif }"
    "H1 { font-size: 14pt; }"
    "P  { font-size: 10pt; }"
    "</style>"
    "</head>"
    "<body>"
    "<img src='logo.png'><br/>";

  int i;
  server.httpSuccess();
  server.printP(htmlHead);

  server << F("<div style='width:660px; margin-left:10px;'><form action='") << PREFIX << F("/form' method='post'>");

  // C100 Panel IP:
  server << F("<h1>SKAARHOJ Device IP Address:</h1><p>");
  for (i = 0; i <= 3; ++i)
  {
    server << F("<input type='text' name='IP") << i << F("' value='") << EEPROM.read(i + 2) << F("' id='IP") << i << F("' size='4'>");
    if (i < 3)  server << F(".");
  }
  server << F("<hr/>");

  // ATEM Switcher Panel IP:
  server << F("<h1>ATEM Switcher IP Address:</h1><p>");
  for (i = 0; i <= 3; ++i)
  {
    server << F("<input type='text' name='ATEM_IP") << i << F("' value='") << EEPROM.read(i + 2 + 4) << F("' id='ATEM_IP") << i << F("' size='4'>");
    if (i < 3)  server << F(".");
  }
  server << F("<hr/>");

  // Set first routing for INPUT buttons
  server << F("<h1>INPUT SELECT:</h1><table border=0 cellspacing=3><tr>");


  for (i = 1; i <= 10; ++i)
  {
    server << F("<td><p>Button ") << i << F(":</p>");
    server << F("<select name='Buttons1F1") << i << F(":'div style='width:78px;>");
    server << F("<option value='0'></option>");

    //server << F("<option value='") << 0 << F("'") << (EEPROM.read(400+i)==0?" selected='selected'":"") << "> \t" << F("</option>");

    // Preview bus inputs 1-20
    for (uint8_t j = 1; j <= 20; j++)  {
      server << F("<option value='") << j << F("'") << (EEPROM.read(400 + i) == j ? F(" selected='selected'") : F("")) << F(">Input ") << j << F("</option>");
    }

    // Other input sources
    server << F("<option value='") << 21 << F("'") << (EEPROM.read(400 + i) == 21 ? F(" selected='selected'") : F("")) << F(">Black") << F("</option>");
    server << F("<option value='") << 22 << F("'") << (EEPROM.read(400 + i) == 22 ? F(" selected='selected'") : F("")) << F(">Bars") << F("</option>");
    server << F("<option value='") << 23 << F("'") << (EEPROM.read(400 + i) == 23 ? F(" selected='selected'") : F("")) << F(">Color1") << F("</option>");
    server << F("<option value='") << 24 << F("'") << (EEPROM.read(400 + i) == 24 ? F(" selected='selected'") : F("")) << F(">Color2") << F("</option>");
    server << F("<option value='") << 25 << F("'") << (EEPROM.read(400 + i) == 25 ? F(" selected='selected'") : F("")) << F(">Media1") << F("</option>");
    server << F("<option value='") << 26 << F("'") << (EEPROM.read(400 + i) == 26 ? F(" selected='selected'") : F("")) << F(">Media1Key") << F("</option>");
    server << F("<option value='") << 27 << F("'") << (EEPROM.read(400 + i) == 27 ? F(" selected='selected'") : F("")) << F(">Media2") << F("</option>");
    server << F("<option value='") << 28 << F("'") << (EEPROM.read(400 + i) == 28 ? F(" selected='selected'") : F("")) << F(">Media2Key") << F("</option>");
    server << F("<option value='") << 29 << F("'") << (EEPROM.read(400 + i) == 29 ? F(" selected='selected'") : F("")) << F(">SuperSource") << F("</option>");
    server << F("<option value='") << 30 << F("'") << (EEPROM.read(400 + i) == 30 ? F(" selected='selected'") : F("")) << F(">CleanFeed1") << F("</option>");
    server << F("<option value='") << 31 << F("'") << (EEPROM.read(400 + i) == 31 ? F(" selected='selected'") : F("")) << F(">CleanFeed2") << F("</option>");
    server << F("<option value='") << 32 << F("'") << (EEPROM.read(400 + i) == 32 ? F(" selected='selected'") : F("")) << F(">Program") << F("</option>");
    server << F("<option value='") << 33 << F("'") << (EEPROM.read(400 + i) == 33 ? F(" selected='selected'") : F("")) << F(">Preview") << F("</option>");

    //server << F("<option value='") << 23 << F("'") << (EEPROM.read(400+i)==23?" selected='selected'":"") << ">Cut" << F("</option>");
    //server << F("<option value='") << 24 << F("'") << (EEPROM.read(400+i)==24?" selected='selected'":"") << ">Auto" << F("</option>");
    //server << F("<option value='") << 25 << F("'") << (EEPROM.read(400+i)==25?" selected='selected'":"") << ">FTB" << F("</option>");

    //server << F("<option value='") << 255 << F("'") << (EEPROM.read(414+i)==255?" selected='selected'":"") << ">Default" << F("</option>");

    server << F("</select></td>");
  }
  server << F("</tr></n>");

  // Set second routing for INPUT buttons
  for (i = 1; i <= 10; ++i)
  {
    server << F("<td></p>");
    server << F("<select name='Buttons1F2") << i << F(":'div style='width:78px;>");
    server << F("<option value='0'></option>");

    server << F("<option value='") << 0 << F("'") << (EEPROM.read(410 + i) == 0 ? F(" selected='selected'") : F("")) << F("> \t") << F("</option>");

    // USK 1-4 on/off
    server << F("<option value='") << 1 << F("'") << (EEPROM.read(410 + i) == 1 ? F(" selected='selected'") : F("")) << F(">USK1") << F("</option>");
    server << F("<option value='") << 2 << F("'") << (EEPROM.read(410 + i) == 2 ? F(" selected='selected'") : F("")) << F(">USK1On") << F("</option>");
    server << F("<option value='") << 3 << F("'") << (EEPROM.read(410 + i) == 3 ? F(" selected='selected'") : F("")) << F(">USK1Off") << F("</option>");
    server << F("<option value='") << 4 << F("'") << (EEPROM.read(410 + i) == 4 ? F(" selected='selected'") : F("")) << F(">USK2") << F("</option>");
    server << F("<option value='") << 5 << F("'") << (EEPROM.read(410 + i) == 5 ? F(" selected='selected'") : F("")) << F(">USK2On") << F("</option>");
    server << F("<option value='") << 6 << F("'") << (EEPROM.read(410 + i) == 6 ? F(" selected='selected'") : F("")) << F(">USK2Off") << F("</option>");
    server << F("<option value='") << 7 << F("'") << (EEPROM.read(410 + i) == 7 ? F(" selected='selected'") : F("")) << F(">USK3") << F("</option>");
    server << F("<option value='") << 8 << F("'") << (EEPROM.read(410 + i) == 8 ? F(" selected='selected'") : F("")) << F(">USK3On") << F("</option>");
    server << F("<option value='") << 9 << F("'") << (EEPROM.read(410 + i) == 9 ? F(" selected='selected'") : F("")) << F(">USK3Off") << F("</option>");
    server << F("<option value='") << 10 << F("'") << (EEPROM.read(410 + i) == 10 ? F(" selected='selected'") : F("")) << F(">USK4") << F("</option>");
    server << F("<option value='") << 11 << F("'") << (EEPROM.read(410 + i) == 11 ? F(" selected='selected'") : F("")) << F(">USK4On") << F("</option>");
    server << F("<option value='") << 12 << F("'") << (EEPROM.read(410 + i) == 12 ? F(" selected='selected'") : F("")) << F(">USK4Off") << F("</option>");

    // DSK 1-2 on/off
    server << F("<option value='") << 13 << F("'") << (EEPROM.read(410 + i) == 13 ? F(" selected='selected'") : F("")) << F(">DSK1") << F("</option>");
    server << F("<option value='") << 14 << F("'") << (EEPROM.read(410 + i) == 14 ? F(" selected='selected'") : F("")) << F(">DSK1On") << F("</option>");
    server << F("<option value='") << 15 << F("'") << (EEPROM.read(410 + i) == 15 ? F(" selected='selected'") : F("")) << F(">DSK1Off") << F("</option>");
    server << F("<option value='") << 16 << F("'") << (EEPROM.read(410 + i) == 16 ? F(" selected='selected'") : F("")) << F(">DSK1Auto") << F("</option>");
    server << F("<option value='") << 17 << F("'") << (EEPROM.read(410 + i) == 17 ? F(" selected='selected'") : F("")) << F(">DSK2") << F("</option>");
    server << F("<option value='") << 18 << F("'") << (EEPROM.read(410 + i) == 18 ? F(" selected='selected'") : F("")) << F(">DSK2On") << F("</option>");
    server << F("<option value='") << 19 << F("'") << (EEPROM.read(410 + i) == 19 ? F(" selected='selected'") : F("")) << F(">DSK2Off") << F("</option>");
    server << F("<option value='") << 20 << F("'") << (EEPROM.read(410 + i) == 20 ? F(" selected='selected'") : F("")) << F(">DSK2Auto") << F("</option>");

    // Others
    server << F("<option value='") << 21 << F("'") << (EEPROM.read(410 + i) == 21 ? F(" selected='selected'") : F("")) << F(">Keys Off") << F("</option>");
    //    server << F("<option value='") << 7 << F("'") << (EEPROM.read(408+i)==7?" selected='selected'":"") << ">Cut" << F("</option>");
    //   server << F("<option value='") << 8 << F("'") << (EEPROM.read(408+i)==8?" selected='selected'":"") << ">Auto" << F("</option>");
    //   server << F("<option value='") << 9 << F("'") << (EEPROM.read(408+i)==9?" selected='selected'":"") << ">FTB" << F("</option>");


    //    server << F("<option value='") << 11 << F("'") << (EEPROM.read(408+i)==11?" selected='selected'":"") << ">PiP" << F("</option>");
    //    server << F("<option value='") << 12 << F("'") << (EEPROM.read(408+i)==12?" selected='selected'":"") << ">VGA+PiP" << F("</option>");

    server << F("</select></td>");
  }
  server << F("</tr></table><hr/>");

  // Set routing for USER buttons
  server << F("<h1>USER BUTTONS:</h1><table border=0 cellspacing=3><tr>");

  // Set routing for USER buttons
  for (i = 1; i <= 5; ++i)
  {
    server << F("<td><p>Button ") << i << F(":</p>");
    server << F("<select name='Buttons2F1") << i << F(":'div style='width:78px;>");
    server << F("<option value='0'></option>");

    //server << F("<option value='") << 0 << F("'") << (EEPROM.read(416+i)==0?" selected='selected'":"") << "> \t" << F("</option>");

    // USK 1-4 on/off
    server << F("<option value='") << 1 << F("'") << (EEPROM.read(420 + i) == 1 ? F(" selected='selected'") : F("")) << F(">USK1") << F("</option>");
    server << F("<option value='") << 2 << F("'") << (EEPROM.read(420 + i) == 2 ? F(" selected='selected'") : F("")) << F(">USK1On") << F("</option>");
    server << F("<option value='") << 3 << F("'") << (EEPROM.read(420 + i) == 3 ? F(" selected='selected'") : F("")) << F(">USK1Off") << F("</option>");
    server << F("<option value='") << 4 << F("'") << (EEPROM.read(420 + i) == 4 ? F(" selected='selected'") : F("")) << F(">USK2") << F("</option>");
    server << F("<option value='") << 5 << F("'") << (EEPROM.read(420 + i) == 5 ? F(" selected='selected'") : F("")) << F(">USK2On") << F("</option>");
    server << F("<option value='") << 6 << F("'") << (EEPROM.read(420 + i) == 6 ? F(" selected='selected'") : F("")) << F(">USK2Off") << F("</option>");
    server << F("<option value='") << 7 << F("'") << (EEPROM.read(420 + i) == 7 ? F(" selected='selected'") : F("")) << F(">USK3") << F("</option>");
    server << F("<option value='") << 8 << F("'") << (EEPROM.read(420 + i) == 8 ? F(" selected='selected'") : F("")) << F(">USK3On") << F("</option>");
    server << F("<option value='") << 9 << F("'") << (EEPROM.read(420 + i) == 9 ? F(" selected='selected'") : F("")) << F(">USK3Off") << F("</option>");
    server << F("<option value='") << 10 << F("'") << (EEPROM.read(420 + i) == 10 ? F(" selected='selected'") : F("")) << F(">USK4") << F("</option>");
    server << F("<option value='") << 11 << F("'") << (EEPROM.read(420 + i) == 11 ? F(" selected='selected'") : F("")) << F(">USK4On") << F("</option>");
    server << F("<option value='") << 12 << F("'") << (EEPROM.read(420 + i) == 12 ? F(" selected='selected'") : F("")) << F(">USK4Off") << F("</option>");

    // DSK 1-2 on/off
    server << F("<option value='") << 13 << F("'") << (EEPROM.read(420 + i) == 13 ? F(" selected='selected'") : F("")) << F(">DSK1") << F("</option>");
    server << F("<option value='") << 14 << F("'") << (EEPROM.read(420 + i) == 14 ? F(" selected='selected'") : F("")) << F(">DSK1On") << F("</option>");
    server << F("<option value='") << 15 << F("'") << (EEPROM.read(420 + i) == 15 ? F(" selected='selected'") : F("")) << F(">DSK1Off") << F("</option>");
    server << F("<option value='") << 16 << F("'") << (EEPROM.read(420 + i) == 16 ? F(" selected='selected'") : F("")) << F(">DSK1Auto") << F("</option>");
    server << F("<option value='") << 17 << F("'") << (EEPROM.read(420 + i) == 17 ? F(" selected='selected'") : F("")) << F(">DSK2") << F("</option>");
    server << F("<option value='") << 18 << F("'") << (EEPROM.read(420 + i) == 18 ? F(" selected='selected'") : F("")) << F(">DSK2On") << F("</option>");
    server << F("<option value='") << 19 << F("'") << (EEPROM.read(420 + i) == 19 ? F(" selected='selected'") : F("")) << F(">DSK2Off") << F("</option>");
    server << F("<option value='") << 20 << F("'") << (EEPROM.read(420 + i) == 20 ? F(" selected='selected'") : F("")) << F(">DSK2Auto") << F("</option>");

    // Others
    server << F("<option value='") << 21 << F("'") << (EEPROM.read(420 + i) == 21 ? F(" selected='selected'") : F("")) << F(">Keys Off") << F("</option>");
    server << F("<option value='") << 22 << F("'") << (EEPROM.read(420 + i) == 22 ? F(" selected='selected'") : F("")) << F(">Cut") << F("</option>");
    server << F("<option value='") << 23 << F("'") << (EEPROM.read(420 + i) == 23 ? F(" selected='selected'") : F("")) << F(">Auto") << F("</option>");
    server << F("<option value='") << 24 << F("'") << (EEPROM.read(420 + i) == 24 ? F(" selected='selected'") : F("")) << F(">FTB") << F("</option>");


    server << F("<option value='") << 25 << F("'") << (EEPROM.read(420 + i) == 25 ? F(" selected='selected'") : F("")) << F(">PiP") << F("</option>");
    server << F("<option value='") << 26 << F("'") << (EEPROM.read(420 + i) == 26 ? F(" selected='selected'") : F("")) << F(">VGA+PiP") << F("</option>");

    server << F("</select></td>");
  }
  server << F("</tr></table><hr/>");
  ///////////////////////////////////////////////////



  // End form and page:
  server << F("<input type='submit' value='Submit'/></form></div>");
  server << F("<br><i>(Reset / Pull the power after submitting the form successfully)</i>");
  server << F("</body></html>");
}

void formCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      repeat = server.readPOSTparam(name, 16, value, 16);
      String Name = String(name);

      // C100 Panel IP:
      if (Name.startsWith("IP"))  {
        int addr = strtoul(name + 2, NULL, 10);
        int val = strtoul(value, NULL, 10);
        if (addr >= 0 && addr <= 3)  {
          EEPROM.write(addr + 2, val); // IP address stored in bytes 0-3
        }
      }

      // ATEM Switcher Panel IP:
      if (Name.startsWith("ATEM_IP"))  {
        int addr = strtoul(name + 7, NULL, 10);
        int val = strtoul(value, NULL, 10);
        if (addr >= 0 && addr <= 3)  {
          EEPROM.write(addr + 2 + 4, val); // IP address stored in bytes 4-7
        }
      }

      // routing
      if (Name.startsWith("Buttons1F1"))  {
        int inputNum = strtoul(name + 10, NULL, 10);
        int val = strtoul(value, NULL, 10);
        if (inputNum >= 1 && inputNum <= 10)  {
          EEPROM.write(400 + inputNum, val);
        }
      }
      if (Name.startsWith("Buttons1F2"))  {
        int inputNum = strtoul(name + 10, NULL, 10);
        int val = strtoul(value, NULL, 10);
        if (inputNum >= 1 && inputNum <= 10)  {
          EEPROM.write(410 + inputNum, val);
        }
      }
      if (Name.startsWith("Buttons2F1"))  {
        int inputNum = strtoul(name + 10, NULL, 10);
        int val = strtoul(value, NULL, 10);
        if (inputNum >= 1 && inputNum <= 5)  {
          EEPROM.write(420 + inputNum, val);
        }
      }


    }
    while (repeat);

    server.httpSeeOther(PREFIX "/form");
  }
  else
    webDefaultView(server, type);
}
void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  webDefaultView(server, type);
}





























/*************************************************************


                       MAIN PROGRAM CODE AHEAD


 **********************************************************/


bool isConfigMode;  // If set, the system will run the Web Configurator, not the normal program
int buttons1function1[10];
int buttons1function2[10];
int buttons2function[5];


void setup() {

  // Start the Ethernet, Serial (debugging) and UDP:
  Serial.begin(115200);
  Serial << F("\n- - - - - - - -\nSerial Started\n");

  // *********************************
  // Start up BI8 boards and I2C bus:
  // *********************************
  // Always initialize Wire before setting up the SkaarhojBI8 class!
  Wire.begin();

  // Set up the SkaarhojBI8 boards:
  inputSelect1.begin(0, false);
  inputSelect2.begin(1, false);
  cmdSelect.begin(2, false);



  // *********************************
  // Mode of Operation (Normal / Configuration)
  // *********************************
  // Determine web config mode

  digitalWrite(redLED, 1);   // To prevent brief light in LEDs upon boot
  digitalWrite(blueLED, 1);   // To prevent brief light in LEDs upon boot
  digitalWrite(greenLED, 1);   // To prevent brief light in LEDs upon boot
  pinMode(redLED, OUTPUT);    // Red Status LED, active low
  pinMode(blueLED, OUTPUT);    // Blue Status LED, active low
  pinMode(greenLED, OUTPUT);    // Green Status LED, active low

  delay(100);


  // Check CFG input:
  Serial << F("\nCheck if config button is held down after power-up/reset...\n");
  if (!digitalRead(18)) {   // Config mode, light up status LED in red and stop.
    digitalWrite(redLED, 1);   // Red
    digitalWrite(greenLED, 1);   // Green
    digitalWrite(blueLED, 0);   // Blue
    Serial << F(" => Config Mode ON: Status LED is blue.\n");
    isConfigMode = true;
  } else {
    Serial << F(" => Config Mode OFF\n");
  }
  delay(500);


  // *********************************
  // EEPROM memory:
  // *********************************
  // EEPROM MEMORY:
  // 0-1:  Initialization check
  // 2-5:  Arduino IP
  // 6-9:  ATEM IP
  // 10-16: Arduino MAC address (6+1 byte)
  // 17: Reboot counter


  // Setting the Arduino IP address:
  // *********************************
  // Setting up IP addresses, starting Ethernet
  // *********************************
  if (isConfigMode)  {
    // Setting the default ip address for configuration mode:
    Serial.println("Config mode");
    ip[0] = default_ip[0];
    ip[1] = default_ip[1];
    ip[2] = default_ip[2];
    ip[3] = default_ip[3];
  }
  else {
    Serial.println("Work mode");
    ip[0] = EEPROM.read(2);
    ip[1] = EEPROM.read(3);
    ip[2] = EEPROM.read(4);
    ip[3] = EEPROM.read(5);
  }

  // Setting the ATEM IP address:
  atem_ip[0] = EEPROM.read(6);
  atem_ip[1] = EEPROM.read(7);
  atem_ip[2] = EEPROM.read(8);
  atem_ip[3] = EEPROM.read(9);

  Serial << F("SKAARHOJ Device IP Address: ") << ip[0] << "." << ip[1] << "." << ip[2] << "." << ip[3] << "\n";
  Serial << F("ATEM Switcher IP Address: ") << atem_ip[0] << "." << atem_ip[1] << "." << atem_ip[2] << "." << atem_ip[3] << "\n";

  // Setting MAC address:
  mac[0] = EEPROM.read(10);
  mac[1] = EEPROM.read(11);
  mac[2] = EEPROM.read(12);
  mac[3] = EEPROM.read(13);
  mac[4] = EEPROM.read(14);
  mac[5] = EEPROM.read(15);
  char buffer[18];
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial << F("SKAARHOJ Device MAC address: ") << buffer << F(" - Checksum: ")
         << ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF) << "\n";
  if ((uint8_t)EEPROM.read(16) != ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF))  {
    Serial << F("MAC address not found in EEPROM memory!\n") <<
           F("Please load example sketch ConfigEthernetAddresses to set it.\n") <<
           F("The MAC address is found on the backside of your Ethernet Shield/Board\n (STOP)");
    while (true);
  }

  Ethernet.begin(mac, ip);

  delay(1000);

  // Sets the Bi-color LED to off = "no connection"
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, HIGH);
  digitalWrite(blueLED, HIGH);

  inputSelect1.setDefaultColor(0);  // Off by default
  inputSelect2.setDefaultColor(0);  // Off by default
  cmdSelect.setDefaultColor(0);  // Off by default

  // *********************************
  // Final Setup based on mode
  // *********************************
  if (isConfigMode)  {

    // Red by default:
    inputSelect1.setDefaultColor(2);
    inputSelect2.setDefaultColor(2);
    cmdSelect.setDefaultColor(2);
    inputSelect1.setButtonColorsToDefault();
    inputSelect2.setButtonColorsToDefault();
    cmdSelect.setButtonColorsToDefault();

    webserver.begin();
    webserver.setDefaultCommand(&defaultCmd);
    webserver.addCommand("form", &formCmd);
    webserver.addCommand("logo.png", &logoCmd);
  }
  else {
    inputSelect1.setDefaultColor(0);  // Off by default
    inputSelect2.setDefaultColor(0);  // Off by default
    cmdSelect.setDefaultColor(0);  // Off by default

    inputSelect1.testSequence();
    inputSelect2.testSequence();
    cmdSelect.testSequence();

    // Initializing the slider:
    utils.uniDirectionalSlider_init();
    utils.uniDirectionalSlider_hasMoved();

    uint16_t inputSources[33] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0, 1000, 2001, 2002, 3010, 3011, 3020, 3021, 6000, 7001, 7002, 10010, 10011};
    // Buttons functions

    buttons1function1[0] = inputSources[(EEPROM.read(401) - 1)];
    buttons1function1[1] = inputSources[(EEPROM.read(406) - 1)];
    buttons1function1[2] = inputSources[(EEPROM.read(402) - 1)];
    buttons1function1[3] = inputSources[(EEPROM.read(403) - 1)];
    buttons1function1[4] = inputSources[(EEPROM.read(404) - 1)];
    buttons1function1[5] = inputSources[(EEPROM.read(405) - 1)];
    buttons1function1[6] = inputSources[(EEPROM.read(407) - 1)];
    buttons1function1[7] = inputSources[(EEPROM.read(408) - 1)];
    buttons1function1[8] = inputSources[(EEPROM.read(409) - 1)];
    buttons1function1[9] = inputSources[(EEPROM.read(410) - 1)];
    buttons1function2[0] = EEPROM.read(411);
    buttons1function2[1] = EEPROM.read(416);
    buttons1function2[2] = EEPROM.read(412);
    buttons1function2[3] = EEPROM.read(413);
    buttons1function2[4] = EEPROM.read(414);
    buttons1function2[5] = EEPROM.read(415);
    buttons1function2[6] = EEPROM.read(417);
    buttons1function2[7] = EEPROM.read(418);
    buttons1function2[8] = EEPROM.read(419);
    buttons1function2[9] = EEPROM.read(420);

    for (uint8_t i = 1; i <= 10; i++) {
      Serial << F("Button: ") << i << F(" input: ") << buttons1function1[i - 1] << F("\n");
    }
    for (uint8_t i = 1; i <= 10; i++) {
      Serial << F("Button: ") << i << F(" function: ") << buttons1function2[i - 1] << F("\n");
    }

    for (uint8_t i = 1; i <= 5; i++) {
      buttons2function[i - 1] = (EEPROM.read(420 + i));
    }

    // Sets the Bi-color LED to off = "no connection"
    digitalWrite(greenLED, !false);
    digitalWrite(redLED, !false);

    // Initialize a connection to the switcher:
    AtemSwitcher.begin(IPAddress(atem_ip[0], atem_ip[1], atem_ip[2], atem_ip[3]), 56417);
    //    AtemSwitcher.serialOutput(0x80);  // Remove or comment out this line for production code. Serial output may decrease performance!
    AtemSwitcher.connect();

    // Set Bi-color LED orange - indicates "connecting...":
    digitalWrite(greenLED, !true);
    digitalWrite(redLED, !true);
  }
}

// These variables are used to track state, for instance when the VGA+PIP button has been pushed.
bool preVGA_active = false;
bool preVGA_UpstreamkeyerStatus = false;
int preVGA_programInput = 0;
bool AtemOnline = false;

void loop() {

  if (isConfigMode)  {
    webserver.processConnection();
    digitalWrite(blueLED, (((unsigned long)millis() >> 3) & B11000000) ? !true : !false);
    if (millis() > 600000) {
      runTest();
    }
  }
  else {
    // Check for packets, respond to them etc. Keeping the connection alive!
    lDelay(0);

    // If the switcher has been initialized, check for button presses as reflect status of switcher in button lights:
    if (AtemSwitcher.hasInitialized())  {
      if (!AtemOnline)  {
        AtemOnline = true;
        Serial.println("Turning on buttons");
        inputSelect1.setDefaultColor(5);  // Dimmed by default
        inputSelect2.setDefaultColor(5);  // Dimmed by default
        cmdSelect.setDefaultColor(5);  // Dimmed by default
        inputSelect1.setButtonColorsToDefault();
        inputSelect2.setButtonColorsToDefault();
        cmdSelect.setButtonColorsToDefault();

        // Set Bi-color LED to red or green depending on mode:
        digitalWrite(redLED, !false);
        digitalWrite(greenLED, !true);
      }
      setButtonColors();
      commandDispatch();
      slider();
    } else {
      if (AtemOnline)  {
        AtemOnline = false;

        Serial.println("Turning off buttons light");
        inputSelect1.setDefaultColor(0);  // Off by default
        inputSelect2.setDefaultColor(0);  // Off by default
        cmdSelect.setDefaultColor(0);  // Off by default
        inputSelect1.setButtonColorsToDefault();
        inputSelect2.setButtonColorsToDefault();
        cmdSelect.setButtonColorsToDefault();

        // Set Bi-color LED off = "no connection"
        digitalWrite(redLED, !true);
        digitalWrite(greenLED, !false);
      }

      Serial.println("Connection to ATEM Switcher has timed out - reconnecting!");

      // Set Bi-color LED orange - indicates "connecting...":
      digitalWrite(redLED, !true);
      digitalWrite(greenLED, !true);
    }
  }
}


uint8_t buttonsTracking[] = {5, 6, 7, 8, 1, 2, 3, 4};


/*************************
   Set button colors
 *************************/
void setButtonColors()  {
  // Setting colors of input select buttons:
  for (uint8_t i = 1; i <= 8; i++)  {
    if (AtemSwitcher.getProgramInputVideoSource(0) == (buttons1function1[i + 1]))  {
      inputSelect2.setButtonColor(buttonsTracking[i - 1], 2);
    }
    else if (AtemSwitcher.getPreviewInputVideoSource(0) == (buttons1function1[i + 1]))  {
      inputSelect2.setButtonColor(buttonsTracking[i - 1], 3);
    }
    else {
      inputSelect2.setButtonColor(buttonsTracking[i - 1], 5);
    }
  }
  if (AtemSwitcher.getProgramInputVideoSource(0) == (buttons1function1[0]))  {
    inputSelect1.setButtonColor(8, 2);
  }
  else if (AtemSwitcher.getPreviewInputVideoSource(0) == (buttons1function1[0]))  {
    inputSelect1.setButtonColor(8, 3);
  }
  else {
    inputSelect1.setButtonColor(8, 5);
  }
  if (AtemSwitcher.getProgramInputVideoSource(0) == (buttons1function1[1]))  {
    inputSelect1.setButtonColor(4, 2);
  }
  else if (AtemSwitcher.getPreviewInputVideoSource(0) == (buttons1function1[1]))  {
    inputSelect1.setButtonColor(4, 3);
  }
  else {
    inputSelect1.setButtonColor(4, 5);
  }
  setButtonColor();
}



/*************************
   Commands handling
 *************************/
void commandDispatch()  {

  // Sending commands, bus input selection:
  uint8_t busSelection1 = inputSelect1.buttonDownAll();
  uint8_t busSelection2 = inputSelect2.buttonDownAll();

  if (inputSelect1.isButtonIn(8, busSelection1))  {
    AtemSwitcher.setPreviewInputVideoSource(0, buttons1function1[0]);
    switch (buttons1function2[0]) {
      case 1:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, !AtemSwitcher.getKeyerOnAirEnabled(0, 0));
        break;
      case 2:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, 1);
        break;
      case 3:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
        break;
      case 4:
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, !AtemSwitcher.getKeyerOnAirEnabled(0, 1));
        break;
      case 5:
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, 1);
        break;
      case 6:
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
        break;
      case 7:
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, !AtemSwitcher.getKeyerOnAirEnabled(0, 2));
        break;
      case 8:
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, 1);
        break;
      case 9:
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
        break;
      case 10:
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, !AtemSwitcher.getKeyerOnAirEnabled(0, 3));
        break;
      case 11:
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, 1);
        break;
      case 12:
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
        break;
      case 13:
        AtemSwitcher.setDownstreamKeyerOnAir(0, !AtemSwitcher.getDownstreamKeyerOnAir(0));
        break;
      case 14:
        AtemSwitcher.setDownstreamKeyerOnAir(0, 1);
        break;
      case 15:
        AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
        break;
      case 16:
        AtemSwitcher.performDownstreamKeyerAutoKeyer(0);
        break;
      case 17:
        AtemSwitcher.setDownstreamKeyerOnAir(1, !AtemSwitcher.getDownstreamKeyerOnAir(1));
        break;
      case 18:
        AtemSwitcher.setDownstreamKeyerOnAir(1, 1);
        break;
      case 19:
        AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
        break;
      case 20:
        AtemSwitcher.performDownstreamKeyerAutoKeyer(1);
        break;
      case 21:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
        AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
        AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
        break;
    }
  }
  if (inputSelect1.isButtonIn(4, busSelection1))  {
    AtemSwitcher.setPreviewInputVideoSource(0, buttons1function1[1]);
    switch (buttons1function2[1]) {
      case 1:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, !AtemSwitcher.getKeyerOnAirEnabled(0, 0));
        break;
      case 2:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, 1);
        break;
      case 3:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
        break;
      case 4:
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, !AtemSwitcher.getKeyerOnAirEnabled(0, 1));
        break;
      case 5:
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, 1);
        break;
      case 6:
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
        break;
      case 7:
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, !AtemSwitcher.getKeyerOnAirEnabled(0, 2));
        break;
      case 8:
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, 1);
        break;
      case 9:
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
        break;
      case 10:
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, !AtemSwitcher.getKeyerOnAirEnabled(0, 3));
        break;
      case 11:
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, 1);
        break;
      case 12:
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
        break;
      case 13:
        AtemSwitcher.setDownstreamKeyerOnAir(0, !AtemSwitcher.getDownstreamKeyerOnAir(0));
        break;
      case 14:
        AtemSwitcher.setDownstreamKeyerOnAir(0, 1);
        break;
      case 15:
        AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
        break;
      case 16:
        AtemSwitcher.performDownstreamKeyerAutoKeyer(0);
        break;
      case 17:
        AtemSwitcher.setDownstreamKeyerOnAir(1, !AtemSwitcher.getDownstreamKeyerOnAir(1));
        break;
      case 18:
        AtemSwitcher.setDownstreamKeyerOnAir(1, 1);
        break;
      case 19:
        AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
        break;
      case 20:
        AtemSwitcher.performDownstreamKeyerAutoKeyer(1);
        break;
      case 21:
        AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
        AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
        AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
        AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
        AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
        AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
        break;
    }
  }

  for (uint8_t i = 1; i <= 8; i++)  {
    if (inputSelect2.isButtonIn(buttonsTracking[i - 1], busSelection2))  {
      AtemSwitcher.setPreviewInputVideoSource(0, buttons1function1[i + 1]);
      switch (buttons1function2[i + 1]) {
        case 1:
          AtemSwitcher.setKeyerOnAirEnabled(0, 0, !AtemSwitcher.getKeyerOnAirEnabled(0, 0));
          break;
        case 2:
          AtemSwitcher.setKeyerOnAirEnabled(0, 0, 1);
          break;
        case 3:
          AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
          break;
        case 4:
          AtemSwitcher.setKeyerOnAirEnabled(0, 1, !AtemSwitcher.getKeyerOnAirEnabled(0, 1));
          break;
        case 5:
          AtemSwitcher.setKeyerOnAirEnabled(0, 1, 1);
          break;
        case 6:
          AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
          break;
        case 7:
          AtemSwitcher.setKeyerOnAirEnabled(0, 2, !AtemSwitcher.getKeyerOnAirEnabled(0, 2));
          break;
        case 8:
          AtemSwitcher.setKeyerOnAirEnabled(0, 2, 1);
          break;
        case 9:
          AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
          break;
        case 10:
          AtemSwitcher.setKeyerOnAirEnabled(0, 3, !AtemSwitcher.getKeyerOnAirEnabled(0, 3));
          break;
        case 11:
          AtemSwitcher.setKeyerOnAirEnabled(0, 3, 1);
          break;
        case 12:
          AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
          break;
        case 13:
          AtemSwitcher.setDownstreamKeyerOnAir(0, !AtemSwitcher.getDownstreamKeyerOnAir(0));
          break;
        case 14:
          AtemSwitcher.setDownstreamKeyerOnAir(0, 1);
          break;
        case 15:
          AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
          break;
        case 16:
          AtemSwitcher.performDownstreamKeyerAutoKeyer(0);
          break;
        case 17:
          AtemSwitcher.setDownstreamKeyerOnAir(1, !AtemSwitcher.getDownstreamKeyerOnAir(1));
          break;
        case 18:
          AtemSwitcher.setDownstreamKeyerOnAir(1, 1);
          break;
        case 19:
          AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
          break;
        case 20:
          AtemSwitcher.performDownstreamKeyerAutoKeyer(1);
          break;
        case 21:
          AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
          AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
          AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
          AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
          AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
          AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
          break;
      }
    }
  }


  // Reading buttons from the Command BI8 board:
  uint8_t cmdSelection = cmdSelect.buttonDownAll();
  if (cmdSelection & (B1 << 0))  {
    cmdSelectionFunction(1);
  }
  if (cmdSelection & (B1 << 2))  {
    cmdSelectionFunction(4);
  }
  if (cmdSelection & (B1 << 3))  {
    cmdSelectionFunction(5);
  }
  if (cmdSelection & (B1 << 6))  {
    cmdSelectionFunction(2);
  }
  if (cmdSelection & (B1 << 7))  {
    cmdSelectionFunction(3);
  }
}

void cmdSelectionFunction (uint8_t i) {
  uint8_t idx;
  switch (i) {
    case 1:
      idx = 1;
      break;
    case 2:
      idx = 7;
      break;
    case 3:
      idx = 8;
      break;
    case 4:
      idx = 3;
      break;
    case 5:
      idx = 4;
      break;
  }
  switch (buttons2function[i - 1]) {
    case 1:
      AtemSwitcher.setKeyerOnAirEnabled(0, 0, !AtemSwitcher.getKeyerOnAirEnabled(0, 0));
      break;
    case 2:
      AtemSwitcher.setKeyerOnAirEnabled(0, 0, 1);
      break;
    case 3:
      AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
      break;
    case 4:
      AtemSwitcher.setKeyerOnAirEnabled(0, 1, !AtemSwitcher.getKeyerOnAirEnabled(0, 1));
      break;
    case 5:
      AtemSwitcher.setKeyerOnAirEnabled(0, 1, 1);
      break;
    case 6:
      AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
      break;
    case 7:
      AtemSwitcher.setKeyerOnAirEnabled(0, 2, !AtemSwitcher.getKeyerOnAirEnabled(0, 2));
      break;
    case 8:
      AtemSwitcher.setKeyerOnAirEnabled(0, 2, 1);
      break;
    case 9:
      AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
      break;
    case 10:
      AtemSwitcher.setKeyerOnAirEnabled(0, 3, !AtemSwitcher.getKeyerOnAirEnabled(0, 3));
      break;
    case 11:
      AtemSwitcher.setKeyerOnAirEnabled(0, 3, 1);
      break;
    case 12:
      AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
      break;
    case 13:
      AtemSwitcher.setDownstreamKeyerOnAir(0, !AtemSwitcher.getDownstreamKeyerOnAir(0));
      break;
    case 14:
      AtemSwitcher.setDownstreamKeyerOnAir(0, 1);
      break;
    case 15:
      AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
      break;
    case 16:
      AtemSwitcher.performDownstreamKeyerAutoKeyer(0);
      break;
    case 17:
      AtemSwitcher.setDownstreamKeyerOnAir(1, !AtemSwitcher.getDownstreamKeyerOnAir(1));
      break;
    case 18:
      AtemSwitcher.setDownstreamKeyerOnAir(1, 1);
      break;
    case 19:
      AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
      break;
    case 20:
      AtemSwitcher.performDownstreamKeyerAutoKeyer(1);
      break;
    case 21:
      AtemSwitcher.setKeyerOnAirEnabled(0, 0, 0);
      AtemSwitcher.setKeyerOnAirEnabled(0, 1, 0);
      AtemSwitcher.setKeyerOnAirEnabled(0, 2, 0);
      AtemSwitcher.setKeyerOnAirEnabled(0, 3, 0);
      AtemSwitcher.setDownstreamKeyerOnAir(0, 0);
      AtemSwitcher.setDownstreamKeyerOnAir(1, 0);
      break;
    case 22:
      cmdSelect.setButtonColor(idx, 4);    // Highlight CUT button
      commandCut();
      break;
    case 23:
      commandAuto();
      break;
    case 24:
      commandFTB();
      break;
    case 25:
      commandPIP();
      break;
    case 26:
      commandPIPVGA();
      break;
  }
}

void setButtonColor () {
  uint8_t idx;
  for (uint8_t i = 1; i <= 5; i++) {
    switch (i) {
      case 1:
        idx = 1;
        break;
      case 2:
        idx = 7;
        break;
      case 3:
        idx = 8;
        break;
      case 4:
        idx = 3;
        break;
      case 5:
        idx = 4;
        break;
    }
    switch (buttons2function[i - 1]) {
      case 1:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 0) ? 4 : 5);
        break;
      case 2:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 0) ? 4 : 5);
        break;
      case 3:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 0) ? 4 : 5);
        break;
      case 4:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 1) ? 4 : 5);
        break;
      case 5:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 1) ? 4 : 5);
        break;
      case 6:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 1) ? 4 : 5);
        break;
      case 7:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 2) ? 4 : 5);
        break;
      case 8:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 2) ? 4 : 5);
        break;
      case 9:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 2) ? 4 : 5);
        break;
      case 10:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 3) ? 4 : 5);
        break;
      case 11:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 3) ? 4 : 5);
        break;
      case 12:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 3) ? 4 : 5);
        break;
      case 13:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(0) ? 4 : 5);
        break;
      case 14:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(0) ? 4 : 5);
        break;
      case 15:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(0) ? 4 : 5);
        break;
      case 16:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(0) ? 4 : 5);
        break;
      case 17:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(1) ? 4 : 5);
        break;
      case 18:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(1) ? 4 : 5);
        break;
      case 19:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(1) ? 4 : 5);
        break;
      case 20:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getDownstreamKeyerOnAir(1) ? 4 : 5);
        break;
      case 21:
        if (AtemSwitcher.getKeyerOnAirEnabled(0, 0) || AtemSwitcher.getKeyerOnAirEnabled(0, 1) || AtemSwitcher.getKeyerOnAirEnabled(0, 2) || AtemSwitcher.getKeyerOnAirEnabled(0, 3) || AtemSwitcher.getDownstreamKeyerOnAir(0) || AtemSwitcher.getDownstreamKeyerOnAir(1)) {
          cmdSelect.setButtonColor(idx, 4);
        }
        else {
          cmdSelect.setButtonColor(idx, 5);
        }
        break;
      case 22:
        if (!cmdSelect.buttonIsPressed(idx))  {
          cmdSelect.setButtonColor(idx, 5); // de-highlight button
        }
        break;
      case 23:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getTransitionPosition(0) > 0 ? 4 : 5);
        break;
      case 24:
        if (AtemSwitcher.getFadeToBlackStateInTransition(0))  {  // Setting button color. This is a more complex example which includes blinking during execution:
          if (AtemSwitcher.getFadeToBlackStateFullyBlack(0) == 0 && (AtemSwitcher.getFadeToBlackStateFramesRemaining(0) != AtemSwitcher.getFadeToBlackRate(0)))  { // It's important to test if fadeToBlack time is more than zero because it's the kind of state from the ATEM which is usually not captured during initialization. Hopefull this will be solved in the future.
            // Blinking with AMBER color if Fade To Black is executing:
            if ((unsigned long)millis() & B10000000)  {
              cmdSelect.setButtonColor(idx, 2);
            }
            else {
              cmdSelect.setButtonColor(idx, 5);
            }
          }
        }
        else if (AtemSwitcher.getFadeToBlackStateFullyBlack(0)) {
          cmdSelect.setButtonColor(idx, 2);  // Sets color of button to RED (2) if Fade To Black is activated
        }
        else {
          cmdSelect.setButtonColor(idx, 5);  // Dimmed background if no fade to black
        }
        break;
      case 25:
        cmdSelect.setButtonColor(idx, AtemSwitcher.getKeyerOnAirEnabled(0, 0) ? 4 : 5);
        break;
      case 26:
        cmdSelect.setButtonColor(idx, preVGA_active ? 4 : 5);
        break;
    }
  }
}

void commandCut()  {
  AtemSwitcher.performCutME(0);
  preVGA_active = false;
}

void commandAuto()  {
  AtemSwitcher.performAutoME(0);
  preVGA_active = false;
}

void commandPIP()  {
  // For Picture-in-picture, do an "auto" transition:
  unsigned long timeoutTime = millis() + 5000;

  // First, store original preview input:
  uint8_t tempPreviewInput = AtemSwitcher.getPreviewInputVideoSource(0);

  // Then, set preview=program (so auto doesn't change input)
  AtemSwitcher.setPreviewInputVideoSource(0, AtemSwitcher.getProgramInputVideoSource(0));
  while (AtemSwitcher.getProgramInputVideoSource(0) != AtemSwitcher.getPreviewInputVideoSource(0))  {
    AtemSwitcher.runLoop();
    if (timeoutTime < millis()) {
      break;
    }
  }

  // Then set transition status:
  bool tempOnNextTransitionStatus = AtemSwitcher.getTransitionNextTransition(0);
  AtemSwitcher.setTransitionNextTransition(0, B00011);  // Set upstream key next transition
  while (!AtemSwitcher.getTransitionNextTransition(0) == 1)  {
    AtemSwitcher.runLoop();
    if (timeoutTime < millis()) {
      break;
    }
  }

  // Make Auto Transition:
  AtemSwitcher.performAutoME(0);
  while (AtemSwitcher.getTransitionPosition(0) == 0)  {
    AtemSwitcher.runLoop();
    if (timeoutTime < millis()) {
      break;
    }
  }
  while (AtemSwitcher.getTransitionPosition(0) > 0)  {
    AtemSwitcher.runLoop();
    if (timeoutTime < millis()) {
      break;
    }
  }

  // Then reset transition status:
  AtemSwitcher.setTransitionNextTransition(0, tempOnNextTransitionStatus);
  while (tempOnNextTransitionStatus != AtemSwitcher.getTransitionNextTransition(0))  {
    AtemSwitcher.runLoop();
    if (timeoutTime < millis()) {
      break;
    }
  }
  // Reset preview bus:
  AtemSwitcher.setPreviewInputVideoSource(0, tempPreviewInput);
  while (tempPreviewInput != AtemSwitcher.getPreviewInputVideoSource(0))  {
    AtemSwitcher.runLoop();
    if (timeoutTime < millis()) {
      break;
    }
  }
  // Finally, tell us how we did:
  if (timeoutTime < millis()) {
    Serial.println("Timed out during operation!");
  }
  else {
    Serial.println("DONE!");
  }
}

void commandPIPVGA()  {
  if (!preVGA_active)  {
    preVGA_active = true;
    preVGA_UpstreamkeyerStatus = AtemSwitcher.getKeyerOnAirEnabled(0, 0);
    preVGA_programInput = AtemSwitcher.getProgramInputVideoSource(0);

    AtemSwitcher.setProgramInputVideoSource(0, 8);
    AtemSwitcher.setKeyerOnAirEnabled(0, 0, 1);
  }
  else {
    preVGA_active = false;
    AtemSwitcher.setProgramInputVideoSource(0, preVGA_programInput);
    AtemSwitcher.setKeyerOnAirEnabled(0, 0, preVGA_UpstreamkeyerStatus);
  }
}

void commandDSK1()  {
  AtemSwitcher.setDownstreamKeyerOnAir(0, !AtemSwitcher.getDownstreamKeyerOnAir(0));
}

void commandFTB() {
  AtemSwitcher.performFadeToBlackME(0);
}


/*************************
   Commands handling
 *************************/
void slider()  {
  // "T-bar" slider:
  if (slider > 0) {
    if (utils.uniDirectionalSlider_hasMoved())  {
      AtemSwitcher.setTransitionPosition(0, 10 * utils.uniDirectionalSlider_position());
      lDelay(20);
      if (utils.uniDirectionalSlider_isAtEnd())  {
        AtemSwitcher.setTransitionPosition(0, 0);
        lDelay(5);
      }
    }
  }
}

void runTest() {
  inputSelect1.testProgramme(B11111111);
  lDelay(20);
  inputSelect2.testProgramme(B00010001);
  lDelay(20);
  cmdSelect.testProgramme(B11001101);
  lDelay(20);
}


/**
   Local delay function
*/
void lDelay(unsigned long timeout)  {
  unsigned long thisTime = millis();
  do {
    if (isConfigMode)  {
      webserver.processConnection();
    } else {
      AtemSwitcher.runLoop();
    }
    Serial << F(".");
    static int k = 1;
    k++;
    if (k > 100) {
      k = 1;
      Serial << F("\n");
    }
  }
  while (!sTools.hasTimedOut(thisTime, timeout));
}
