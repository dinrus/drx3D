/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file kseed.c
  seed implementation of SEED derived from RFC4269
  Tom St Denis
*/

#include "tomcrypt.h"

#ifdef LTC_KSEED

const struct ltc_cipher_descriptor kseed_desc = {
   "seed",
   20,
   16, 16, 16, 16,
   &kseed_setup,
   &kseed_ecb_encrypt,
   &kseed_ecb_decrypt,
   &kseed_test,
   &kseed_done,
   &kseed_keysize,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static const ulong32 SS0[256] = {
0x2989A1A8UL,0x05858184UL,0x16C6D2D4UL,0x13C3D3D0UL,0x14445054UL,0x1D0D111CUL,0x2C8CA0ACUL,0x25052124UL,
0x1D4D515CUL,0x03434340UL,0x18081018UL,0x1E0E121CUL,0x11415150UL,0x3CCCF0FCUL,0x0ACAC2C8UL,0x23436360UL,
0x28082028UL,0x04444044UL,0x20002020UL,0x1D8D919CUL,0x20C0E0E0UL,0x22C2E2E0UL,0x08C8C0C8UL,0x17071314UL,
0x2585A1A4UL,0x0F8F838CUL,0x03030300UL,0x3B4B7378UL,0x3B8BB3B8UL,0x13031310UL,0x12C2D2D0UL,0x2ECEE2ECUL,
0x30407070UL,0x0C8C808CUL,0x3F0F333CUL,0x2888A0A8UL,0x32023230UL,0x1DCDD1DCUL,0x36C6F2F4UL,0x34447074UL,
0x2CCCE0ECUL,0x15859194UL,0x0B0B0308UL,0x17475354UL,0x1C4C505CUL,0x1B4B5358UL,0x3D8DB1BCUL,0x01010100UL,
0x24042024UL,0x1C0C101CUL,0x33437370UL,0x18889098UL,0x10001010UL,0x0CCCC0CCUL,0x32C2F2F0UL,0x19C9D1D8UL,
0x2C0C202CUL,0x27C7E3E4UL,0x32427270UL,0x03838380UL,0x1B8B9398UL,0x11C1D1D0UL,0x06868284UL,0x09C9C1C8UL,
0x20406060UL,0x10405050UL,0x2383A3A0UL,0x2BCBE3E8UL,0x0D0D010CUL,0x3686B2B4UL,0x1E8E929CUL,0x0F4F434CUL,
0x3787B3B4UL,0x1A4A5258UL,0x06C6C2C4UL,0x38487078UL,0x2686A2A4UL,0x12021210UL,0x2F8FA3ACUL,0x15C5D1D4UL,
0x21416160UL,0x03C3C3C0UL,0x3484B0B4UL,0x01414140UL,0x12425250UL,0x3D4D717CUL,0x0D8D818CUL,0x08080008UL,
0x1F0F131CUL,0x19899198UL,0x00000000UL,0x19091118UL,0x04040004UL,0x13435350UL,0x37C7F3F4UL,0x21C1E1E0UL,
0x3DCDF1FCUL,0x36467274UL,0x2F0F232CUL,0x27072324UL,0x3080B0B0UL,0x0B8B8388UL,0x0E0E020CUL,0x2B8BA3A8UL,
0x2282A2A0UL,0x2E4E626CUL,0x13839390UL,0x0D4D414CUL,0x29496168UL,0x3C4C707CUL,0x09090108UL,0x0A0A0208UL,
0x3F8FB3BCUL,0x2FCFE3ECUL,0x33C3F3F0UL,0x05C5C1C4UL,0x07878384UL,0x14041014UL,0x3ECEF2FCUL,0x24446064UL,
0x1ECED2DCUL,0x2E0E222CUL,0x0B4B4348UL,0x1A0A1218UL,0x06060204UL,0x21012120UL,0x2B4B6368UL,0x26466264UL,
0x02020200UL,0x35C5F1F4UL,0x12829290UL,0x0A8A8288UL,0x0C0C000CUL,0x3383B3B0UL,0x3E4E727CUL,0x10C0D0D0UL,
0x3A4A7278UL,0x07474344UL,0x16869294UL,0x25C5E1E4UL,0x26062224UL,0x00808080UL,0x2D8DA1ACUL,0x1FCFD3DCUL,
0x2181A1A0UL,0x30003030UL,0x37073334UL,0x2E8EA2ACUL,0x36063234UL,0x15051114UL,0x22022220UL,0x38083038UL,
0x34C4F0F4UL,0x2787A3A4UL,0x05454144UL,0x0C4C404CUL,0x01818180UL,0x29C9E1E8UL,0x04848084UL,0x17879394UL,
0x35053134UL,0x0BCBC3C8UL,0x0ECEC2CCUL,0x3C0C303CUL,0x31417170UL,0x11011110UL,0x07C7C3C4UL,0x09898188UL,
0x35457174UL,0x3BCBF3F8UL,0x1ACAD2D8UL,0x38C8F0F8UL,0x14849094UL,0x19495158UL,0x02828280UL,0x04C4C0C4UL,
0x3FCFF3FCUL,0x09494148UL,0x39093138UL,0x27476364UL,0x00C0C0C0UL,0x0FCFC3CCUL,0x17C7D3D4UL,0x3888B0B8UL,
0x0F0F030CUL,0x0E8E828CUL,0x02424240UL,0x23032320UL,0x11819190UL,0x2C4C606CUL,0x1BCBD3D8UL,0x2484A0A4UL,
0x34043034UL,0x31C1F1F0UL,0x08484048UL,0x02C2C2C0UL,0x2F4F636CUL,0x3D0D313CUL,0x2D0D212CUL,0x00404040UL,
0x3E8EB2BCUL,0x3E0E323CUL,0x3C8CB0BCUL,0x01C1C1C0UL,0x2A8AA2A8UL,0x3A8AB2B8UL,0x0E4E424CUL,0x15455154UL,
0x3B0B3338UL,0x1CCCD0DCUL,0x28486068UL,0x3F4F737CUL,0x1C8C909CUL,0x18C8D0D8UL,0x0A4A4248UL,0x16465254UL,
0x37477374UL,0x2080A0A0UL,0x2DCDE1ECUL,0x06464244UL,0x3585B1B4UL,0x2B0B2328UL,0x25456164UL,0x3ACAF2F8UL,
0x23C3E3E0UL,0x3989B1B8UL,0x3181B1B0UL,0x1F8F939CUL,0x1E4E525CUL,0x39C9F1F8UL,0x26C6E2E4UL,0x3282B2B0UL,
0x31013130UL,0x2ACAE2E8UL,0x2D4D616CUL,0x1F4F535CUL,0x24C4E0E4UL,0x30C0F0F0UL,0x0DCDC1CCUL,0x08888088UL,
0x16061214UL,0x3A0A3238UL,0x18485058UL,0x14C4D0D4UL,0x22426260UL,0x29092128UL,0x07070304UL,0x33033330UL,
0x28C8E0E8UL,0x1B0B1318UL,0x05050104UL,0x39497178UL,0x10809090UL,0x2A4A6268UL,0x2A0A2228UL,0x1A8A9298UL
};

static const ulong32 SS1[256] = {
0x38380830UL,0xE828C8E0UL,0x2C2D0D21UL,0xA42686A2UL,0xCC0FCFC3UL,0xDC1ECED2UL,0xB03383B3UL,0xB83888B0UL,
0xAC2F8FA3UL,0x60204060UL,0x54154551UL,0xC407C7C3UL,0x44044440UL,0x6C2F4F63UL,0x682B4B63UL,0x581B4B53UL,
0xC003C3C3UL,0x60224262UL,0x30330333UL,0xB43585B1UL,0x28290921UL,0xA02080A0UL,0xE022C2E2UL,0xA42787A3UL,
0xD013C3D3UL,0x90118191UL,0x10110111UL,0x04060602UL,0x1C1C0C10UL,0xBC3C8CB0UL,0x34360632UL,0x480B4B43UL,
0xEC2FCFE3UL,0x88088880UL,0x6C2C4C60UL,0xA82888A0UL,0x14170713UL,0xC404C4C0UL,0x14160612UL,0xF434C4F0UL,
0xC002C2C2UL,0x44054541UL,0xE021C1E1UL,0xD416C6D2UL,0x3C3F0F33UL,0x3C3D0D31UL,0x8C0E8E82UL,0x98188890UL,
0x28280820UL,0x4C0E4E42UL,0xF436C6F2UL,0x3C3E0E32UL,0xA42585A1UL,0xF839C9F1UL,0x0C0D0D01UL,0xDC1FCFD3UL,
0xD818C8D0UL,0x282B0B23UL,0x64264662UL,0x783A4A72UL,0x24270723UL,0x2C2F0F23UL,0xF031C1F1UL,0x70324272UL,
0x40024242UL,0xD414C4D0UL,0x40014141UL,0xC000C0C0UL,0x70334373UL,0x64274763UL,0xAC2C8CA0UL,0x880B8B83UL,
0xF437C7F3UL,0xAC2D8DA1UL,0x80008080UL,0x1C1F0F13UL,0xC80ACAC2UL,0x2C2C0C20UL,0xA82A8AA2UL,0x34340430UL,
0xD012C2D2UL,0x080B0B03UL,0xEC2ECEE2UL,0xE829C9E1UL,0x5C1D4D51UL,0x94148490UL,0x18180810UL,0xF838C8F0UL,
0x54174753UL,0xAC2E8EA2UL,0x08080800UL,0xC405C5C1UL,0x10130313UL,0xCC0DCDC1UL,0x84068682UL,0xB83989B1UL,
0xFC3FCFF3UL,0x7C3D4D71UL,0xC001C1C1UL,0x30310131UL,0xF435C5F1UL,0x880A8A82UL,0x682A4A62UL,0xB03181B1UL,
0xD011C1D1UL,0x20200020UL,0xD417C7D3UL,0x00020202UL,0x20220222UL,0x04040400UL,0x68284860UL,0x70314171UL,
0x04070703UL,0xD81BCBD3UL,0x9C1D8D91UL,0x98198991UL,0x60214161UL,0xBC3E8EB2UL,0xE426C6E2UL,0x58194951UL,
0xDC1DCDD1UL,0x50114151UL,0x90108090UL,0xDC1CCCD0UL,0x981A8A92UL,0xA02383A3UL,0xA82B8BA3UL,0xD010C0D0UL,
0x80018181UL,0x0C0F0F03UL,0x44074743UL,0x181A0A12UL,0xE023C3E3UL,0xEC2CCCE0UL,0x8C0D8D81UL,0xBC3F8FB3UL,
0x94168692UL,0x783B4B73UL,0x5C1C4C50UL,0xA02282A2UL,0xA02181A1UL,0x60234363UL,0x20230323UL,0x4C0D4D41UL,
0xC808C8C0UL,0x9C1E8E92UL,0x9C1C8C90UL,0x383A0A32UL,0x0C0C0C00UL,0x2C2E0E22UL,0xB83A8AB2UL,0x6C2E4E62UL,
0x9C1F8F93UL,0x581A4A52UL,0xF032C2F2UL,0x90128292UL,0xF033C3F3UL,0x48094941UL,0x78384870UL,0xCC0CCCC0UL,
0x14150511UL,0xF83BCBF3UL,0x70304070UL,0x74354571UL,0x7C3F4F73UL,0x34350531UL,0x10100010UL,0x00030303UL,
0x64244460UL,0x6C2D4D61UL,0xC406C6C2UL,0x74344470UL,0xD415C5D1UL,0xB43484B0UL,0xE82ACAE2UL,0x08090901UL,
0x74364672UL,0x18190911UL,0xFC3ECEF2UL,0x40004040UL,0x10120212UL,0xE020C0E0UL,0xBC3D8DB1UL,0x04050501UL,
0xF83ACAF2UL,0x00010101UL,0xF030C0F0UL,0x282A0A22UL,0x5C1E4E52UL,0xA82989A1UL,0x54164652UL,0x40034343UL,
0x84058581UL,0x14140410UL,0x88098981UL,0x981B8B93UL,0xB03080B0UL,0xE425C5E1UL,0x48084840UL,0x78394971UL,
0x94178793UL,0xFC3CCCF0UL,0x1C1E0E12UL,0x80028282UL,0x20210121UL,0x8C0C8C80UL,0x181B0B13UL,0x5C1F4F53UL,
0x74374773UL,0x54144450UL,0xB03282B2UL,0x1C1D0D11UL,0x24250521UL,0x4C0F4F43UL,0x00000000UL,0x44064642UL,
0xEC2DCDE1UL,0x58184850UL,0x50124252UL,0xE82BCBE3UL,0x7C3E4E72UL,0xD81ACAD2UL,0xC809C9C1UL,0xFC3DCDF1UL,
0x30300030UL,0x94158591UL,0x64254561UL,0x3C3C0C30UL,0xB43686B2UL,0xE424C4E0UL,0xB83B8BB3UL,0x7C3C4C70UL,
0x0C0E0E02UL,0x50104050UL,0x38390931UL,0x24260622UL,0x30320232UL,0x84048480UL,0x68294961UL,0x90138393UL,
0x34370733UL,0xE427C7E3UL,0x24240420UL,0xA42484A0UL,0xC80BCBC3UL,0x50134353UL,0x080A0A02UL,0x84078783UL,
0xD819C9D1UL,0x4C0C4C40UL,0x80038383UL,0x8C0F8F83UL,0xCC0ECEC2UL,0x383B0B33UL,0x480A4A42UL,0xB43787B3UL
};

static const ulong32 SS2[256] = {
0xA1A82989UL,0x81840585UL,0xD2D416C6UL,0xD3D013C3UL,0x50541444UL,0x111C1D0DUL,0xA0AC2C8CUL,0x21242505UL,
0x515C1D4DUL,0x43400343UL,0x10181808UL,0x121C1E0EUL,0x51501141UL,0xF0FC3CCCUL,0xC2C80ACAUL,0x63602343UL,
0x20282808UL,0x40440444UL,0x20202000UL,0x919C1D8DUL,0xE0E020C0UL,0xE2E022C2UL,0xC0C808C8UL,0x13141707UL,
0xA1A42585UL,0x838C0F8FUL,0x03000303UL,0x73783B4BUL,0xB3B83B8BUL,0x13101303UL,0xD2D012C2UL,0xE2EC2ECEUL,
0x70703040UL,0x808C0C8CUL,0x333C3F0FUL,0xA0A82888UL,0x32303202UL,0xD1DC1DCDUL,0xF2F436C6UL,0x70743444UL,
0xE0EC2CCCUL,0x91941585UL,0x03080B0BUL,0x53541747UL,0x505C1C4CUL,0x53581B4BUL,0xB1BC3D8DUL,0x01000101UL,
0x20242404UL,0x101C1C0CUL,0x73703343UL,0x90981888UL,0x10101000UL,0xC0CC0CCCUL,0xF2F032C2UL,0xD1D819C9UL,
0x202C2C0CUL,0xE3E427C7UL,0x72703242UL,0x83800383UL,0x93981B8BUL,0xD1D011C1UL,0x82840686UL,0xC1C809C9UL,
0x60602040UL,0x50501040UL,0xA3A02383UL,0xE3E82BCBUL,0x010C0D0DUL,0xB2B43686UL,0x929C1E8EUL,0x434C0F4FUL,
0xB3B43787UL,0x52581A4AUL,0xC2C406C6UL,0x70783848UL,0xA2A42686UL,0x12101202UL,0xA3AC2F8FUL,0xD1D415C5UL,
0x61602141UL,0xC3C003C3UL,0xB0B43484UL,0x41400141UL,0x52501242UL,0x717C3D4DUL,0x818C0D8DUL,0x00080808UL,
0x131C1F0FUL,0x91981989UL,0x00000000UL,0x11181909UL,0x00040404UL,0x53501343UL,0xF3F437C7UL,0xE1E021C1UL,
0xF1FC3DCDUL,0x72743646UL,0x232C2F0FUL,0x23242707UL,0xB0B03080UL,0x83880B8BUL,0x020C0E0EUL,0xA3A82B8BUL,
0xA2A02282UL,0x626C2E4EUL,0x93901383UL,0x414C0D4DUL,0x61682949UL,0x707C3C4CUL,0x01080909UL,0x02080A0AUL,
0xB3BC3F8FUL,0xE3EC2FCFUL,0xF3F033C3UL,0xC1C405C5UL,0x83840787UL,0x10141404UL,0xF2FC3ECEUL,0x60642444UL,
0xD2DC1ECEUL,0x222C2E0EUL,0x43480B4BUL,0x12181A0AUL,0x02040606UL,0x21202101UL,0x63682B4BUL,0x62642646UL,
0x02000202UL,0xF1F435C5UL,0x92901282UL,0x82880A8AUL,0x000C0C0CUL,0xB3B03383UL,0x727C3E4EUL,0xD0D010C0UL,
0x72783A4AUL,0x43440747UL,0x92941686UL,0xE1E425C5UL,0x22242606UL,0x80800080UL,0xA1AC2D8DUL,0xD3DC1FCFUL,
0xA1A02181UL,0x30303000UL,0x33343707UL,0xA2AC2E8EUL,0x32343606UL,0x11141505UL,0x22202202UL,0x30383808UL,
0xF0F434C4UL,0xA3A42787UL,0x41440545UL,0x404C0C4CUL,0x81800181UL,0xE1E829C9UL,0x80840484UL,0x93941787UL,
0x31343505UL,0xC3C80BCBUL,0xC2CC0ECEUL,0x303C3C0CUL,0x71703141UL,0x11101101UL,0xC3C407C7UL,0x81880989UL,
0x71743545UL,0xF3F83BCBUL,0xD2D81ACAUL,0xF0F838C8UL,0x90941484UL,0x51581949UL,0x82800282UL,0xC0C404C4UL,
0xF3FC3FCFUL,0x41480949UL,0x31383909UL,0x63642747UL,0xC0C000C0UL,0xC3CC0FCFUL,0xD3D417C7UL,0xB0B83888UL,
0x030C0F0FUL,0x828C0E8EUL,0x42400242UL,0x23202303UL,0x91901181UL,0x606C2C4CUL,0xD3D81BCBUL,0xA0A42484UL,
0x30343404UL,0xF1F031C1UL,0x40480848UL,0xC2C002C2UL,0x636C2F4FUL,0x313C3D0DUL,0x212C2D0DUL,0x40400040UL,
0xB2BC3E8EUL,0x323C3E0EUL,0xB0BC3C8CUL,0xC1C001C1UL,0xA2A82A8AUL,0xB2B83A8AUL,0x424C0E4EUL,0x51541545UL,
0x33383B0BUL,0xD0DC1CCCUL,0x60682848UL,0x737C3F4FUL,0x909C1C8CUL,0xD0D818C8UL,0x42480A4AUL,0x52541646UL,
0x73743747UL,0xA0A02080UL,0xE1EC2DCDUL,0x42440646UL,0xB1B43585UL,0x23282B0BUL,0x61642545UL,0xF2F83ACAUL,
0xE3E023C3UL,0xB1B83989UL,0xB1B03181UL,0x939C1F8FUL,0x525C1E4EUL,0xF1F839C9UL,0xE2E426C6UL,0xB2B03282UL,
0x31303101UL,0xE2E82ACAUL,0x616C2D4DUL,0x535C1F4FUL,0xE0E424C4UL,0xF0F030C0UL,0xC1CC0DCDUL,0x80880888UL,
0x12141606UL,0x32383A0AUL,0x50581848UL,0xD0D414C4UL,0x62602242UL,0x21282909UL,0x03040707UL,0x33303303UL,
0xE0E828C8UL,0x13181B0BUL,0x01040505UL,0x71783949UL,0x90901080UL,0x62682A4AUL,0x22282A0AUL,0x92981A8AUL
};

static const ulong32 SS3[256] = {
0x08303838UL,0xC8E0E828UL,0x0D212C2DUL,0x86A2A426UL,0xCFC3CC0FUL,0xCED2DC1EUL,0x83B3B033UL,0x88B0B838UL,
0x8FA3AC2FUL,0x40606020UL,0x45515415UL,0xC7C3C407UL,0x44404404UL,0x4F636C2FUL,0x4B63682BUL,0x4B53581BUL,
0xC3C3C003UL,0x42626022UL,0x03333033UL,0x85B1B435UL,0x09212829UL,0x80A0A020UL,0xC2E2E022UL,0x87A3A427UL,
0xC3D3D013UL,0x81919011UL,0x01111011UL,0x06020406UL,0x0C101C1CUL,0x8CB0BC3CUL,0x06323436UL,0x4B43480BUL,
0xCFE3EC2FUL,0x88808808UL,0x4C606C2CUL,0x88A0A828UL,0x07131417UL,0xC4C0C404UL,0x06121416UL,0xC4F0F434UL,
0xC2C2C002UL,0x45414405UL,0xC1E1E021UL,0xC6D2D416UL,0x0F333C3FUL,0x0D313C3DUL,0x8E828C0EUL,0x88909818UL,
0x08202828UL,0x4E424C0EUL,0xC6F2F436UL,0x0E323C3EUL,0x85A1A425UL,0xC9F1F839UL,0x0D010C0DUL,0xCFD3DC1FUL,
0xC8D0D818UL,0x0B23282BUL,0x46626426UL,0x4A72783AUL,0x07232427UL,0x0F232C2FUL,0xC1F1F031UL,0x42727032UL,
0x42424002UL,0xC4D0D414UL,0x41414001UL,0xC0C0C000UL,0x43737033UL,0x47636427UL,0x8CA0AC2CUL,0x8B83880BUL,
0xC7F3F437UL,0x8DA1AC2DUL,0x80808000UL,0x0F131C1FUL,0xCAC2C80AUL,0x0C202C2CUL,0x8AA2A82AUL,0x04303434UL,
0xC2D2D012UL,0x0B03080BUL,0xCEE2EC2EUL,0xC9E1E829UL,0x4D515C1DUL,0x84909414UL,0x08101818UL,0xC8F0F838UL,
0x47535417UL,0x8EA2AC2EUL,0x08000808UL,0xC5C1C405UL,0x03131013UL,0xCDC1CC0DUL,0x86828406UL,0x89B1B839UL,
0xCFF3FC3FUL,0x4D717C3DUL,0xC1C1C001UL,0x01313031UL,0xC5F1F435UL,0x8A82880AUL,0x4A62682AUL,0x81B1B031UL,
0xC1D1D011UL,0x00202020UL,0xC7D3D417UL,0x02020002UL,0x02222022UL,0x04000404UL,0x48606828UL,0x41717031UL,
0x07030407UL,0xCBD3D81BUL,0x8D919C1DUL,0x89919819UL,0x41616021UL,0x8EB2BC3EUL,0xC6E2E426UL,0x49515819UL,
0xCDD1DC1DUL,0x41515011UL,0x80909010UL,0xCCD0DC1CUL,0x8A92981AUL,0x83A3A023UL,0x8BA3A82BUL,0xC0D0D010UL,
0x81818001UL,0x0F030C0FUL,0x47434407UL,0x0A12181AUL,0xC3E3E023UL,0xCCE0EC2CUL,0x8D818C0DUL,0x8FB3BC3FUL,
0x86929416UL,0x4B73783BUL,0x4C505C1CUL,0x82A2A022UL,0x81A1A021UL,0x43636023UL,0x03232023UL,0x4D414C0DUL,
0xC8C0C808UL,0x8E929C1EUL,0x8C909C1CUL,0x0A32383AUL,0x0C000C0CUL,0x0E222C2EUL,0x8AB2B83AUL,0x4E626C2EUL,
0x8F939C1FUL,0x4A52581AUL,0xC2F2F032UL,0x82929012UL,0xC3F3F033UL,0x49414809UL,0x48707838UL,0xCCC0CC0CUL,
0x05111415UL,0xCBF3F83BUL,0x40707030UL,0x45717435UL,0x4F737C3FUL,0x05313435UL,0x00101010UL,0x03030003UL,
0x44606424UL,0x4D616C2DUL,0xC6C2C406UL,0x44707434UL,0xC5D1D415UL,0x84B0B434UL,0xCAE2E82AUL,0x09010809UL,
0x46727436UL,0x09111819UL,0xCEF2FC3EUL,0x40404000UL,0x02121012UL,0xC0E0E020UL,0x8DB1BC3DUL,0x05010405UL,
0xCAF2F83AUL,0x01010001UL,0xC0F0F030UL,0x0A22282AUL,0x4E525C1EUL,0x89A1A829UL,0x46525416UL,0x43434003UL,
0x85818405UL,0x04101414UL,0x89818809UL,0x8B93981BUL,0x80B0B030UL,0xC5E1E425UL,0x48404808UL,0x49717839UL,
0x87939417UL,0xCCF0FC3CUL,0x0E121C1EUL,0x82828002UL,0x01212021UL,0x8C808C0CUL,0x0B13181BUL,0x4F535C1FUL,
0x47737437UL,0x44505414UL,0x82B2B032UL,0x0D111C1DUL,0x05212425UL,0x4F434C0FUL,0x00000000UL,0x46424406UL,
0xCDE1EC2DUL,0x48505818UL,0x42525012UL,0xCBE3E82BUL,0x4E727C3EUL,0xCAD2D81AUL,0xC9C1C809UL,0xCDF1FC3DUL,
0x00303030UL,0x85919415UL,0x45616425UL,0x0C303C3CUL,0x86B2B436UL,0xC4E0E424UL,0x8BB3B83BUL,0x4C707C3CUL,
0x0E020C0EUL,0x40505010UL,0x09313839UL,0x06222426UL,0x02323032UL,0x84808404UL,0x49616829UL,0x83939013UL,
0x07333437UL,0xC7E3E427UL,0x04202424UL,0x84A0A424UL,0xCBC3C80BUL,0x43535013UL,0x0A02080AUL,0x87838407UL,
0xC9D1D819UL,0x4C404C0CUL,0x83838003UL,0x8F838C0FUL,0xCEC2CC0EUL,0x0B33383BUL,0x4A42480AUL,0x87B3B437UL
};

static const ulong32 KCi[16] = {
0x9E3779B9,0x3C6EF373,
0x78DDE6E6,0xF1BBCDCC,
0xE3779B99,0xC6EF3733,
0x8DDE6E67,0x1BBCDCCF,
0x3779B99E,0x6EF3733C,
0xDDE6E678,0xBBCDCCF1,
0x779B99E3,0xEF3733C6,
0xDE6E678D,0xBCDCCF1B
};

#define G(x) (SS3[((x)>>24)&255] ^ SS2[((x)>>16)&255] ^ SS1[((x)>>8)&255] ^ SS0[(x)&255])

#define F(L1, L2, R1, R2, K1, K2) \
   T2 = G((R1 ^ K1) ^ (R2 ^ K2)); \
   T = G( G(T2 + (R1 ^ K1)) + T2); \
   L2 ^= T; \
   L1 ^= (T + G(T2 + (R1 ^ K1))); \

 /**
    Initialize the SEED block cipher
    @param key The symmetric key you wish to pass
    @param keylen The key length in bytes
    @param num_rounds The number of rounds desired (0 for default)
    @param skey The key in as scheduled by this function.
    @return CRYPT_OK if successful
 */
i32 kseed_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey)
{
   i32     i;
   ulong32 tmp, k1, k2, k3, k4;

   if (keylen != 16) {
      return CRYPT_INVALID_KEYSIZE;
   }

   if (num_rounds != 16 && num_rounds != 0) {
      return CRYPT_INVALID_ROUNDS;
   }

   /* load key */
   LOAD32H(k1, key);
   LOAD32H(k2, key+4);
   LOAD32H(k3, key+8);
   LOAD32H(k4, key+12);

   for (i = 0; i < 16; i++) {
      skey->kseed.K[2*i+0] = G(k1 + k3 - KCi[i]);
      skey->kseed.K[2*i+1] = G(k2 - k4 + KCi[i]);
      if (i&1) {
         tmp = k3;
         k3 = ((k3 << 8) | (k4 >> 24)) & 0xFFFFFFFF;
         k4 = ((k4 << 8) | (tmp >> 24)) & 0xFFFFFFFF;
      } else {
         tmp = k1;
         k1 = ((k1 >> 8) | (k2 << 24)) & 0xFFFFFFFF;
         k2 = ((k2 >> 8) | (tmp << 24)) & 0xFFFFFFFF;
      }
      /* reverse keys for decrypt */
      skey->kseed.dK[2*(15-i)+0] = skey->kseed.K[2*i+0];
      skey->kseed.dK[2*(15-i)+1] = skey->kseed.K[2*i+1];
   }

   return CRYPT_OK;
}

