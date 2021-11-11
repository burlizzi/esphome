#include "remote_transmitter.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

#include <sstream>

#include <Arduino.h>
#include <SPI.h>
// RF12 command codes
#define RF_RECV_CONTROL 0x94A0
#define RF_RECEIVER_ON  0x82DD
#define RF_XMITTER_ON   0x823D
#define RF_IDLE_MODE    0x820D
#define RF_SLEEP_MODE   0x8205
#define RF_WAKEUP_MODE  0x8207
#define RF_TXREG_WRITE  0xB800
#define RF_RX_FIFO_READ 0xB000
#define RF_WAKEUP_TIMER 0xE000

#define CS_PIN          5
#define RFM_IRQ         17
//#define RCV             16
#define TX              26


#define RF12_433MHZ     1   ///< RFM12B 433 MHz frequency band.
#define RF12_868MHZ     2   ///< RFM12B 868 MHz frequency band.
#define RF12_915MHZ     3   ///< RFM12B 915 MHz frequency band.


SPIClass  vspi (VSPI);


static void rf12_xfer (uint16_t cmd) {
    // writing can take place at full speed, even 8 MHz works
    digitalWrite(CS_PIN, LOW);   
    vspi.transfer16(cmd);
    digitalWrite(CS_PIN, HIGH);   
    //delay(1);
}

void rf12_onOff (uint8_t value) {
    rf12_xfer(value ? RF_XMITTER_ON : RF_IDLE_MODE);
    //digitalWrite(TX, value); 

}
#define LOGI(X,...) esp_log_printf_(ESPHOME_LOG_LEVEL_INFO,"l",__LINE__,X, ##__VA_ARGS__)

#define REGISTER_DETAIL 1
#define SerialPrint Serial.print



