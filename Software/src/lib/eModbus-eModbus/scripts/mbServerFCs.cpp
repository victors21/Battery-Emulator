#include "mbServerFCs.h"
#undef LOCAL_LOG_LEVEL
#define LOCAL_LOG_LEVEL LOG_LEVEL_VERBOSE
#include "../Logging.h"
#include <map>
#include <cstdint>
//modbus register memory - declared in main.cpp

// uint16_t mbPV[MBPV_MAX];

static std::map<uint16_t, std::uint16_t> mbPVmap;
static bool is_initialized = false;

// Server function to handle FC 0x03
ModbusMessage FC03(ModbusMessage request)
{
    ModbusMessage response; // The Modbus message we are going to give back
    uint16_t addr = 0;      // Start address
    uint16_t words = 0;     // # of words requested
    request.get(2, addr);   // read address from request
    request.get(4, words);  // read # of words from request
    Serial.println(addr);
    Serial.println(words);
    // for (const auto& pair : mbPVmap) {
    //     Serial.printf("%d: 0x%X\n", pair.first, pair.second);
    // }
    char debugString[1000];

    LOG_D("FC03 received: read %d words @ %d\r\n", words, addr);

    // Address overflow?
    // if ((addr + words) > MBPV_MAX)
    // {
    //     // Yes - send respective error response
    //     response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    //     LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
    //     return response;
    // }

    // Set up response
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
    sprintf(debugString, "Read : ");
    for (uint8_t i = 0; i < words; ++i)
    {
        // send increasing data values
        if(mbPVmap.count(addr + i) == 0) {
            if((addr + i) > MBPV_MAX) {
                // Key doesn't exist - send respective error response
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
                return response;
            }
        }
        response.add((uint16_t)(mbPVmap[addr + i]));
        sprintf(debugString + strlen(debugString), "%i ", mbPVmap[addr + i]);
    }
    LOG_V("%s\r\n", debugString);
    // Serial.println(debugString);

    return response;
}

// Server function to handle FC 0x06
ModbusMessage FC06(ModbusMessage request)
{
    //Serial.println(request);
    ModbusMessage response; // The Modbus message we are going to give back
    uint16_t addr = 0;      // Start address
    uint16_t val = 0;       // value to write
    request.get(2, addr);   // read address from request
    request.get(4, val);    // read # of words from request

    Serial.println(addr);
    Serial.println(val);

    LOG_D("FC06 received: write 1 word @ %d\r\n", addr);

    // Address overflow?
    // if ((addr) > MBPV_MAX)
    if(mbPVmap.count(addr) == 0)
    {
        if(addr > MBPV_MAX) {
            // Yes - send respective error response
            response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
            LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
            Serial.println("Error!?");
            return response;
        }
    }
    // Do the write
    mbPVmap[addr] = val;
    LOG_V("Write : %i", val);

    // Set up response
    response.add(request.getServerID(), request.getFunctionCode(), mbPVmap[addr]);
    Serial.println("All is well?");
    return response;
}

// Server function to handle FC 0x10 (FC16)
ModbusMessage FC16(ModbusMessage request)
{
    //Serial.println(request);
    ModbusMessage response; // The Modbus message we are going to give back
    uint16_t addr = 0;      // Start address
    uint16_t words = 0;     // total words to write
    uint8_t bytes = 0;      // # of data bytes in request
    uint16_t val = 0;       // value to be written
    request.get(2, addr);   // read address from request
    request.get(4, words);  // read # of words from request
    request.get(6, bytes);  // read # of data bytes from request (seems redundant with # of words)
    char debugString[1000];
    Serial.println("FC16!");

    LOG_D("FC16 received: write %d words @ %d\r\n", words, addr);

    // # of registers proper?
    if ((bytes != (words * 2)) // byte count in request must match # of words in request
        || (words > 123))      // can't support more than this in request packet
    {                          // Yes - send respective error response
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
        LOG_D("ERROR - ILLEGAL DATA VALUE\r\n");
        return response;
    }
    // Address overflow?
    // if ((addr + words) > MBPV_MAX)
    // {
    //     // Yes - send respective error response
    //     response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    //     LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
    //     return response;
    // }

    // Do the writes
    sprintf(debugString, "Write : ");
    for (uint8_t i = 0; i < words; ++i)
    {
        if(mbPVmap.count(addr + i) == 0) {
            if((addr + i) > MBPV_MAX) {
                // Key doesn't exist - send respective error response
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
                return response;
            }
        }

        request.get(7 + (i * 2), val); //data starts at byte 6 in request packet
        mbPVmap[addr + i] = val;
        sprintf(debugString + strlen(debugString), "%i ", mbPVmap[addr + i]);
    }
    LOG_V("%s\r\n", debugString);

    // Set up response
    response.add(request.getServerID(), request.getFunctionCode(), addr, words);
    return response;
}