static void rounds(ulong32 *P, ulong32 *K)
{
   ulong32 T, T2;
   i32     i;
   for (i = 0; i < 16; i += 2) {
     F(P[0], P[1], P[2], P[3], K[0], K[1]);
     F(P[2], P[3], P[0], P[1], K[2], K[3]);
     K += 4;
   }
}

/**
  Encrypts a block of text with SEED
  @param pt The input plaintext (16 bytes)
  @param ct The output ciphertext (16 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
i32 kseed_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey)
{
   ulong32 P[4];
   LOAD32H(P[0], pt);
   LOAD32H(P[1], pt+4);
   LOAD32H(P[2], pt+8);
   LOAD32H(P[3], pt+12);
   rounds(P, skey->kseed.K);
   STORE32H(P[2], ct);
   STORE32H(P[3], ct+4);
   STORE32H(P[0], ct+8);
   STORE32H(P[1], ct+12);
   return CRYPT_OK;
}

/**
  Decrypts a block of text with SEED
  @param ct The input ciphertext (16 bytes)
  @param pt The output plaintext (16 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
i32 kseed_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey)
{
   ulong32 P[4];
   LOAD32H(P[0], ct);
   LOAD32H(P[1], ct+4);
   LOAD32H(P[2], ct+8);
   LOAD32H(P[3], ct+12);
   rounds(P, skey->kseed.dK);
   STORE32H(P[2], pt);
   STORE32H(P[3], pt+4);
   STORE32H(P[0], pt+8);
   STORE32H(P[1], pt+12);
   return CRYPT_OK;
}

/** Terminate the context
   @param skey    The scheduled key
*/
void kseed_done(symmetric_key *skey)
{
  LTC_UNUSED_PARAM(skey);
}

