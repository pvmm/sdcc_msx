#include <psg.h>

//#ifdef ENABLE_PSG

T_AY_3_8910_Registers AY_3_8910_Registers;

void PSG_init() {
	
	memset(AY_3_8910_Registers.reg,0,14);
	AY_3_8910_Registers.enable.value = 0xFF;
	
}

#ifdef MSX
	__sfr __at 0xA0 PSG0;
	__sfr __at 0xA1 PSG1;
	
	
	void PSG_syncRegisters_asm();
	
	static void PSG_syncRegisters_asm_placeholder() __naked {
			
		__asm
_PSG_syncRegisters_asm:
			XOR A
			LD C,#0xA0
			LD HL,#_AY_3_8910_Registers
__LOUT:		OUT (C),A
			INC C
			OUTI 
			DEC C
			INC A
			CP #13
			JR NZ,__LOUT
			OUT (C),A
			LD A,(HL)
			AND A
			RET M
			INC C
			OUT (C),A
			RET	
		__endasm;
	}


	void PSG_syncRegisters() {
		
		AY_3_8910_Registers.enable.ioa_out = 0;
		AY_3_8910_Registers.enable.iob_out = 1;
		PSG_syncRegisters_asm();
	}

#else

	void PSG_syncRegisters() {}
#endif

////////////////////////////////////////////////////////////////////////
// AYR PLAYER
static const uint16_t midi2ay[] = {
	0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 
	0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFE4, 0xEFF, 0xE28, 0xD5C, 0xC9D, 0xBE7, 0xB3C, 0xA9B, 0xA02, 0x973, 0x8EB, 
	0x86B, 0x7F2, 0x780, 0x714, 0x6AE, 0x64E, 0x5F4, 0x59E, 0x54D, 0x501, 0x4B9, 0x475, 0x435, 0x3F9, 0x3C0, 0x38A, 
	0x357, 0x327, 0x2FA, 0x2CF, 0x2A7, 0x281, 0x25D, 0x23B, 0x21B, 0x1FC, 0x1E0, 0x1C5, 0x1AC, 0x194, 0x17D, 0x168, 
	0x153, 0x140, 0x12E, 0x11D, 0x10D, 0x0FE, 0x0F0, 0x0E2, 0x0D6, 0x0CA, 0x0BE, 0x0B4, 0x0AA, 0x0A0, 0x097, 0x08F, 
	0x087, 0x07F, 0x078, 0x071, 0x06B, 0x065, 0x05F, 0x05A, 0x055, 0x050, 0x04C, 0x047, 0x043, 0x040, 0x03C, 0x039, 
	0x035, 0x032, 0x030, 0x02D, 0x02A, 0x028, 0x026, 0x024, 0x022, 0x020, 0x01E, 0x01C, 0x01B, 0x019, 0x018, 0x016, 
	0x015, 0x014, 0x013, 0x012, 0x011, 0x010, 0x00F, 0x00E, 0x00D, 0x00D, 0x00C, 0x00B, 0x00B, 0x00A, 0x009, 0x009 };

typedef struct {
	
	uint8_t duration;
	const uint8_t *p;
} T_AYR_Channel_Status;

typedef struct {
	
	AYR ayr;
    bool busy, paused;
	uint8_t frameCount;
    int8_t volume;
    int8_t fade_out;
	
	T_AYR_Channel_Status channels[3];
} T_AYR_Status;


static T_AY_3_8910_Registers ayr_registers;
static T_AYR_Status ayr_status;

void ayr_init() {
	
	ayr_play(nullptr); 
}



void ayr_play(const AYR *ayr) {
    
	ayr_status.busy = false;
    ayr_status.paused = 0;
	ayr_status.frameCount = 0;
    ayr_status.volume = 0;
    ayr_status.fade_out = 0;
	
	if (ayr==nullptr) return;
    
    memcpy(&ayr_status.ayr, ayr, sizeof(AYR));
	ayr_status.busy = true;
    
    memset(ayr_registers.reg,0,14);
    ayr_registers.enable.value = 0xFF;

    ayr_status.channels[0].duration = 1;
    ayr_status.channels[1].duration = 1;
    ayr_status.channels[2].duration = 1;
    
    ayr_status.channels[0].p = ayr_status.ayr.channel[0].data;
    ayr_status.channels[1].p = ayr_status.ayr.channel[1].data;
    ayr_status.channels[2].p = ayr_status.ayr.channel[2].data;
}