// Server function to handle FC 0x17 (FC23)
ModbusMessage FC23(ModbusMessage request)
{
    //Serial.println(request);
    ModbusMessage response;       // The Modbus message we are going to give back
    uint16_t read_addr = 0;       // Start address for read
    uint16_t read_words = 0;      // # of words requested for read
    uint16_t write_addr = 0;      // Start address for write
    uint16_t write_words = 0;     // total words to write
    uint8_t write_bytes = 0;      // # of data bytes in write request
    uint16_t write_val = 0;       // value to be written
    request.get(2, read_addr);    // read address from request
    request.get(4, read_words);   // read # of words from request
    request.get(6, write_addr);   // read address from request
    request.get(8, write_words);  // read # of words from request
    request.get(10, write_bytes); // read # of data bytes from request (seems redundant with # of words)
    char debugString[1000];

    LOG_D("FC23 received: write %d @ %d, read %d @ %d\r\n", write_words, write_addr, read_words, read_addr);

    // ERROR CHECKS
    // # of registers proper?
    if ((write_bytes != (write_words * 2)) // byte count in request must match # of words in request
        || (write_words > 121)             // can't fit more than this in the packet for FC23
        || (read_words > 125))             // can't fit more than this in the response packet
    {                                      // Yes - send respective error response
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
        LOG_D("ERROR - ILLEGAL DATA VALUE\r\n");
        return response;
    }
    // Address overflow?
    // if (((write_addr + write_words) > MBPV_MAX) || ((read_addr + read_words) > MBPV_MAX))
    // { // Yes - send respective error response
    //     response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    //     LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
    //     return response;
    // }

    //WRITE SECTION  - write is done before read for FC23
    // Do the writes
    sprintf(debugString, "Write: ");
    for (uint8_t i = 0; i < write_words; ++i)
    {
        if(mbPVmap.count(write_addr + i) == 0)
        {
            if((write_addr + i) > MBPV_MAX) {
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
                return response;
            }
        }

        request.get(11 + (i * 2), write_val); //data starts at byte 6 in request packet
        mbPVmap[write_addr + i] = write_val;
        sprintf(debugString + strlen(debugString), "%i ", mbPVmap[write_addr + i]);
    }
    LOG_V("%s\r\n", debugString);

    // READ SECTION
    // Set up response
    sprintf(debugString, "Read: ");
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(read_words * 2));
    for (uint8_t i = 0; i < read_words; ++i)
    {
        if(mbPVmap.count(read_addr + i) == 0)
        {
            if((read_addr + i) > MBPV_MAX) {
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                LOG_D("ERROR - ILLEGAL DATA ADDRESS\r\n");
                return response;
            }
        }
        // send increasing data values
        response.add((uint16_t)(mbPVmap[read_addr + i]));
        sprintf(debugString + strlen(debugString), "%i ", mbPVmap[read_addr + i]);
    }
    LOG_V("%s\r\n", debugString);

    return response;
}

std::map<uint16_t, uint16_t>& mbServerFCs_GetMapRef(void)
{
    return mbPVmap;
}
