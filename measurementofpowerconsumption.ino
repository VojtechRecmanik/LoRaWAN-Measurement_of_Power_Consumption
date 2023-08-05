#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <PZEM004Tv30.h>
#include <CayenneLPP.h>
CayenneLPP lpp(51);
PZEM004Tv30 pzem(Serial1);

//--------------------------------------- Here change your keys -------------------------------------------------

static const PROGMEM u1_t NWKSKEY[16] = { 0x13, 0x19, 0xC2, 0x32, 0xD1, 0x7B, 0xCC, 0x7A, 0x9E, 0xD0, 0x5F, 0xE1, 0x0F, 0x67, 0xF6, 0x7C };
static const u1_t PROGMEM APPSKEY[16] = { 0x5F, 0xA5, 0x71, 0x1F, 0xF1, 0x4C, 0x76, 0x1C, 0xD4, 0xD2, 0xCB, 0x0E, 0xD1, 0xC6, 0x0E, 0x6A };
static const u4_t DEVADDR = 0x260BC9EF;

//---------------------------------------------------------------------------------------------------------------

#if defined(ARDUINO_SAMD_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0)
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,
    .spi_freq = 8000000,
};
#elif defined(ARDUINO_AVR_FEATHER32U4)
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {7, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,
    .spi_freq = 1000000,
};
#else
# error "Unknown target"
#endif

static osjob_t sendjob;       

const unsigned TX_INTERVAL = 30;

void onEvent (ev_t ev) {
    if(ev == EV_TXCOMPLETE) {
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }
}

void do_send(osjob_t* j){
        
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    Serial.println("Sending: ");
    Serial.print("Voltage: ");
    Serial.print(voltage, 1);
    Serial.println(" V");
    Serial.print("Current: ");
    Serial.print(current, 3);
    Serial.println(" A");
    Serial.print("Power: ");
    Serial.print(power, 1);
    Serial.println(" W");
    Serial.print("Energy: ");
    Serial.print(energy, 3);
    Serial.println(" kWh");
    Serial.print("Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");
    Serial.print("Power Factor: ");
    Serial.println(pf, 2);

    delay(1000);

    lpp.reset();
    lpp.addVoltage(1,voltage);
    lpp.addCurrent(2,current);
    lpp.addPower(3,power);
    lpp.addEnergy(4,energy);
    lpp.addFrequency(5,frequency);
    lpp.addAnalogInput(6, pf);
    LMIC_setTxData2(1, lpp.getBuffer(), lpp.getSize(), 0);
}

void setup() {
    Serial.begin(9600);
    
    os_init();
    LMIC_reset();

    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);

    //EU868
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    
    LMIC_setLinkCheckMode(0);       
    LMIC.dn2Dr = DR_SF9;            
    LMIC_setDrTxpow(DR_SF9,14);     
    LMIC_setAdrMode(0);             

    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100); 

    do_send(&sendjob);     
}

void loop() {
    os_runloop_once();
}