void ayr_restart() { ayr_play(&ayr_status.ayr); }
void ayr_pause() { ayr_status.paused = true; }
void ayr_resume() { ayr_status.paused = false; }
void ayr_stop() { ayr_init(); }
void ayr_set_volume(int8_t volume) {ayr_status.volume = volume;}
void ayr_fade_out(int8_t fade_out) {ayr_status.fade_out = fade_out;}

INLINE uint8_t ayr_spin_channel( uint8_t i ) {
    
    T_AYR_Channel_Status *chan = &ayr_status.channels[i];

	if (chan->duration == 0) {
        ayr_registers.amplitude[i].volume = 0;
        return 0;
    }
	// We return true iff the chanel has finished playing.
	
	if (--chan->duration) return 1;
	
	{
		uint8_t meta = *chan->p++;

        {
            uint8_t vol = (meta & 0x0F) + 16 + (ayr_status.volume/16);
            if (vol>16+15) vol = 16+15;
            if (vol<16) vol = 16;
            vol -= 16;
            
            ayr_registers.amplitude[i].volume = vol;
        }

		{
			uint8_t duration = (meta&0x70)>>4;
					
			if ( duration == 0x07 )
				duration = *chan->p++;

			chan->duration = duration;

            for (uint8_t j=0; j<i; j++) printf("                ");
            printf("%d: %X (%02X)\n", i, (meta & 0x0F), duration);
		}
        
		if ( !!(meta & 0x80) ) {
			
			uint8_t tone = *chan->p++;
			if (tone) {
				ayr_registers.tone[i] = midi2ay[tone];
				switch (i) {
				case 0:
					ayr_registers.enable.tone_a  = 0;
					ayr_registers.enable.noise_a = 1;
					break;
				case 1:
					ayr_registers.enable.tone_b  = 0;
					ayr_registers.enable.noise_b = 1;
					break;
				case 2:
				default:
					ayr_registers.enable.tone_c  = 0;
					ayr_registers.enable.noise_c = 1;
					break;
				
				}
			} else {
				switch (i) {
				case 0:
					ayr_registers.enable.tone_a  = 1;
					ayr_registers.enable.noise_a = 0;
					break;
				case 1:
					ayr_registers.enable.tone_b  = 1;
					ayr_registers.enable.noise_b = 0;
					break;
				case 2:
				default:
					ayr_registers.enable.tone_c  = 1;
					ayr_registers.enable.noise_c = 0;
					break;
				}
			}
		}

	}	
	return 1;	
}


static uint8_t ayr_spin_channel_0() { uint8_t r; IN_SEGMENT(ayr_status.ayr.channel[0].segment, PAGE_D, r=ayr_spin_channel(0);); return r;}
static uint8_t ayr_spin_channel_1() { uint8_t r; IN_SEGMENT(ayr_status.ayr.channel[1].segment, PAGE_D, r=ayr_spin_channel(1);); return r;}
static uint8_t ayr_spin_channel_2() { uint8_t r; IN_SEGMENT(ayr_status.ayr.channel[2].segment, PAGE_D, r=ayr_spin_channel(2);); return r;}

bool ayr_spin() {
		
	if (ayr_status.busy==false) return false;
    if (ayr_status.paused==true) return true;
    
	if (msxhal_get_interrupt_frequency()==MSX_FREQUENCY_60HZ) {

		if (++ayr_status.frameCount == 6) {
			ayr_status.frameCount=0;
			memcpy(AY_3_8910_Registers.reg, ayr_registers.reg, sizeof(AY_3_8910_Registers));
			return true;
		}
	}
	
    {
        ayr_status.volume -= ayr_status.fade_out;
    }
    
    {
        if (ayr_status.volume<-120) {
			ayr_status.busy=false;
            return false;
        }
    }
    
	{
		if (ayr_spin_channel_0() +
			ayr_spin_channel_1() +
			ayr_spin_channel_2() == 0) {
			ayr_status.busy=false;
            return false;
		}
	}

	memcpy(AY_3_8910_Registers.reg, ayr_registers.reg, sizeof(AY_3_8910_Registers));
    return true;
}


