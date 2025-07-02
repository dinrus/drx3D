// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef XML_STRING
#define XML_STRING

typedef string xml_string;
/*
   #include <malloc.h>
   #include <conio.h>

   class xml_string {
   public:
   xml_string()
   {
    m_sBuf=NULL;
   }
   xml_string(tukk s)
   {
    m_sBuf=NULL;
    set(s);
   }
   xml_string(const wchar_t *s)
   {
    m_sBuf=NULL;
    set(s);
   }
   ~xml_string()
   {
    if(m_sBuf)
      free(m_sBuf);
    m_sBuf=NULL;
   }
   void set(tukk s)
   {
    if(m_sBuf)free(m_sBuf);
    size_t len=strlen(s);
    m_sBuf=(u8*)malloc(len+2);
    m_sBuf[0]=0;
    strcpy((char *)&m_sBuf[1],s);
   }
   void set(const wchar_t *s)
   {
    if(m_sBuf)free(m_sBuf);
    size_t len=wcslen(s);
    m_sBuf=(u8*)malloc((len+2)*sizeof(wchar_t));
    m_sBuf[0]=1;
    wcscpy(((wchar_t *)&m_sBuf[1]),s);
   }
   tukk c_str_s()
   {
    if(!m_sBuf) return NULL;
    if(m_sBuf[0]!=0)
    {
      u8 *t=m_sBuf;
      m_sBuf=wide2single((const wchar_t *)&t[1]);
      free(t);
      return ((tukk)&m_sBuf[1]);

    }
    return ((tukk)&m_sBuf[1]);
   }
   const wchar_t *c_str_w()
   {
    if(!m_sBuf) return NULL;
    if(m_sBuf[0]==0)
    {
      u8 *t=m_sBuf;
      m_sBuf=single2wide((tukk)&t[1]);
      free(t);
      return ((const wchar_t *)&m_sBuf[1]);
    }
    return ((const wchar_t *)&m_sBuf[1]);
   }
   xml_string& operator =(xml_string& s)
   {
    set(s.c_str_w());
    return *this;
   }
   xml_string& operator =(tukk s)
   {
    set(s);
    return *this;
   }
   xml_string& operator =(const wchar_t *s)
   {
    set(s);
    return *this;
   }

   private:
   u8 *wide2single(const wchar_t *s)
   {
    size_t len=wcslen(s);
    u8 *ns=allocsingle(len);
    char *t=((char *)&ns[1]);
    for(size_t i=0;i<len;i++)
    {
      t[i]=(char)s[i];
    }
    return ns;
   }
   u8 *single2wide(tukk s)
   {
    size_t len=strlen(s);
    u8 *w=allocwide(len);
    wchar_t *t=(wchar_t *)&w[1];
    for(size_t i=0;i<len;i++)
    {
      t[i]=(u8)s[i];
    }
    return w;
   }
   u8 *allocsingle(size_t len)
   {
    u8* buf=(u8*)malloc(len+2);
    memset(buf,0,len+2);
    buf[0]=0;
    return buf;
   }
   u8 *allocwide(size_t len)
   {
    u8 *buf=(u8*)malloc((len+2)*sizeof(wchar_t));
    memset(buf,0,(len+2)*sizeof(wchar_t));
    buf[0]=1;
    return buf;
   }
   //the first byte tell if is a wide char or a single char
   //first byte != 0 widechar
   //first byte == 0 singlechar
   u8 *m_sBuf;
   };

   /*i32 _tmain(i32 argc, _TCHAR* argv[])
   {
   xml_string a("sono small"),w(L"sono wide");
   for(i32 i=0;i<10000;i++){
    printf("%s\n",w.c_str_s());
    wprintf(L"%s\n",w.c_str_w());
    printf("%s\n",a.c_str_s());
    wprintf(L"%s\n",a.c_str_w());
   }
   _getch();
   return 0;
   }*/

#endif //XML_STRING