/**
  Performs a self-test of the SEED block cipher
  @return CRYPT_OK if functional, CRYPT_NOP if self-test has been disabled
*/
i32 kseed_test(void)
{
#if !defined(LTC_TEST)
  return CRYPT_NOP;
#else
  static const struct test {
     u8 pt[16], ct[16], key[16];
  } tests[] = {

{
  { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F },
  { 0x5E,0xBA,0xC6,0xE0,0x05,0x4E,0x16,0x68,0x19,0xAF,0xF1,0xCC,0x6D,0x34,0x6C,0xDB },
  { 0 },
},

{
  { 0 },
  { 0xC1,0x1F,0x22,0xF2,0x01,0x40,0x50,0x50,0x84,0x48,0x35,0x97,0xE4,0x37,0x0F,0x43 },
  { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F },
},

{
  { 0x83,0xA2,0xF8,0xA2,0x88,0x64,0x1F,0xB9,0xA4,0xE9,0xA5,0xCC,0x2F,0x13,0x1C,0x7D },
  { 0xEE,0x54,0xD1,0x3E,0xBC,0xAE,0x70,0x6D,0x22,0x6B,0xC3,0x14,0x2C,0xD4,0x0D,0x4A },
  { 0x47,0x06,0x48,0x08,0x51,0xE6,0x1B,0xE8,0x5D,0x74,0xBF,0xB3,0xFD,0x95,0x61,0x85 },
},

{
  { 0xB4,0x1E,0x6B,0xE2,0xEB,0xA8,0x4A,0x14,0x8E,0x2E,0xED,0x84,0x59,0x3C,0x5E,0xC7 },
  { 0x9B,0x9B,0x7B,0xFC,0xD1,0x81,0x3C,0xB9,0x5D,0x0B,0x36,0x18,0xF4,0x0F,0x51,0x22 },
  { 0x28,0xDB,0xC3,0xBC,0x49,0xFF,0xD8,0x7D,0xCF,0xA5,0x09,0xB1,0x1D,0x42,0x2B,0xE7 },
}
};
   i32 x;
   u8 buf[2][16];
   symmetric_key skey;

   for (x = 0; x < (i32)(sizeof(tests)/sizeof(tests[0])); x++) {
       kseed_setup(tests[x].key, 16, 0, &skey);
       kseed_ecb_encrypt(tests[x].pt, buf[0], &skey);
       kseed_ecb_decrypt(buf[0], buf[1], &skey);
       if (compare_testvector(buf[0], 16, tests[x].ct, 16, "KSEED Encrypt", x) ||
             compare_testvector(buf[1], 16, tests[x].pt, 16, "KSEED Decrypt", x)) {
          return CRYPT_FAIL_TESTVECTOR;
       }
   }
   return CRYPT_OK;
#endif
}

/**
  Gets suitable key size
  @param keysize [in/out] The length of the recommended key (in bytes).  This function will store the suitable size back in this variable.
  @return CRYPT_OK if the input key size is acceptable.
*/
i32 kseed_keysize(i32 *keysize)
{
   LTC_ARGCHK(keysize != NULL);
   if (*keysize >= 16) {
      *keysize = 16;
   } else {
      return CRYPT_INVALID_KEYSIZE;
   }
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