void readAllRegs()
{
  uint8_t regVal;

#if REGISTER_DETAIL 
  int capVal;

  //... State Variables for intelligent decoding
  uint8_t modeFSK = 0;
  int bitRate = 0;
  int freqDev = 0;
  long freqCenter = 0;
#endif
  
  Serial.println("Address - HEX - BIN");
  for (uint8_t regAddr = 1; regAddr <= 2; regAddr++)
  {
    digitalWrite(CS_PIN, LOW);  
    SPI.transfer(regAddr & 0x7F); // send address + r/w bit
    regVal = SPI.transfer(0);
    digitalWrite(CS_PIN, HIGH);  

    Serial.print(regAddr, HEX);
    Serial.print(" - ");
    Serial.print(regVal,HEX);
    Serial.print(" - ");
    Serial.println(regVal,BIN);

#if REGISTER_DETAIL 
    switch ( regAddr ) 
    {
        case 0x1 : {
            SerialPrint ( "Controls the automatic Sequencer ( see section 4.2 )\nSequencerOff : " );
            if ( 0x80 & regVal ) {
                SerialPrint ( "1 -> Mode is forced by the user\n" );
            } else {
                SerialPrint ( "0 -> Operating mode as selected with Mode bits in RegOpMode is automatically reached with the Sequencer\n" );
            }
            
            SerialPrint( "\nEnables Listen mode, should be enabled whilst in Standby mode:\nListenOn : " );
            if ( 0x40 & regVal ) {
                SerialPrint ( "1 -> On\n" );
            } else {
                SerialPrint ( "0 -> Off ( see section 4.3)\n" );
            }
            
            SerialPrint( "\nAborts Listen mode when set together with ListenOn=0 See section 4.3.4 for details (Always reads 0.)\n" );
            if ( 0x20 & regVal ) {
                SerialPrint ( "ERROR - ListenAbort should NEVER return 1 this is a write only register\n" );
            }
            
            SerialPrint("\nTransceiver's operating modes:\nMode : ");
            capVal = (regVal >> 2) & 0x7;
            if ( capVal == 0b000 ) {
                SerialPrint ( "000 -> Sleep mode (SLEEP)\n" );
            } else if ( capVal == 0b001 ) {
                SerialPrint ( "001 -> Standby mode (STDBY)\n" );
            } else if ( capVal == 0b010 ) {
                SerialPrint ( "010 -> Frequency Synthesizer mode (FS)\n" );
            } else if ( capVal == 0b011 ) {
                SerialPrint ( "011 -> Transmitter mode (TX)\n" );
            } else if ( capVal == 0b100 ) {
                SerialPrint ( "100 -> Receiver Mode (RX)\n" );
            } else {
                Serial.print( capVal, BIN );
                SerialPrint ( " -> RESERVED\n" );
            }
            SerialPrint ( "\n" );
            break;
        }
        
        case 0x2 : {
        
            SerialPrint("Data Processing mode:\nDataMode : ");
            capVal = (regVal >> 5) & 0x3;
            if ( capVal == 0b00 ) {
                SerialPrint ( "00 -> Packet mode\n" );
            } else if ( capVal == 0b01 ) {
                SerialPrint ( "01 -> reserved\n" );
            } else if ( capVal == 0b10 ) {
                SerialPrint ( "10 -> Continuous mode with bit synchronizer\n" );
            } else if ( capVal == 0b11 ) {
                SerialPrint ( "11 -> Continuous mode without bit synchronizer\n" );
            }
            
            SerialPrint("\nModulation scheme:\nModulation Type : ");
            capVal = (regVal >> 3) & 0x3;
            if ( capVal == 0b00 ) {
                SerialPrint ( "00 -> FSK\n" );
                modeFSK = 1;
            } else if ( capVal == 0b01 ) {
                SerialPrint ( "01 -> OOK\n" );
            } else if ( capVal == 0b10 ) {
                SerialPrint ( "10 -> reserved\n" );
            } else if ( capVal == 0b11 ) {
                SerialPrint ( "11 -> reserved\n" );
            }
            
            SerialPrint("\nData shaping: ");
            if ( modeFSK ) {
                SerialPrint( "in FSK:\n" );
            } else {
                SerialPrint( "in OOK:\n" );
            }
            SerialPrint ("ModulationShaping : ");
            capVal = regVal & 0x3;
            if ( modeFSK ) {
                if ( capVal == 0b00 ) {
                    SerialPrint ( "00 -> no shaping\n" );
                } else if ( capVal == 0b01 ) {
                    SerialPrint ( "01 -> Gaussian filter, BT = 1.0\n" );
                } else if ( capVal == 0b10 ) {
                    SerialPrint ( "10 -> Gaussian filter, BT = 0.5\n" );
                } else if ( capVal == 0b11 ) {
                    SerialPrint ( "11 -> Gaussian filter, BT = 0.3\n" );
                }
            } else {
                if ( capVal == 0b00 ) {
                    SerialPrint ( "00 -> no shaping\n" );
                } else if ( capVal == 0b01 ) {
                    SerialPrint ( "01 -> filtering with f(cutoff) = BR\n" );
                } else if ( capVal == 0b10 ) {
                    SerialPrint ( "10 -> filtering with f(cutoff) = 2*BR\n" );
                } else if ( capVal == 0b11 ) {
                    SerialPrint ( "ERROR - 11 is reserved\n" );
                }
            }
            
            SerialPrint ( "\n" );
            break;
        }
        
        case 0x3 : {
            bitRate = (regVal << 8);
            break;
        }
        
        case 0x4 : {
            bitRate |= regVal;
            SerialPrint ( "Bit Rate (Chip Rate when Manchester encoding is enabled)\nBitRate : ");
            unsigned long val = 32UL * 1000UL * 1000UL / bitRate;
            Serial.println( val );
            SerialPrint( "\n" );
            break;
        }
        
        case 0x5 : {
            freqDev = ( (regVal & 0x3f) << 8 );
            break;
        }
        
        case 0x6 : {
            freqDev |= regVal;
            SerialPrint( "Frequency deviation\nFdev : " );
            unsigned long val = 61UL * freqDev;
            Serial.println( val );
            SerialPrint ( "\n" );
            break;
        }
        
        case 0x7 : {
            unsigned long tempVal = regVal;
            freqCenter = ( tempVal << 16 );
            break;
        }
       
        case 0x8 : {
            unsigned long tempVal = regVal;
            freqCenter = freqCenter | ( tempVal << 8 );
            break;
        }

        case 0x9 : {        
            freqCenter = freqCenter | regVal;
            SerialPrint ( "RF Carrier frequency\nFRF : " );
            unsigned long val = 61UL * freqCenter;
            Serial.println( val );
            SerialPrint( "\n" );
            break;
        }

        case 0xa : {
            SerialPrint ( "RC calibration control & status\nRcCalDone : " );
            if ( 0x40 & regVal ) {
                SerialPrint ( "1 -> RC calibration is over\n" );
            } else {
                SerialPrint ( "0 -> RC calibration is in progress\n" );
            }
        
            SerialPrint ( "\n" );
            break;
        }

        case 0xb : {
            SerialPrint ( "Improved AFC routine for signals with modulation index lower than 2.  Refer to section 3.4.16 for details\nAfcLowBetaOn : " );
            if ( 0x20 & regVal ) {
                SerialPrint ( "1 -> Improved AFC routine\n" );
            } else {
                SerialPrint ( "0 -> Standard AFC routine\n" );
            }
            SerialPrint ( "\n" );
            break;
        }
        
        case 0xc : {
            SerialPrint ( "Reserved\n\n" );
            break;
        }

        case 0xd : {
            byte val;
            SerialPrint ( "Resolution of Listen mode Idle time (calibrated RC osc):\nListenResolIdle : " );
            val = regVal >> 6;
            if ( val == 0b00 ) {
                SerialPrint ( "00 -> reserved\n" );
            } else if ( val == 0b01 ) {
                SerialPrint ( "01 -> 64 us\n" );
            } else if ( val == 0b10 ) {
                SerialPrint ( "10 -> 4.1 ms\n" );
            } else if ( val == 0b11 ) {
                SerialPrint ( "11 -> 262 ms\n" );
            }
            
            SerialPrint ( "\nResolution of Listen mode Rx time (calibrated RC osc):\nListenResolRx : " );
            val = (regVal >> 4) & 0x3;
            if ( val == 0b00 ) {
                SerialPrint ( "00 -> reserved\n" );
            } else if ( val == 0b01 ) {
                SerialPrint ( "01 -> 64 us\n" );
            } else if ( val == 0b10 ) {
                SerialPrint ( "10 -> 4.1 ms\n" );
            } else if ( val == 0b11 ) {
                SerialPrint ( "11 -> 262 ms\n" );
            }

            SerialPrint ( "\nCriteria for packet acceptance in Listen mode:\nListenCriteria : " );
            if ( 0x8 & regVal ) {
                SerialPrint ( "1 -> signal strength is above RssiThreshold and SyncAddress matched\n" );
            } else {
                SerialPrint ( "0 -> signal strength is above RssiThreshold\n" );
            }
            
            SerialPrint ( "\nAction taken after acceptance of a packet in Listen mode:\nListenEnd : " );
            val = (regVal >> 1 ) & 0x3;
            if ( val == 0b00 ) {
                SerialPrint ( "00 -> chip stays in Rx mode. Listen mode stops and must be disabled (see section 4.3)\n" );
            } else if ( val == 0b01 ) {
                SerialPrint ( "01 -> chip stays in Rx mode until PayloadReady or Timeout interrupt occurs.  It then goes to the mode defined by Mode. Listen mode stops and must be disabled (see section 4.3)\n" );
            } else if ( val == 0b10 ) {
                SerialPrint ( "10 -> chip stays in Rx mode until PayloadReady or Timeout occurs.  Listen mode then resumes in Idle state.  FIFO content is lost at next Rx wakeup.\n" );
            } else if ( val == 0b11 ) {
                SerialPrint ( "11 -> Reserved\n" );
            }
            
            
            SerialPrint ( "\n" );
            break;
        }
        
        default : {
        }
    }
#endif
  }
    digitalWrite(CS_PIN, HIGH);
}

