class String
{
protected:  
  char      *m_pch;
  uint16_t  m_cch;
  
  int8_t Compare(const String & s) const;

  static char ToUpper(char ch);
  static char NumToHex(uint8_t n);
  static uint8_t HexToNum(char ch);
  
public:
  String()                              { m_pch = NULL; m_cch = 0; };
  String(const char *pch)               { m_pch = (char *)pch; m_cch = 0xFFFF; m_cch = SzLen(); };
  String(const char *pch, uint16_t cch) { m_pch = (char *)pch; m_cch = cch; };
  String(const String & s)              { m_pch = s.m_pch; m_cch = s.m_cch; };
  const String&     operator=(const String & s)   { m_pch = s.m_pch; m_cch = s.m_cch; return *this;};
  
  static uint16_t   SzLen(const char *pch);
  static uint16_t   SzLen(const char *pch, uint16_t maxLen);
  char *            Pch() const       { return m_pch; };
  uint16_t          Len() const       { return m_cch; };
  uint16_t &        Len()             { return m_cch; };
  uint16_t          SzLen() const     { return SzLen(m_pch, m_cch); };
  bool              NullTerminated() const  { return (m_cch > 0) ? m_pch[m_cch - 1] == '\0' : false; };

  bool              operator==(const String & s) const;
  bool              operator!=(const String & s) const;

  bool              operator<(const String & s) const;
  bool              operator<=(const String & s) const;
  bool              operator>(const String & s) const;
  bool              operator>=(const String & s) const;

  char              operator[] (uint16_t i)       { return m_pch[i]; };
  const char&       operator[] (uint16_t i) const { return m_pch[i]; };

  String            SubStr(uint16_t iStart, uint16_t len) const;
  
  bool              IsNumber() const;
  int64_t           StrToInt() const;
  float             StrToFloat() const;
  uint64_t          HexToInt() const;
  uint16_t          HexToBin(uint8_t *pbDest, uint16_t cbDest) const;
};

class Buffer : public String
{
protected:
  uint16_t  m_cchMax;
public:
  Buffer(): String()          { m_cchMax = 0; };
  Buffer(char *pch, uint16_t cch): String(pch, 0) { m_cchMax = cch; Reset(); };
  Buffer(uint16_t maxLen);
  
  uint16_t  MaxLen() const    { return m_cchMax; };

  bool      Full() const      { return m_cch == m_cchMax; };
  
  bool      operator=(const String & s);
  bool      operator=(const Buffer & s);
  bool      operator+=(const String & s);
  bool      operator+=(char ch);
  bool      operator+=(uint8_t n);
  bool      AddData(const uint8_t *pData, uint8_t cbData);
  
  void      StrFromInt(int32_t n);
  void      HexFromInt(uint16_t n);
  void      HexFromBin(const uint8_t *pbSrc, uint16_t cbSrc);

  void      Reset()           { if (m_pch) { *m_pch = 0; memset(m_pch, 0, m_cchMax); } m_cch = 0; };  
  void      SetLenToSzLen()   { m_cch = m_cchMax; m_cch = SzLen(); };  
  void      Shift(uint16_t cch);
  void      Flush();
};

template <class T>
class Array
{
protected:
  T         *m_pData;
  uint16_t  m_cElems;
  
  static void InvalidAccess()             { while(1) { /* do nothing*/ }; }; 
  
public:
  Array();
  Array(uint16_t cElems);
  
  uint16_t  Size() const                  { return m_cElems; };
  void      Alloc(uint16_t cElems);
  T&        Elem(uint16_t i);
  const T&  Elem(uint16_t i) const;
  T&        operator[] (uint16_t i)       { return Elem(i); };
  const T&  operator[] (uint16_t i) const { return Elem(i); };
};

template <class T>
class Queue : public Array<T>
{
protected:
  uint16_t  m_iHead;
  uint16_t  m_cQueueElems;  

  uint16_t ArrayIndex(uint16_t iQueue) const    { return ((m_iHead + iQueue) % Array<T>::m_cElems); };
public:
  Queue(): Array<T>()                           { m_iHead = 0; m_cQueueElems = 0; };
  Queue(uint16_t cMaxSize): Array<T>(cMaxSize)  { m_iHead = 0; m_cQueueElems = 0; };
  
