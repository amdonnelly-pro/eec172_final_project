
#include "speaker.h"
#include "sound.h"
#include "i2s.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom_map.h"
#include "prcm.h"
#include "stdint.h"

void Init_DAC_I2S(void)
{
    MAP_PRCMI2SClockFreqSet(256000);

    MAP_I2SConfigSetExpClk(I2S_BASE, MAP_PRCMPeripheralClockGet(PRCM_I2S),
                           8000, I2S_SLOT_SIZE_16 | I2S_PORT_CPU);

    MAP_I2SSerializerConfig(I2S_BASE,I2S_DATA_LINE_0,I2S_SER_MODE_TX,
                            I2S_INACT_LOW_LEVEL);

    MAP_I2SEnable(I2S_BASE, I2S_MODE_TX_ONLY);
}

void PlaySound(void)
{
    const int16_t *audio = (const int16_t*)output_pcm;

    uint32_t samples = output_pcm_len / 2;

    uint32_t i;

    for(i = 0; i < samples; i++) {
        int16_t s = audio[i];
        uint32_t frame = ((uint16_t)s << 16) | (uint16_t)s;

        while(!MAP_I2SDataPutNonBlocking(I2S_BASE, I2S_DATA_LINE_0, frame))
        {
        }
    }
}
