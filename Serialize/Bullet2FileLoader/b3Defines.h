#ifndef __B_DEFINES_H__
#define __B_DEFINES_H__

// MISC defines, see BKE_global.h, BKE_utildefines.h
#define D3_SIZEOFBLENDERHEADER 12

// ------------------------------------------------------------
#if defined(__sgi) || defined(__sparc) || defined(__sparc__) || defined(__PPC__) || defined(__ppc__) || defined(__BIG_ENDIAN__)
#define D3_MAKE_ID(a, b, c, d) ((i32)(a) << 24 | (i32)(b) << 16 | (c) << 8 | (d))
#else
#define D3_MAKE_ID(a, b, c, d) ((i32)(d) << 24 | (i32)(c) << 16 | (b) << 8 | (a))
#endif

// ------------------------------------------------------------
#if defined(__sgi) || defined(__sparc) || defined(__sparc__) || defined(__PPC__) || defined(__ppc__) || defined(__BIG_ENDIAN__)
#define D3_MAKE_ID2(c, d) ((c) << 8 | (d))
#else
#define D3_MAKE_ID2(c, d) ((d) << 8 | (c))
#endif

// ------------------------------------------------------------
#define D3_ID_SCE D3_MAKE_ID2('S', 'C')
#define D3_ID_LI D3_MAKE_ID2('L', 'I')
#define D3_ID_OB D3_MAKE_ID2('O', 'B')
#define D3_ID_ME D3_MAKE_ID2('M', 'E')
#define D3_ID_CU D3_MAKE_ID2('C', 'U')
#define D3_ID_MB D3_MAKE_ID2('M', 'B')
#define D3_ID_MA D3_MAKE_ID2('M', 'A')
#define D3_ID_TE D3_MAKE_ID2('T', 'E')
#define D3_ID_IM D3_MAKE_ID2('I', 'M')
#define D3_ID_IK D3_MAKE_ID2('I', 'K')
#define D3_ID_WV D3_MAKE_ID2('W', 'V')
#define D3_ID_LT D3_MAKE_ID2('L', 'T')
#define D3_ID_SE D3_MAKE_ID2('S', 'E')
#define D3_ID_LF D3_MAKE_ID2('L', 'F')
#define D3_ID_LA D3_MAKE_ID2('L', 'A')
#define D3_ID_CA D3_MAKE_ID2('C', 'A')
#define D3_ID_IP D3_MAKE_ID2('I', 'P')
#define D3_ID_KE D3_MAKE_ID2('K', 'E')
#define D3_ID_WO D3_MAKE_ID2('W', 'O')
#define D3_ID_SCR D3_MAKE_ID2('S', 'R')
#define D3_ID_VF D3_MAKE_ID2('V', 'F')
#define D3_ID_TXT D3_MAKE_ID2('T', 'X')
#define D3_ID_SO D3_MAKE_ID2('S', 'O')
#define D3_ID_SAMPLE D3_MAKE_ID2('S', 'A')
#define D3_ID_GR D3_MAKE_ID2('G', 'R')
#define D3_ID_ID D3_MAKE_ID2('I', 'D')
#define D3_ID_AR D3_MAKE_ID2('A', 'R')
#define D3_ID_AC D3_MAKE_ID2('A', 'C')
#define D3_ID_SCRIPT D3_MAKE_ID2('P', 'Y')
#define D3_ID_FLUIDSIM D3_MAKE_ID2('F', 'S')
#define D3_ID_NT D3_MAKE_ID2('N', 'T')
#define D3_ID_BR D3_MAKE_ID2('B', 'R')

#define D3_ID_SEQ D3_MAKE_ID2('S', 'Q')
#define D3_ID_CO D3_MAKE_ID2('C', 'O')
#define D3_ID_PO D3_MAKE_ID2('A', 'C')
#define D3_ID_NLA D3_MAKE_ID2('N', 'L')

#define D3_ID_VS D3_MAKE_ID2('V', 'S')
#define D3_ID_VN D3_MAKE_ID2('V', 'N')

// ------------------------------------------------------------
#define D3_FORM D3_MAKE_ID('F', 'O', 'R', 'M')
#define D3_DDG1 D3_MAKE_ID('3', 'D', 'G', '1')
#define D3_DDG2 D3_MAKE_ID('3', 'D', 'G', '2')
#define D3_DDG3 D3_MAKE_ID('3', 'D', 'G', '3')
#define D3_DDG4 D3_MAKE_ID('3', 'D', 'G', '4')
#define D3_GOUR D3_MAKE_ID('G', 'O', 'U', 'R')
#define D3_BLEN D3_MAKE_ID('B', 'L', 'E', 'N')
#define D3_DER_ D3_MAKE_ID('D', 'E', 'R', '_')
#define D3_V100 D3_MAKE_ID('V', '1', '0', '0')
#define D3_DATA D3_MAKE_ID('D', 'A', 'T', 'A')
#define D3_GLOB D3_MAKE_ID('G', 'L', 'O', 'B')
#define D3_IMAG D3_MAKE_ID('I', 'M', 'A', 'G')
#define D3_TEST D3_MAKE_ID('T', 'E', 'S', 'T')
#define D3_USER D3_MAKE_ID('U', 'S', 'E', 'R')

// ------------------------------------------------------------
#define D3_DNA1 D3_MAKE_ID('D', 'N', 'A', '1')
#define D3_REND D3_MAKE_ID('R', 'E', 'N', 'D')
#define D3_ENDB D3_MAKE_ID('E', 'N', 'D', 'B')
#define D3_NAME D3_MAKE_ID('N', 'A', 'M', 'E')
#define D3_SDNA D3_MAKE_ID('S', 'D', 'N', 'A')
#define D3_TYPE D3_MAKE_ID('T', 'Y', 'P', 'E')
#define D3_TLEN D3_MAKE_ID('T', 'L', 'E', 'N')
#define D3_STRC D3_MAKE_ID('S', 'T', 'R', 'C')

// ------------------------------------------------------------
#define D3_SWITCH_INT(a)    \
	{                       \
		char s_i, *p_i;     \
		p_i = (char *)&(a); \
		s_i = p_i[0];       \
		p_i[0] = p_i[3];    \
		p_i[3] = s_i;       \
		s_i = p_i[1];       \
		p_i[1] = p_i[2];    \
		p_i[2] = s_i;       \
	}

// ------------------------------------------------------------
#define D3_SWITCH_SHORT(a)  \
	{                       \
		char s_i, *p_i;     \
		p_i = (char *)&(a); \
		s_i = p_i[0];       \
		p_i[0] = p_i[1];    \
		p_i[1] = s_i;       \
	}

// ------------------------------------------------------------
#define D3_SWITCH_LONGINT(a) \
	{                        \
		char s_i, *p_i;      \
		p_i = (char *)&(a);  \
		s_i = p_i[0];        \
		p_i[0] = p_i[7];     \
		p_i[7] = s_i;        \
		s_i = p_i[1];        \
		p_i[1] = p_i[6];     \
		p_i[6] = s_i;        \
		s_i = p_i[2];        \
		p_i[2] = p_i[5];     \
		p_i[5] = s_i;        \
		s_i = p_i[3];        \
		p_i[3] = p_i[4];     \
		p_i[4] = s_i;        \
	}

#endif  //__B_DEFINES_H__