  void      Init(uint16_t cMaxSize)             { Array<T>::Alloc(cMaxSize); m_iHead = 0; m_cQueueElems = 0; }; 
  void      Clear()                             { m_iHead = 0; m_cQueueElems = 0; };
  uint16_t  Size() const                        { return m_cQueueElems; };
  uint16_t  Capacity() const                    { return Array<T>::Size; };
  bool      Empty() const                       { return m_cQueueElems == 0; };
  bool      Full() const                        { return m_cQueueElems == Array<T>::m_cElems; };
  bool      Find(T & elem) const;
  bool      Find(const T & elem) const;
  uint16_t  FindIndex(const T & elem) const;    // returns 0xFFFF if element not found
  T&        Elem(uint16_t i);
  const T&  Elem(uint16_t i) const;
  T&        operator[] (uint16_t i)             { return Elem(i); };
  const T&  operator[] (uint16_t i) const       { return Elem(i); };
  T&        Head()                              { return Array<T>::Elem(m_iHead); };
  T&        Tail()                              { return Elem(m_cQueueElems - 1); };
  
  bool      Enqueue();
  bool      Enqueue(const T & elem);
  bool      EnqueueUnique(const T & elem);
  bool      EnqueueHead();
  bool      EnqueueHead(const T & elem);
  
  bool      Dequeue();
  bool      Dequeue(T & elem);
  bool      DequeueTail();
  bool      DequeueTail(T & elem);
};

enum LexToken_t
{
  TOK_INVALID = 0,
  TOK_LEFT_BRACE,
  TOK_RIGHT_BRACE,
  TOK_LEFT_SQUARE_BRACKET,
  TOK_RIGHT_SQUARE_BRACKET,
  TOK_COLON,
  TOK_COMMA,
  TOK_EQUALS,
  TOK_NUMBER,
  TOK_QUOTED_STRING,
  TOK_NULL,
  TOK_TRUE,
  TOK_FALSE,
  TOK_END_OF_STREAM,
  TOK_ERROR  
};

class Lexer
{
protected:
  char          m_chNext;
  LexToken_t    m_tokNext;
  char *        m_pchString;
  uint16_t      m_cchString;
  
  String        m_sSource;
  uint16_t      m_ichSource;
  
  bool          GetNextChar()             { m_chNext = m_sSource[m_ichSource++]; return (0 != m_chNext && m_ichSource <= m_sSource.Len()); };  
  bool          IsWS(char ch)             { return (' ' == ch || '\n' == ch || '\r' == ch || '\t' == ch); };

public:
  Lexer()                                 { Reset(NULL); };
  Lexer(const String & sSource)                   { Reset(sSource); };
  
  void          Reset(const String & sSource);
  String        CurrString() const        { return String(m_pchString,m_cchString); };
  void          GetNextTok();
  LexToken_t    NextTok() const           { return m_tokNext; };
};

typedef enum {
  JTOK_UNDEFINED = 0,
  JTOK_OBJECT = 1,
  JTOK_ARRAY = 2,
  JTOK_STRING = 3,
  JTOK_NUMBER = 4,
  JTOK_NULL = 5,
  JTOK_TRUE = 6,
  JTOK_FALSE = 7
} JTokenType_t;

class JSON;

class JToken
{
protected:
  JTokenType_t    m_type;
  String          m_string;
  JToken *        m_pNext;
  JToken *        m_pChild;
public:
  JToken():m_string()                           { Reset(); };
  void            Reset()                       { m_type = JTOK_UNDEFINED; m_pNext = NULL; m_pChild = NULL; m_string = String();};
  const String&   operator=(const String & s)   { m_string = s; m_type = JTOK_STRING; m_pNext = NULL; m_pChild = NULL; return m_string;};
  void            SetType(JTokenType_t t)       { m_type = t; };
  JTokenType_t    Type() const                  { return m_type; };
  String          Value() const                 { return m_string; };
  String          ChildValue() const            { return m_pChild ? m_pChild->m_string : String(); };
  JToken *        Next()                        { return m_pNext; };
  JToken *        Child()                       { return m_pChild; };
  
  JToken *        Add(JToken *pJToken);
  JToken *        Find(const String & sKey);   
  bool            Find(const String & sKey, const String & sValue);   
  bool            FindHex(const String & sKey, uint16_t nValue);
  bool            Stringify(Buffer & buffer);
  void            Debugify(uint8_t depth = 0);

  friend JSON;
};

class JSON
{
protected:
  String          m_sError;
  
  Queue<JToken> * m_pTokens;
  
  JToken *        ParseToken(Lexer & lexer);
  JToken *        ParseValue(Lexer & lexer);
  JToken *        ParseObject(Lexer & lexer);
  JToken *        ParseArray(Lexer & lexer);
public:
  JSON(Queue<JToken> & qTokens): m_sError() { m_pTokens = & qTokens; };
  void            Reset()             { m_pTokens->Clear(); };