////////////////////////////////////////////////////////////////////////
// AYFX PLAYER

//16-bit numbers stored in the LSB,MSB format.
//Single effect, file extension .afx 
//Every frame encoded with a flag byte and a number of bytes, which is vary depending from value change flags.
 //bit0..3  Volume
 //bit4     Disable T
 //bit5     Change Tone
 //bit6     Change Noise
 //bit7     Disable N
//When the bit5 set, two bytes with tone period will follow; when the bit6 set, a single byte with noise period will follow; when both bits are set, first two bytes of tone period, then single byte with noise period will follow. When none of the bits are set, next flags byte will follow. 
//End of the effect is marked with byte sequence #D0, #20. Player should detect it before outputting it to the AY registers, by checking noise period value to be equal #20. The editor considers last non-zero volume value as the last frame of an effect, other parameters aren't matter. 
//Effects bank, file extension .afb 
//Header: 

//+0 (1 byte) Total number of effects in the bank, up to 256 (0 means 256);
//+1 (2 bytes per effect) Table of offsets to data of every effect. Offset value is given relative to the second byte of the offset itself, this allows to calculate absolute address very fast: 

   //hl=offset in the effects table
   //ld c,(hl)
   //inc hl
   //ld b,(hl)
   //add hl,bc
   //hl=absolute address of effect data
//Data: 
//+n (m bytes) The effects data, format is the same as in the single effect file. After every effect there could be a null terminated text string with name of the effect. It may absent, if the bank was saved into a file using corresponding item of the File menu, in this case null terminator bytes are absent as well.

typedef union {
	uint8_t value;
	struct {
		uint8_t volume   : 4;
		uint8_t disableT : 1;
		uint8_t changeT  : 1;
		uint8_t changeN  : 1;
		uint8_t disableN : 1;
	};
} T_ayFX_Flags;


typedef struct {
	const uint8_t *afx;
	uint16_t tone;
	uint8_t noise;
	uint8_t priority;
	int8_t adjustedVolume;
	uint8_t segment;

} T_ayFX_Effect;

#define AYFX_MAX_EFFECTS 8
typedef struct {
	
	T_ayFX_Effect effects[AYFX_MAX_EFFECTS];
	uint8_t sortedEffects[AYFX_MAX_EFFECTS];
	uint8_t nEffects;
	
	uint8_t channel;
	
} T_ayFX_Status;

static T_ayFX_Status ayFX_status;

void ayFX_init() {
	
	PSG_init(); 
	
	ayFX_status.channel = 0;
	ayFX_status.nEffects = 0;
	{
		uint8_t i;
		for (i=0; i<AYFX_MAX_EFFECTS; i++)
			ayFX_status.sortedEffects[i] = i;
	}
}


INLINE T_ayFX_Effect *ayFX_effect(uint8_t idx) {
	
	return &ayFX_status.effects[ayFX_status.sortedEffects[idx]];
}

static void ayFX_swap(uint8_t idx1, uint8_t idx2) {

	uint8_t s = ayFX_status.sortedEffects[idx1];
	ayFX_status.sortedEffects[idx1] = ayFX_status.sortedEffects[idx2];
	ayFX_status.sortedEffects[idx2] = s;
}

static void ayFX_remove(uint8_t idx) {
	
	ayFX_status.nEffects--;
	while (idx<ayFX_status.nEffects) {
		ayFX_swap(idx, idx+1);
		idx++;
	}
}


void ayFX_afx(const uint8_t *afx, uint8_t segment, uint8_t priority, int8_t adjustedVolume) {
	
	
	uint8_t idx = ayFX_status.nEffects;

	if (idx==AYFX_MAX_EFFECTS) {
		
		ayFX_remove(AYFX_MAX_EFFECTS-1);
		idx--;
	}
	ayFX_status.nEffects++;
	
	ayFX_effect(idx)->afx = afx;
	ayFX_effect(idx)->priority = priority;
	ayFX_effect(idx)->adjustedVolume = adjustedVolume;
	ayFX_effect(idx)->segment = segment;
	
	while (idx>0 && ayFX_effect(idx-1)->priority <= ayFX_effect(idx)->priority) {
		ayFX_swap(idx-1, idx);
		idx--;
	}
}