/**/









namespace esphome {
namespace remote_transmitter {



static const char *const TAG = "rfm12_transmitter";





uint8_t rf12_initialize (uint8_t id, uint8_t band, float frequency=433.92, uint8_t g=0xD4) {
   
    uint16_t reg=(frequency-430)*400;
    reg &= 0xfff;
    reg = 0xA000 + reg;
    ESP_LOGD(TAG,"freq=%f reg=%x(%d)",frequency,reg,reg);

 
    rf12_xfer(0x0000); // initial SPI transfer added to avoid power-up problem
    
    rf12_xfer(RF_SLEEP_MODE); // DC (disable clk pin), enable lbd

    // wait until RFM12B is out of power-up reset, this takes several *seconds*
    rf12_xfer(RF_TXREG_WRITE); // in case we're still in OOK mode
    //while (digitalRead(RFM_IRQ) == 0)
   for (size_t i = 0; i < 100; i++)
   {
       rf12_xfer(0x0000);
   }
        

    rf12_xfer(0x80C7 | (band << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF
    rf12_xfer(0xA000 + reg); // 96-3960 freq range of values within band
    rf12_xfer(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
    rf12_xfer(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
    rf12_xfer(0xC2AC); // AL,!ml,DIG,DQD4
    if (g != 0) {
        rf12_xfer(0xCA83); // FIFO8,2-SYNC,!ff,DR
        rf12_xfer(0xCE00 | g); // SYNC=2DXX；
    } else {
        rf12_xfer(0xCA8B); // FIFO8,1-SYNC,!ff,DR
        rf12_xfer(0xCE2D); // SYNC=2D；
    }
    rf12_xfer(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
    rf12_xfer(0x9800); // !mp,90kHz,MAX OUT
    rf12_xfer(0xCC77); // OB1，OB0, LPX,！ddy，DDIT，BW0
    rf12_xfer(0xE000); // NOT USE
    rf12_xfer(0xC800); // NOT USE
    rf12_xfer(0xC049); // 1.66MHz,3.1V

 

    return id;
}


#define rf12_control rf12_xfer
void RemoteTransmitterComponent::rf12_init_OOK (float frequency) 
{
    uint16_t reg=(frequency-430)*400;
    reg &= 0xfff;
    reg = 0xA000 + reg;
    ESP_LOGD(TAG,"freq=%f reg=%x(%d)",frequency,reg,reg);


    rf12_control(0x8017); // 8027    868 Mhz;disabel tx register; disable RX
                          //         fifo buffer; xtal cap 12pf, same as xmitter
    rf12_control(0x82c0); // 82C0    enable receiver; enable basebandblock 
    rf12_control(0xA000 + reg); // A68A    868.2500 MHz
    rf12_control(0xc691); // C691    c691 datarate 2395 kbps 0xc647 = 4.8kbps 
    rf12_control(0x9425); // 9420    VDI; FAST;400khz;GAIn MAX; DRSSI -103dbm 
//    rf12_control(0x9489); // 9489    VDI; FAST;200khz;GAIn -6db; DRSSI 97dbm 
    rf12_control(0xC220); // C220    datafiltercommand; ** not documented cmd 
    rf12_control(0xCA00); // CA00    FiFo and resetmode cmd; FIFO fill disabeld
    rf12_control(0xC473); // C473    AFC run only once; enable AFC; enable
                          //         frequency offset register; +3 -4
    rf12_control(0xCC67); // CC67    pll settings command
    rf12_control(0xB800); // TX register write command not used
    rf12_control(0xC800); // disable low dutycycle 
    rf12_control(0xC040); // 1.66MHz,2.2V not used see 82c0  
}



void RemoteTransmitterComponent::setup() {
    vspi.setFrequency(80*1000000);
    vspi.begin();
    pinMode(CS_PIN, OUTPUT); //VSPI SS
    pinMode(TX, OUTPUT); //VSPI SS
    //digitalWrite(23, LOW);  

    rf12_initialize(0, RF12_433MHZ,frequency_);
    //rf12_initialize(0, RF12_868MHZ);
    rf12_init_OOK (frequency_);


}

void RemoteTransmitterComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Remote Transmitter...");
  //ESP_LOGCONFIG(TAG, "  Carrier Duty: %u%%", this->carrier_duty_percent_);
  //LOG_PIN("  Pin: ", this->pin_);
  readAllRegs();
}

void RemoteTransmitterComponent::calculate_on_off_time_(uint32_t carrier_frequency, uint32_t *on_time_period,
                                                        uint32_t *off_time_period) {
  if (carrier_frequency == 0) {
    *on_time_period = 0;
    *off_time_period = 0;
    return;
  }
  uint32_t period = (1000000UL + carrier_frequency / 2) / carrier_frequency;  // round(1000000/freq)
  period = std::max(uint32_t(1), period);
}

void RemoteTransmitterComponent::mark_12( uint32_t usec) {
    //ESP_LOGD(TAG, "mark");
    rf12_onOff(1);
    delayMicroseconds(usec+178);
    rf12_onOff(0);
    return;
  
}
void RemoteTransmitterComponent::space_12(uint32_t usec) {
  rf12_onOff(0);
    delayMicroseconds(usec-178);
//  delay_microseconds_accurate(usec);
    //ESP_LOGD(TAG, "space");

}

void RemoteTransmitterComponent::set_mode(uint32_t mode) {
    rf12_control(mode);
}

void RemoteTransmitterComponent::set_frequency(float frequency) {
    this->frequency_=frequency;
    //rf12_initialize(0, RF12_433MHZ,frequency);
    rf12_init_OOK(frequency); 
}


void RemoteTransmitterComponent::send_internal(uint32_t send_times, uint32_t send_wait) {
  std::stringstream s;
  for (int32_t item : this->temp_.get_data()) 
    s<<item<<',';


  ESP_LOGD(TAG, "Sending rfm12 remote code... %d %d %d %s",send_wait,send_times,this->temp_.get_data().size(),s.str().c_str());
     vspi.setFrequency(80*1000000);

  //rf12_xfer(RF_XMITTER_ON);
  digitalWrite(TX, HIGH); 
  for (uint32_t i = 0; i < send_times; i++) {
    {
      InterruptLock lock;
      for (int32_t item : this->temp_.get_data()) {
        if (item > 0) {
          const auto length = uint32_t(item);
          this->mark_12( length);
        } else {
          const auto length = uint32_t(-item);
          this->space_12(length);
        }
        App.feed_wdt();
      }
    }
    if (i + 1 < send_times) {
      delay_microseconds_accurate(send_wait);
    }
  }
  digitalWrite(TX, LOW); 

    rf12_xfer(RF_IDLE_MODE);
    rf12_init_OOK (this->frequency_);

}

}  // namespace remote_transmitter
}  // namespace esphome

