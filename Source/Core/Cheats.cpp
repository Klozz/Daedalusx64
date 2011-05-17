/*
Copyright (C) 2011 StrmnNrmn
Copyright (C) 1999-2004 Joel Middendorf, <schibo@emulation64.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#include "Cheats.h"
#include "Rom.h"
#include "Memory.h"

#include "Utility/IO.h"
#include "OSHLE/ultra_R4300.h"

#include "ConfigOptions.h"

//
// Cheatcode routines based from 1964 and followed PJ64's gameshark format
//

CODEGROUP *codegrouplist;
u32		codegroupcount		= 0;
s32		currentgroupindex	= -1;
char	current_rom_name[128];


#define CHEAT_CODE_MAGIC_VALUE 0xDEAD

enum { CHEAT_ALL_COUNTRY, CHEAT_USA, CHEAT_JAPAN, CHEAT_USA_AND_JAPAN, CHEAT_EUR, CHEAT_AUS, CHEAT_FR, CHEAT_GER };
//*****************************************************************************
//
//*****************************************************************************
static void CheatCodes_Apply(u32 index, u32 mode) 
{
	u32		i;
	u32		address;
	u32		type;
	u16		value;
	bool	executenext = true;

	for (i = 0; i < codegrouplist[index].codecount; i ++) 
	{
		// Used by activator codes, skip until the specified button is pressed
		// OK, skip this code
		//
		if(executenext == false)
		{
			executenext = true;
			continue;
		}

		address = PHYS_TO_K0(codegrouplist[index].codelist[i].addr & 0xFFFFFF);
		value	= codegrouplist[index].codelist[i].val;
		type	= codegrouplist[index].codelist[i].addr & 0xFF000000;

		if( mode == GS_BUTTON )
		{
			switch(type)
			{
			case 0x88000000:			
				Write8Bits(address,(u8)value);
				break;
			case 0x89000000:
				Write16Bits(address, value);
				break;
			}
			break;	
		}
		else
		{
			switch(type)
			//switch(codegrouplist[index].codelist[i].addr / 0x1000000)
			{
			case 0x80000000:
			case 0xA0000000:
				// Check if orig value is unitialized and valid to store current value
				//
				if(codegrouplist[index].codelist[i].orig && (codegrouplist[index].codelist[i].orig == CHEAT_CODE_MAGIC_VALUE))
					codegrouplist[index].codelist[i].orig = Read8Bits(address);
			
				// Cheat code is no longer active, restore to original value
				//
				if(codegrouplist[index].enable==false)
					value = codegrouplist[index].codelist[i].orig;

				Write8Bits(address,(u8)value);
				break;
			case 0x81000000:
			case 0xA1000000:
				// Check if orig value is unitialized and valid to store current value
				//
				if(codegrouplist[index].codelist[i].orig && (codegrouplist[index].codelist[i].orig == CHEAT_CODE_MAGIC_VALUE))
					codegrouplist[index].codelist[i].orig = Read16Bits(address);

				// Cheat code is no longer active, restore to original value
				//
				if(codegrouplist[index].enable==false)
					value = codegrouplist[index].codelist[i].orig;
		
				Write16Bits(address, value);
				break;
			// case 0xD8000000:
			case 0xD0000000:
				if(Read8Bits(address) != value) executenext = false;
				break;
			//case 0xD9000000:
			case 0xD1000000:
				if(Read16Bits(address) != value) executenext = false;
				break;
			case 0xD2000000:
				if(Read8Bits(address) == value) executenext = false;
				break;
			case 0xD3000000:
				if(Read16Bits(address) == value) executenext = false;
				break;
			case 0x50000000:						
				{
					s32	repeatcount = (address & 0x0000FF00) >> 8;
					u32	addroffset	= (address & 0x000000FF);
					u16	valinc		= value;

					if(i + 1 < codegrouplist[index].codecount)
					{
						u32 type	= codegrouplist[index].codelist[i + 1].addr / 0x1000000;
						address		= PHYS_TO_K0(codegrouplist[index].codelist[i + 1].addr & 0x00FFFFFF);
						value		= codegrouplist[index].codelist[i + 1].val;
						u8 valbyte = (u8)value;

						if( type == 0x80 )
						{
							do
							{
								Write8Bits(address,valbyte);
								address += addroffset;
								valbyte += (u8)valinc;
								repeatcount--;
							} while(repeatcount > 0);
						}
						else if( type == 0x81 )
						{
							do
							{
								Write16Bits(address, value);
								address += addroffset;
								value += valinc;
								repeatcount--;
							} while(repeatcount > 0);
						}
					}
				}
				executenext = false;
				break;
			case 0: 
				i = codegrouplist[index].codecount;
				break;
			}
		} 
	}	
}

//*****************************************************************************
//
//*****************************************************************************
void CheatCodes_Activate( CHEAT_MODE mode )
{
	for(u32 i = 0; i < codegroupcount; i++)
	{
		// Apply only enabled cheats
		//
		if(codegrouplist[i].active)
		{
			// Keep track of active cheatcodes, when they are disable, 
			// this flag will signal that we need to restore the hacked value to normal
			//
			codegrouplist[i].enable = true;

			CheatCodes_Apply( i, mode);
		}
		else if(codegrouplist[i].enable)	// If cheat code is no longer disabled, do one pass to restore value
		{
			codegrouplist[i].enable = false;
			CheatCodes_Apply( i, mode);
		}
	}
}

//*****************************************************************************
//
//*****************************************************************************
void CheatCodes_Clear()
{
	for(u32 i = 0; i < codegroupcount; i++) 
	{
		codegrouplist[i].codecount = 0;
	}

	codegroupcount = 0;

	if(codegrouplist != NULL)
	{
		memset(codegrouplist, 0, sizeof(codegrouplist));
	}
}

//*****************************************************************************
// (STRMNNRMN - Strip spaces from end of names)
//*****************************************************************************
static char * tidy(char * s)
{
	if (s == NULL || *s == '\0')
		return s;
	
	char * p = s + strlen(s);

	p--;
	while (p >= s && (*p == ' ' || *p == '\r' || *p == '\n'))
	{
		*p = 0;
		p--;
	}
	return s;
}

//*****************************************************************************
//
//*****************************************************************************
static bool IsCodeMatchRomCountryCode(u32 cheat_country_code, u8 rom_country_code)
{
	//
	// Added by Witten (witten@pj64cheats.net)
	//
	switch (cheat_country_code)
	{
	case CHEAT_ALL_COUNTRY: // all countries
		{
			return true;
		}
	case CHEAT_USA: // USA
		{
			if (rom_country_code == 0x45) 
				return true;
			else
				return false;
		}
	case CHEAT_JAPAN: // JAP
		{
			if (rom_country_code == 0x4A)
				return true;
			else
			    return false;
		}
	case CHEAT_USA_AND_JAPAN: // USA&JAP
		{
			if (rom_country_code == 0x41)
				return true;
			else
			    return false;
		}
	case CHEAT_EUR: // Europe
		{
			switch(rom_country_code)
			{
			case 0x50:
			case 0x58:
			case 0x20:
			case 0x21:
			case 0x38:
			case 0x70:
					return true;
			default:
			    return false;
			}
		}
	case CHEAT_AUS: // Australia
		{
			if (rom_country_code == 0x55 || 0x59)
				return true;
			else
				return false;
		}
		break;
	case CHEAT_FR: // France
		{
			if (rom_country_code == 0x46)
				return true;
			else
				return false;
		}
		break;
	case CHEAT_GER: // Germany
		{
			if (rom_country_code == 0x44)
				return true;
			else
				return false;
		}
	default :
		{
			return false;
		}
		break;
	}
}
//*****************************************************************************
//  I should not need to write such a stupid function to convert String to Int  .
//	However, the sscanf() function does not work for me to input hex number from input string. 
//  I spent some time to debug it, no use, so I wrote  this function to do the converting myself. 
//  Someone could help me to elimiate this function.
//*****************************************************************************
static u32 ConvertHexCharToInt(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	else if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

//*****************************************************************************
//
//*****************************************************************************
static u32 ConvertHexStringToInt(const char *str, int nchars)
{
	int		i;
	u32		result = 0;

	for(i = 0; i < nchars; i++) 
	{
		result = result * 16 + ConvertHexCharToInt(str[i]);
	}
	return result;
}

//*****************************************************************************
//
//*****************************************************************************
bool CheatCodes_Read(char *rom_name, char *file, u8 countryID)
{
	static char		last_rom_name[128];
	char			path[MAX_PATH];
	char			line[2048], romname[256]/*, errormessage[400]*/;	//changed line length to 2048 previous was 256
	bool			bfound;
	u32				c1, c2;
	FILE			*stream;

	strcpy(current_rom_name, rom_name);

	// Do not parse again, if we already parsed for this ROM
	if(strcmp(current_rom_name, last_rom_name) == 0)
	{
		//printf("Cheat file is already parsed for this ROM\n");
		return true;
	}

	strcpy(last_rom_name, current_rom_name);

	// Always clear when parsing a new ROM
	CheatCodes_Clear();

	strcpy(path, gDaedalusExePath);
	strcat(path, file);

	stream = fopen(path, "rt");
	if(stream == NULL)
	{
		// File does not exist, try to create a new empty one
		stream = fopen(path, "wt");

		if(stream == NULL)
		{
			//printf("Cannot find Daedalus.cht file and cannot create it.");
			return false;
		}

		//printf("Cannot find Daedalus.cht file, creating an empty one\n");
		fclose(stream);
		return true;
	}

	// g_ROM.rh.Name adds extra spaces, remove 'em
	//
	tidy( current_rom_name );

	// Locate the entry for current rom by searching for g_ROM.rh.Name
	//
	sprintf(romname, "[%s]", current_rom_name);

	bfound = false;

	while(fgets(line, 256, stream))
	{
		// Remove any extra character that is added at the end of the string
		//
		tidy(line);

		if(strcmp(line, romname) == 0)
		{
			// Found cheatcode entry
			bfound = true;
			break;
		}
		else
		{
			// No match? Keep looking for cheatcode..
			continue;
		}
	}

	if( bfound )
	{
		u32 numberofgroups;

		// First step, read the number of (cheat) groups for the current rom
		//
		if(fgets(line, 256, stream))
		{
			tidy(line);
			if(strncmp(line, "NumberOfGroups=", 15) == 0)
			{
				numberofgroups = atoi(line + 15);
				if( numberofgroups > MAX_CHEATCODE_GROUP_PER_ROM )
				{
					numberofgroups = MAX_CHEATCODE_GROUP_PER_ROM;
				}
			}
			else
			{
				numberofgroups = MAX_CHEATCODE_GROUP_PER_ROM;
			}
		}
		else
		{
			// Auch no number of groups? Cheat must be formated incorrectly
			return false;
		}

		// Allocate memory for groups
		//
		codegrouplist = (CODEGROUP *) malloc(numberofgroups *sizeof(CODEGROUP));
		if(codegrouplist == NULL)
		{
			//printf("Cannot allocate memory to load cheat codes");
			return false;
		}
	
		codegroupcount = 0;
		while(codegroupcount < numberofgroups && fgets(line, 32767, stream) && strlen(line) > 8)	// 32767 makes sure the entire line is read
		{																							
			// Codes for the group are in the string line[]
			for(c1 = 0; line[c1] != '=' && line[c1] != '\0'; c1++) codegrouplist[codegroupcount].name[c1] = line[c1];

			if(codegrouplist[codegroupcount].name[c1 - 2] != ',')
			{
				codegrouplist[codegroupcount].country = 0;
				codegrouplist[codegroupcount].name[c1] = '\0';
			}
			else
			{
				codegrouplist[codegroupcount].country = codegrouplist[codegroupcount].name[c1 - 1] - '0';
				codegrouplist[codegroupcount].name[c1 - 2] = '\0';

				if(IsCodeMatchRomCountryCode(codegrouplist[codegroupcount].country, countryID) == false)
				{
					//printf("Wrong country id %d for cheatcode\n",codegrouplist[codegroupcount].country);
					continue;
				}
			}

			if(line[c1 + 1] == '"')
			{
				// we have a note for this cheat code group
				u32 c3;

				for(c3 = 0; line[c3 + c1 + 2] != '"' && line[c3 + c1 + 2] != '\0'; c3++)
				{
					codegrouplist[codegroupcount].note[c3] = line[c3 + c1 + 2];
				}

				codegrouplist[codegroupcount].note[c3] = '\0';
				c1 = c1 + c3 + 3;
			}
			else
			{
				codegrouplist[codegroupcount].note[0] = '\0';
			}

			codegrouplist[codegroupcount].active = line[c1 + 1] - '0';
			codegrouplist[codegroupcount].enable = false;

			c1 += 2;

			for(c2 = 0; c2 < (strlen(line) - c1 - 1) / 14; c2++, codegrouplist[codegroupcount].codecount++)
			{
				if (c2 < MAX_CHEATCODE_PER_GROUP)
				{
					codegrouplist[codegroupcount].codelist[c2].orig = CHEAT_CODE_MAGIC_VALUE;
					codegrouplist[codegroupcount].codelist[c2].addr = ConvertHexStringToInt(line + c1 + 1 + c2 * 14, 8);
					codegrouplist[codegroupcount].codelist[c2].val = (u16) ConvertHexStringToInt
						(
							line + c1 + 1 + c2 * 14 + 9,
							4
						);
				}
				else
				{
					codegrouplist[codegroupcount].codecount=MAX_CHEATCODE_PER_GROUP;
					/*sprintf (errormessage,
						     "Too many codes for cheat: %s (Max = %d)! Cheat will be truncated and won't work!",
							 codegrouplist[codegroupcount].name,
							 MAX_CHEATCODE_PER_GROUP);
					printf (errormessage);*/
					break;
				}
			}

			codegroupcount++;
		}

		//printf("Succesfully Loaded %d groups of cheat codes\n", codegroupcount);
	}
	else
	{
		// Cannot find entry for the current rom
		//printf("Cannot find entry %d groups of cheat code\n", codegroupcount);
	}

	fclose(stream);
	return true;
}