void ayFX_afb(const uint8_t *afb, uint8_t segment, uint8_t idx, uint8_t priority, int8_t adjustedVolume) {
	
	uint8_t oldSegmentPageD = mapper_load_segment(segment, PAGE_D);

	const uint16_t *afxTable = (const uint16_t *)(afb+1);
	const uint8_t *afx = afb;

	afx += 2; // To be finetuned
	afx += idx;
	afx += idx;
	afx += afxTable[idx];	
	ayFX_afx(afx, segment, priority, adjustedVolume);

	mapper_load_segment(oldSegmentPageD, PAGE_D);    
}

uint8_t ayFX_afb_getNSounds(const uint8_t *afb, uint8_t segment) {
	
	uint8_t oldSegmentPageD = mapper_load_segment(segment, PAGE_D);
	uint8_t nSounds = *afb;
	mapper_load_segment(oldSegmentPageD, PAGE_D);    
	return nSounds;
}

void ayFX_spin(void) {
	
	uint8_t oldSegmentPageD = CURRENT_SEGMENT(PAGE_D);
	uint8_t idx = 0, nAppliedEffects = 0;
	
	uint8_t channel = ayFX_status.channel;
	bool noiseUpdated = false;

	uint8_t r_enable = AY_3_8910_Registers.enable.value;
	
	if (!channel) ayFX_status.channel = 3;
	ayFX_status.channel--;


	while (idx<ayFX_status.nEffects) {		
		
		T_ayFX_Effect *effect = ayFX_effect(idx++);
		int8_t volume;
		T_ayFX_Flags flags;

		//printf("Playing sound %d of %d\n", idx, ayFX_status.nEffects);
		
        mapper_load_segment(effect->segment, PAGE_D);    
		
		flags.value = *effect->afx++;
		
		if (flags.value==0xD0 && *effect->afx==0x20) {

			printf("Removed sound %d of %d\n", idx, ayFX_status.nEffects);
			idx--;
			ayFX_remove(idx);
			continue;
		}
		
		if (flags.changeT) {
			effect->tone = *(const uint16_t *)effect->afx;
			effect->afx += 2;
		}

		if (flags.changeN) {
			effect->noise = *effect->afx++;
		}
		
		volume = flags.volume;
		volume += effect->adjustedVolume;

		if (volume>15) volume=15;
		if (volume< 0) volume=0;
			
		if (volume==0) continue; //No need to write it.
		
		if (flags.disableN && flags.disableT) continue;
		if (noiseUpdated && flags.disableT) continue;

		if (nAppliedEffects>2) continue;
		nAppliedEffects++;

		if (!channel) channel=3;
		channel--;

		AY_3_8910_Registers.amplitude[channel].volume = volume;		
		AY_3_8910_Registers.amplitude[channel].m = 0;		
		
		
		if (!noiseUpdated && !flags.disableN) {
			AY_3_8910_Registers.noise_period = effect->noise;
			noiseUpdated = true;
		}
		
		if (!flags.disableT) {
			AY_3_8910_Registers.tone[channel] = effect->tone;
		}
		
		r_enable &= ~(((~flags.value) & 0x90) >> (4-channel));

		printf("%d, %d, %d, %d, %d, %d, %d\n", 
			AY_3_8910_Registers.amplitude[channel].volume, 
			AY_3_8910_Registers.amplitude[channel].m, 
			AY_3_8910_Registers.noise_period,
			AY_3_8910_Registers.tone[channel],
			flags.value,
			r_enable,
			channel); 
	}
	
	AY_3_8910_Registers.enable.value = r_enable;

	mapper_load_segment(oldSegmentPageD, PAGE_D);    
}


////////////////////////////////////////////////////////////////////////
// PT3 PLAYER


//#endif /* ENABLE_PSG */
