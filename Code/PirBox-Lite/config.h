/////////////////////////// LoRa Gateway Key ///////////////////////////

#define GATEWAY_KEY "xy"          // Keep it Short
#define NODE_NAME "PirBoxL"        // Sensor Name which will be visible in Home Assistant

////////////////////////////// LORA CONFIG ////////////////////////////////////

// For sending 6 bytes ("6 characters") of data using different Spreading Factors (SF), the estimated time on air (ToA)
//
// SF7: Approximately 0.027 seconds (27.494 milliseconds)
// SF8: Approximately 0.052 seconds (52.224 milliseconds)
// SF9: Approximately 0.100 seconds (100.147 milliseconds)
// SF10: Approximately 0.193 seconds (193.413 milliseconds)
// SF11: Approximately 0.385 seconds (385.297 milliseconds)
// SF12: Approximately 0.746 seconds (746.127 milliseconds)
//
// These calculations demonstrate how the time on air increases with higher Spreading Factors
// due to the decreased data rate, which is a trade-off for increased communication range and signal robustness.

#define BAND                      868E6     // 433E6 MHz or 868E6 MHz or 915E6 MHz
#define TX_OUTPUT_POWER           20        // dBm tx output power
#define LORA_BANDWIDTH            4         // bandwidth 2: 31.25Khz, 3: 62.5Khz, 4: 125Khz, 5: 250KHZ, 6: 500Khz
#define LORA_SPREADING_FACTOR     8         // spreading factor 6-12 [SF5..SF12]
#define LORA_CODINGRATE           1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH      6         // Same for Tx and Rx
#define LORA_PAYLOADLENGTH        0         // 0: Variable length packet (explicit header),  1..255 for Fixed length packet (implicit header)
#define LORA_CRC_ON               true

// PirBox uses the SX126x module, which requires a 16-bit sync word (e.g., 0x3444 or 0x1424).
// Don’t use sync words 0x1444 it compatible with public and privat at the same times.
// Do NOT use 0x3444 — this is reserved for LoRaWAN (equivalent to 8-bit 0x34).
// CapiBridge uses the SX127x module, which uses an 8-bit sync word (e.g., 0x12 or 0x34).
//
// Use 0x1424 for private (non-LoRaWAN) networks — this matches 8-bit CapiBridge sync word "0x12".
// You can also choose a custom sync word to isolate your network:
//   SX126x   →     SX127x Equivalent
//   0x1437   →         0x37
//   0x14A9   →         0xA9
//   0x14D5   →         0xD5
//
//     Only use values starting with 0x14 (i.e., format 0x14YZ), as this format is required
//     by the SX126x for compatibility with 8-bit sync words.

#define LORA_SYNC_WORD            0x1424  // 0x1424 Private sync word.