  JToken *        NewJToken()         { return NewJToken(NULL, ""); };
  JToken *        NewJToken(JTokenType_t type);
  JToken *        NewJToken(JToken * parent, const String & s);
  JToken *        NewJToken(const String & s)
                                      { return NewJToken(NULL, s); };
  JToken *        NewJToken(JToken * parent, const String & key, const String & val);
  JToken *        NewJToken(const String & key, const String & val)
                                      { return NewJToken(NULL, key, val); }
  
  JToken *        Parse(const String & sJSON);
  const String &  ErrorString() const { return m_sError; };
  JToken *        Root()              { return &(m_pTokens->Head()); };

  bool            Stringify(Buffer & buffer);
};

template <class T>
Array<T>::Array()
{
  m_cElems = 0; 
  m_pData = NULL; 
}

template <class T>
Array<T>::Array(uint16_t cElems)
{
  Alloc(cElems);
}

template <class T>
void Array<T>::Alloc(uint16_t cElems)  
{ 
  m_cElems = 0; 
  m_pData = new T[cElems];
  if (m_pData)
    m_cElems = cElems;   
}

template <class T>
T& Array<T>::Elem(uint16_t i)
{
  if (i >= m_cElems)
    InvalidAccess();

  return m_pData[i];
}

template <class T>
const T& Array<T>::Elem(uint16_t i) const
{
  if (i >= m_cElems)
    InvalidAccess();

  return m_pData[i];
}

template <class T>
bool Queue<T>::Find(T & elem) const
{
  uint16_t  i;

  for (i = 0; i < Size(); i++)
    if (Elem(i) == elem)
    {
      elem = Elem(i);
      return true;
    }
  
  return false;
}

template <class T>
bool Queue<T>::Find(const T & elem) const
{
  uint16_t  i;

  for (i = 0; i < Size(); i++)
    if (Elem(i) == elem)
      return true;
 
  return false;
}

template <class T>
uint16_t Queue<T>::FindIndex(const T & elem) const
{
  uint16_t  i;

  for (i = 0; i < Size(); i++)
    if (Elem(i) == elem)
      return i;

  return 0xFFFF;
}

template <class T>
T& Queue<T>::Elem(uint16_t i)
{
  if (Empty() || i >= Size())
    Array<T>::InvalidAccess();

  return Array<T>::m_pData[ArrayIndex(i)];
}

template <class T>
const T& Queue<T>::Elem(uint16_t i) const
{
  if (Empty() || i >= Size())
    Array<T>::InvalidAccess();

  return Array<T>::m_pData[ArrayIndex(i)];
}

template <class T>
bool Queue<T>::Enqueue()
{
  if (Full())
    return false;
  
  m_cQueueElems++;
  return true;
}

template <class T>
bool Queue<T>::Enqueue(const T & elem)
{
  if (Full())
    return false;

  Elem(m_cQueueElems++) = elem;
  return true;
}

template <class T>
bool Queue<T>::EnqueueUnique(const T & elem)
{
  uint16_t  i;

  for (i = 0; i < Size(); i++)
    if (Elem(i) == elem)
      return false;
  
  return Enqueue(elem);
}

template <class T>
bool Queue<T>::EnqueueHead()
{
  if (Full())
    return false;
  
  if (m_iHead == 0)
    m_iHead = Array<T>::m_cElems - 1;
  else
    m_iHead--;
  
  m_cQueueElems++;
  return true;
}

template <class T>
bool Queue<T>::EnqueueHead(const T & elem)
{
  if (!EnqueueHead())
    return false;
  
  Head() = elem;
  return true;
}

template <class T>
bool Queue<T>::Dequeue()
{
  if (!Empty())
  {
    m_iHead = (m_iHead + 1) % Array<T>::Size();
    m_cQueueElems--;
    return true;
  }
  
  return false;
}

template <class T>
bool Queue<T>::Dequeue(T & elem)
{
  if (!Empty())
  {
    elem = Array<T>::Elem(m_iHead);
    m_iHead = (m_iHead + 1) % Array<T>::Size();
    m_cQueueElems--;
    return true;
  }
  
  return false;
}

template <class T>
bool Queue<T>::DequeueTail(T & elem)
{
  if (!Empty())
  {
    elem = Tail();
    m_cQueueElems--;
    return true;
  }
  
  return false;
}

template <class T>
bool Queue<T>::DequeueTail()
{
  if (!Empty())
  {
    m_cQueueElems--;
    return true;
  }
  
  return false;
}
