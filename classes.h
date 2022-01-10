#pragma once

#include <Windows.h>
#include <inttypes.h>
#include <utility>

typedef Vector3 v3;

/*
struct netPlayerData
{
	char pad_0000[8]; //0x0000
	uint64_t rockstar_id_0; //0x0008
	char pad_0010[52]; //0x0010
	uint64_t relay_ip; //0x0044
	uint16_t relay_port; //0x0048
	char pad_004A[2]; //0x004A
	uint64_t external_ip; //0x004C
	uint16_t external_port; //0x0050
	char pad_0052[2]; //0x0052
	uint64_t internal_ip; //0x0054
	uint16_t internal_port; //0x0058
	char pad_005A[6]; //0x005A
	uint64_t host_token; //0x0060
	char pad_0068[8]; //0x0068
	uint64_t rockstar_id; //0x0070
	char pad_0078[12]; //0x0078
	char name[16 + 1]; //0x0084
	char pad_0095[3]; //0x0095
}; //Size: 0x0098
*/

class netPlayerData
{
public:
	char pad_0000[8]; //0x0000
	uint64_t m_rockstar_id_0; //0x0008
	char pad_0010[56]; //0x0010
	uint16_t N000005BF; //0x0048
	char pad_004A[2]; //0x004A
	uint32_t m_relay_ip; //0x004C
	uint32_t m_relay_port; //0x0050
	uint32_t m_online_ip; //0x0054
	uint16_t m_online_port; //0x0058
	char pad_005A[6]; //0x005A
	uint32_t m_host_token; //0x0060
	char pad_0064[12]; //0x0064
	uint64_t m_rockstar_id; //0x0070
	char pad_0078[12]; //0x0078
	char m_name[20]; //0x0084
}; //Size: 0x0098

static const Hash AmmoTypes[]
{
	0x743D4F54,
	0x6C7D23B8,
	0xD05319F,
	0x6AA1343F,
	0x90083D3B,
	0xB02EADE0,
	0x4C98087B,
	0xFEDA7D30,
	0x3BCCA5EE,
	0x313FD340,
	0x67DD81F2,
	0x914C813A,
	0x9FC5C882,
	0x3BD313B1,
	0x5424B617,
	0xE60E08A6,
	0x9B747EA4,
	0x5633F9D5,
	0x5106B43C,
	0xCA6318A1,
	0xFF956666,
	0x6BCCF76F,
	0xA81B4220,
	0x1F75106C,
	0x47735976,
	0xF624D80A,
	0xAE2EA48C,
	0x4298C094,
	0x155663F8,
	0x99150E2D,
	0xAF2208A7,
	0x8218416D,
	0x1941D244,
	0x5E962DDC,
	0x92F129CD,
	0xB0198D5F,
	0xA6BCBDA9,
	0xF5F1C616,
	0x2F7CA4A6,
	0x469293CD,
	0x72A3A760,
	0xED906955,
	0x7C867272,
	0xDBACD794,
	0xBC7AF403,
	0xCE23B916,
	0xAB8EA0F9,
	0xB8DCEE2B,
	0x2EC80A10,
	0xDFD80B5,
	0x57237470,
	0x4919B4EB,
	0xADD16CB9,
	0x2D31ADD9,
	0x27F43E92,
	0xEC2875E7,
	0x5D9106D1,
	0x45F0E965,
	0xAF23EE0F,
	0x794446FD,
};

enum eWeaponGroup
{
	wpPISTOL,
	wpMG,
	wpRIFLE,
	wpSNIPER,
	wpMELEE,
	wpSHOTGUN,
	wpHEAVY,
	wpSPECIAL,
};

static Hash wp_melees[] =
{
	0x92A27487, 0x958A4A8F, 0xF9E6AA4B, 0x84BD7BFD,0x8BB05FD7,
	0x440E4788, 0x4E875F73, 0xF9DCBF2D, 0xD8DF3C3C, 0x99B507EA,
	0xDD5DF8D9, 0xDFE37640, 0x678B81B1, 0x19044EE0, 0xCD274149,
	0x94117305, 0x3813FC08, 0
};

static Hash wp_pistols[] =
{
	0x1B06D571, 0xBFE256D4, 0x5EF9FEC4, 0x22D8FE39, 0x3656C8C1,
	0x99AEEB3B, 0xBFD21232, 0x88374054, 0xD205520E, 0x83839C4,
	0x47757124, 0xDC4DB296, 0xC1B3C3D1, 0xCB96392F, 0x97EA20B8,
	0xAF3696A1, 0x2B5EF5EC, 0x917F6C8C, 0x57A4368C, 0
};

static Hash wp_mgs[] =
{
	0x13532244, 0x2BE6766B, 0x78A97CD0, 0xEFE7E2DF, 0x0A3D4D34,
	0xDB1AA450, 0xBD248B55, 0x476BF155, 0x9D07F764, 0x7FD62962,
	0xDBBD7280, 0x61012683, 0
};

static Hash wp_shotguns[] =
{
	0x1D073A89, 0x555AF99A, 0x7846A318, 0xE284C527, 0x9D61E50F,
	0xA89CB99E, 0x3AABBBAA, 0xEF951FBB, 0x12E82D3D, 0x5A96BA4, 0
};

static Hash wp_rifles[] =
{
	0xBFEFFF6D, 0x394F415C, 0x83BF0278, 0xFAD1F1C9, 0xAF113F99,
	0xC0A3098D, 0x969C3D67, 0x7F229F94, 0x84D6FAFD, 0x624FE830,
	0x9D1F17E6, 0
};

static Hash wp_snipers[] = { 0x05FC3C11, 0x0C472FE2, 0xA914799, 0xC734385A, 0x6A6C02E0 , 0 };
static Hash wp_heavys[] =
{
	0xB1CA77B1, 0xA284510B, 0x4DD2DC56, 0x42BF8A85, 0x7F7497E5,
	0x6D544C99, 0x63AB0442, 0x0781FE4A, 0xB62D1F67, 0
};

static Hash wp_specials[] =
{
	0x93E220BD, 0xA0973D5E, 0x24B17070, 0x2C3731D9, 0xAB564B93,
	0x787F0BB, 0xBA45E8B8, 0x23C9F95C, 0xFDBC8A50, 0x497FACC3,
	0x34A67B97, 0xFBAB5776, 0x060EC506, 0xBA536372, 0
};

class CColor
{
public:
	BYTE r, g, b, a;

	CColor& swap_r_b()
	{
		std::swap(r, b);
		return *this;
	}
};

class CTextInfo
{
public:
	CColor	color;
	float	text_scale;
	float	text_scale2;
	float	wrap_start;
	float	wrap_end;
	int		font;
	uint8_t pad[0x4];
	WORD	justification;

	void setColor(CColor c)
	{
		c.swap_r_b();
		color = c;
	}

	void setScale(float f)
	{
		text_scale = f;
		text_scale2 = f;
	}
};