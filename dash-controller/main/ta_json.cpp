/********************************************************************************
  System
********************************************************************************/

extern "C" {
#include "lvgl.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
}

#include "ta_json.hpp"

static const char *TAG = "ta_json";

float pow(float x, int y);

// inline int32_t  abs(int32_t n)                { if (n >= 0) return n; else return (-1 * n); }; 
// inline uint16_t min(uint16_t n1, uint16_t n2) { if (n1 < n2) return n1; else return n2; };

char String::ToUpper(char ch)
{
  if (ch >= 'a' && ch <= 'z')
    return (ch - 'a' + 'A');
  else
    return ch;
}

char String::NumToHex(uint8_t n)
{
  switch (n)
  {
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
    case 10: return 'A';
    case 11: return 'B';
    case 12: return 'C';
    case 13: return 'D';
    case 14: return 'E';
    case 15: return 'F';
    default: return '*';
  }
}

uint8_t String::HexToNum(char ch)
{
  switch (ToUpper(ch))
  {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
    default: return 0;
  }
}

uint16_t String::SzLen(const char *pch)
{
  uint16_t i = 0;
  
  while (*(pch + i) != 0)
    i++;
  
  return i;   
}

uint16_t String::SzLen(const char *pch, uint16_t maxLen)
{
  uint16_t i = 0;
  
  while ((*(pch + i) != 0) && (i < maxLen))
    i++;
  
  return i;  
}

bool String::operator==(const String & s) const
{
  uint16_t i;
  
  if (m_cch != s.m_cch)
    return false;

  for (i = 0; i < m_cch; i++)
    if (ToUpper(*(m_pch + i)) != ToUpper(*(s.m_pch + i)))
      return false;
  
  return true;	
}

bool String::operator!=(const String & s) const
{
  return !(*this == s);
}

int8_t String::Compare(const String & s) const
{
  uint16_t i = 0;
  while (i < m_cch && i < s.m_cch && ToUpper(m_pch[i]) == ToUpper(s.m_pch[i])) i++;
  if (i < m_cch && i < s.m_cch)
    return (ToUpper(m_pch[i]) < ToUpper(s.m_pch[i]) ? -1 : 1);
  if (i == m_cch && i == s.m_cch)
    return 0;
  return (m_cch < s.m_cch ? -1 : 1);
}

bool String::operator<(const String & s) const
{
  return Compare(s) < 0;
}

bool String::operator<=(const String & s) const
{
  return Compare(s) <= 0;
}

bool String::operator>(const String & s) const
{
  return Compare(s) > 0;
}

bool String::operator>=(const String & s) const
{
  return Compare(s) >= 0;
}

String String::SubStr(uint16_t iStart, uint16_t len) const
{
  if (iStart >= m_cch)
    return String(NULL, 0);
  else
    return String(m_pch + iStart, iStart + len > m_cch ? m_cch - iStart : len);
}


int64_t String::StrToInt() const
{
  int64_t nResult = 0;
  uint16_t i = 0;
  
  if (0 == m_cch)
    return 0;
  
  if (*m_pch == '-' || *m_pch == '+')
    i = 1;
  
  for (; i < m_cch && *(m_pch + i) >= '0' && *(m_pch + i) <= '9'; i++)
    nResult = nResult * 10 + (uint16_t)(*(m_pch + i) - '0');
  
  if (*m_pch == '-')
    nResult *= -1;
  
  return nResult;
}


float String::StrToFloat() const
{
  float     fResult = 0.0;
  float     fDivisor;
  int32_t   nExponentPart;
  uint8_t   i = 1;
  
  if (0 == m_cch)
    return 0.0;

  while (i < m_cch && *(m_pch + i) != '.')
    i++;
  
  for (i++, fDivisor = 10.0; 
       i < m_cch && fDivisor < 1000001.0 && *(m_pch + i) >= '0' && *(m_pch + i) <= '9'; 
       i++, fDivisor *= 10.0)
    fResult = fResult + ((float)(*(m_pch + i) - '0') / fDivisor);
  
  fResult += (float)StrToInt();
  
  while (i < m_cch && *(m_pch + i) != 'e' && *(m_pch + i) != 'E')
    i++;
  
  if (i < m_cch)
  {
    i++;
    nExponentPart = String(m_pch + i, m_cch - i).StrToInt();
    fResult *= pow(10.0, nExponentPart);
  }
  
  return fResult;
}

uint64_t String::HexToInt() const
{
  uint64_t nResult = 0;
  uint16_t i;
  
  for (i = 0; i < m_cch; i++)
  {
    if (m_pch[i] >= '0' && m_pch[i] <= '9')
      nResult = ((nResult << 4) | (uint32_t)(m_pch[i] - '0'));
    else if (m_pch[i] >= 'A' && m_pch[i] <= 'F')
      nResult = ((nResult << 4) | (uint32_t)(m_pch[i] - 'A' + 10));
    else
      break;
  }
  return nResult;
}

uint16_t String::HexToBin(uint8_t *pbDest, uint16_t cbDest) const
{
  uint16_t  i = 0;
  uint16_t  cbResult;
  uint8_t    *pbCurr = pbDest;
  
  while (i < m_cch && (m_pch[i] != 0) && (pbCurr - pbDest < cbDest))
  {
    *pbCurr = HexToNum(m_pch[i++]) << 4;
    *pbCurr++ |= HexToNum(m_pch[i++]);
  }
  
  cbResult = (pbCurr - pbDest);
  
  while (pbCurr < pbDest + cbDest)
    *pbCurr++ = 0;
  
  return cbResult;
}


Buffer::Buffer(uint16_t maxLen)
{
  m_pch = new char[maxLen];

  m_cch = 0;
  m_cchMax = maxLen;
  Reset();
}


bool Buffer::operator=(const String & s)
{
  if (m_cchMax >= s.Len())
  {
    Reset();
    memcpy(m_pch, s.Pch(), s.Len());
    m_cch = s.Len();
    return true;
  }
  
  return false;
}

bool Buffer::operator=(const Buffer & s)
{
  return *this = String(s);
}

bool  Buffer::operator+=(const String & s)
{
  if (m_cchMax >= m_cch + s.Len())
  {
    memcpy(m_pch + m_cch, s.Pch(), s.Len());
    m_cch += s.Len();
    return true;
  }
  
  return false;
}

bool  Buffer::operator+=(char ch)
{
  if (m_cch < m_cchMax)
  {
    m_pch[m_cch++] = ch;
    return true;
  }
  
  return false;  
}

bool  Buffer::operator+=(uint8_t n)
{
  if (m_cch < m_cchMax)
  {
    m_pch[m_cch++] = (char)n;
    return true;
  }
  
  return false;  
}

bool Buffer::AddData(const uint8_t *pData, uint8_t cbData)
{
  if (m_cchMax >= m_cch + cbData)
  {
    memcpy(m_pch + m_cch, pData, cbData);
    m_cch += cbData;
    return true;
  }
  
  return false;
}

void Buffer::StrFromInt(int32_t n)
{
  char      *pchCurr = m_pch + m_cch;
  char      *pchEnd = m_pch + m_cchMax;
  int32_t   nDivisor = 1000000000;
  
  if (0 == n)
  {
    if (pchCurr < pchEnd)
      *pchCurr++ = '0';
    if (pchCurr < pchEnd)
      *pchCurr = 0;
  }
  else
  {    
    if (n < 0)
    {
      if (pchCurr < pchEnd)
        *pchCurr++ = '-';
      n = -1 * n;
    }
  
    while (0 == (n / nDivisor))
      nDivisor /= 10;
    
    while (nDivisor > 0 && (pchCurr < pchEnd))
    {
      *pchCurr++ = NumToHex((uint8_t)(n / nDivisor));
      n %= nDivisor;
      nDivisor /= 10;
    }
    if (pchCurr < pchEnd)
      *pchCurr = 0;
  }
  m_cch = (pchCurr - m_pch);  
}

void Buffer::HexFromInt(uint16_t n)
{
  uint16_t  i = 0;
  
  if ((n & 0xF000) && (m_cch < m_cchMax))
    m_pch[m_cch + i++] = NumToHex(n >> 12);
  if ((i > 0 || (n & 0x0F00)) && (m_cch + i < m_cchMax))
    m_pch[m_cch + i++] = NumToHex((n >> 8) & 0xF);
  if ((i > 0 || (n & 0x00F0)) && (m_cch + i < m_cchMax))
    m_pch[m_cch + i++] = NumToHex((n >> 4) & 0xF);
  if (m_cch + i < m_cchMax)
    m_pch[m_cch + i++] = NumToHex(n & 0x000F);
  
  m_cch += i;
}

void Buffer::HexFromBin(const uint8_t *pbSrc, uint16_t cbSrc)
{
  uint16_t  i;
  char      *pchCurr = m_pch + m_cch;
  char      *pchEnd = m_pch + m_cchMax;
  
  for (i = 0; (i < cbSrc) && (pchCurr <= pchEnd - 2); i++)
  {
    *pchCurr++ = NumToHex(*(pbSrc + i) >> 4);
    *pchCurr++ = NumToHex(*(pbSrc + i) & 0xF);
  }
  
  m_cch = (pchCurr - m_pch);
}

void Buffer::Shift(uint16_t cch)
{
  if (cch >= m_cch)
  {
    m_cch = 0; 
  }
  else
  {
    memmove(m_pch, m_pch + cch, m_cch - cch);
    m_cch -= cch;
  }
}

void Buffer::Flush()
{
  if (m_cch > 0)
    Shift(SzLen() + 1);
}

void Lexer::Reset(const String & sSource)
{
  m_chNext = ' '; 
  m_tokNext = TOK_INVALID;
  m_pchString = NULL;
  m_cchString = 0;
  
  m_sSource = sSource;
  m_ichSource = 0;
}

void Lexer::GetNextTok()
{
  m_tokNext = TOK_END_OF_STREAM;
  
  while (IsWS(m_chNext))
    if (!GetNextChar())
      return;
  
  switch (m_chNext)
  {
    case '{':
    {
      GetNextChar();
      m_tokNext = TOK_LEFT_BRACE;
      break;
    }
    case '}':
    {
      GetNextChar();
      m_tokNext = TOK_RIGHT_BRACE;
      break;
    }
    case '[':
    {
      GetNextChar();
      m_tokNext = TOK_LEFT_SQUARE_BRACKET;
      break;
    }
    case ']':
    {
      GetNextChar();
      m_tokNext = TOK_RIGHT_SQUARE_BRACKET;
      break;
    }
    case ':':
    {
      GetNextChar();
      m_tokNext = TOK_COLON;
      break;
    }
    case ',':
    {
      GetNextChar();
      m_tokNext = TOK_COMMA;
      break;
    }
    case '=':
    {
      GetNextChar();
      m_tokNext = TOK_EQUALS;
      break;
    }
    case 0:
    {
      m_tokNext = TOK_END_OF_STREAM;
      break;				
    }
    case '"':  // quoted string
    {
      m_tokNext = TOK_QUOTED_STRING;
      
      m_pchString = m_sSource.Pch() + m_ichSource;
      m_cchString = 0;
      while (GetNextChar())
      {
        if ('"' == m_chNext)
        {
          GetNextChar();
          break;
        }        
        m_cchString++;
      }
      break;
    }
    case 'n':  // NULL
    {
      m_tokNext = TOK_NULL;
      
      m_pchString = m_sSource.Pch() + m_ichSource - 1;
      m_cchString = 4;
      if (CurrString() == "null")
        m_ichSource += 3;
      else
        m_tokNext = TOK_INVALID;
      
      GetNextChar();

      break;
    }
    case 't':  // true
    {
      m_tokNext = TOK_TRUE;
      
      m_pchString = m_sSource.Pch() + m_ichSource - 1;
      m_cchString = 4;
      if (CurrString() == "true")
        m_ichSource += 3;
      else
        m_tokNext = TOK_INVALID;
      
      GetNextChar();

      break;
    }
    case 'f':  // NULL
    {
      m_tokNext = TOK_FALSE;
      
      m_pchString = m_sSource.Pch() + m_ichSource - 1;
      m_cchString = 5;
      if (CurrString() == "false")
        m_ichSource += 4;
      else
        m_tokNext = TOK_INVALID;
      
      GetNextChar();

      break;
    }
    default:  // number assumed
    {
      if (m_chNext != '-' && (m_chNext < '0' || m_chNext > '9'))
      {
        m_tokNext = TOK_INVALID;
        return;      
      }
      
      m_tokNext = TOK_NUMBER;
      
      m_pchString = m_sSource.Pch() + m_ichSource - 1;
      m_cchString = 1;
      while (GetNextChar())
      {
        if (m_chNext != '.' && (m_chNext < '0' || m_chNext > '9'))
          break;
        
        m_cchString++;
      }
      break;
    }
  }  
}

bool JToken::Find(const String & sKey, const String & sValue)
{
  JToken * pJToken = Find(sKey);

  return (pJToken && pJToken->Child() && pJToken->Child()->Value() == sValue);
}

bool JToken::FindHex(const String & sKey, uint16_t nValue)
{
  JToken * pJToken = Find(sKey);

  return (pJToken && pJToken->Value().HexToInt() == nValue);
}

JToken * JToken::Find(const String & sKey)
{
  JToken * pJToken = this;

  if (m_pChild && Type() == JTOK_OBJECT) {
    pJToken = m_pChild;
  }

  while (pJToken && (pJToken->m_string != sKey)) {
    pJToken = pJToken->m_pNext;
  }
  
  return pJToken;
}

const char *x_pchInvalidInput = "Invalid Input";
#define CONSUME_TOK(tok) if (tok != lexer.NextTok()) { m_pTokens->Clear(); m_sError = (char *)x_pchInvalidInput; return NULL; } else { lexer.GetNextTok(); }
#define ERROR_CHECK(cond,msg) if (cond) { m_pTokens->Clear(); m_sError = msg; return NULL; }

JToken * JSON::ParseToken(Lexer & lexer)
{
  switch (lexer.NextTok())
  {
  case TOK_LEFT_BRACE:
    return ParseObject(lexer);
  case TOK_LEFT_SQUARE_BRACKET:
    return ParseArray(lexer);
  case TOK_QUOTED_STRING: case TOK_NUMBER: case TOK_NULL: case TOK_TRUE: case TOK_FALSE:
    return ParseValue(lexer);
  default:
    return NULL;
  }
  
  return NULL;
}

JToken * JSON::ParseValue(Lexer & lexer)
{
  JToken * pResult = NewJToken(lexer.CurrString());
  ERROR_CHECK(!pResult, "Too many nodes required for parsing");
  
  if (lexer.NextTok() == TOK_NUMBER)
    pResult->SetType(JTOK_NUMBER);
  else if (lexer.NextTok() == TOK_NULL)
    pResult->SetType(JTOK_NULL);
  else if (lexer.NextTok() == TOK_TRUE)
    pResult->SetType(JTOK_TRUE);
  else if (lexer.NextTok() == TOK_FALSE)
    pResult->SetType(JTOK_FALSE);
  
  lexer.GetNextTok();
  return pResult;
}

JToken * JSON::ParseObject(Lexer & lexer)
{
  JToken * pResult = NewJToken(JTOK_OBJECT);
  JToken * pCurrent;
  ERROR_CHECK(!pResult, "Too many nodes required for parsing");

  CONSUME_TOK(TOK_LEFT_BRACE);
  
  pCurrent = NewJToken(lexer.CurrString());
  ERROR_CHECK(!pCurrent, "Too many nodes required for parsing");  
  pResult->m_pChild = pCurrent;
  
  CONSUME_TOK(TOK_QUOTED_STRING);
  CONSUME_TOK(TOK_COLON);
  
  if (!(pCurrent->m_pChild = ParseToken(lexer)))
      return NULL;

  while (TOK_RIGHT_BRACE != lexer.NextTok())
  {
    CONSUME_TOK(TOK_COMMA);
    
    pCurrent->m_pNext = NewJToken(lexer.CurrString());
    pCurrent = pCurrent->m_pNext;
    ERROR_CHECK(!pCurrent, "Too many nodes required for parsing");  
    
    CONSUME_TOK(TOK_QUOTED_STRING);
    CONSUME_TOK(TOK_COLON);

    ERROR_CHECK(!(pCurrent->m_pChild = ParseToken(lexer)), x_pchInvalidInput);
  }
        
  CONSUME_TOK(TOK_RIGHT_BRACE);
  return pResult;
}

JToken * JSON::ParseArray(Lexer & lexer)
{
  JToken * pResult = NewJToken(JTOK_ARRAY);
  JToken * pCurrent;
  ERROR_CHECK(!pResult, "Too many nodes required for parsing");

  CONSUME_TOK(TOK_LEFT_SQUARE_BRACKET);
  
  pCurrent = ParseToken(lexer);
  pResult->m_pChild = pCurrent;
  
  while (TOK_RIGHT_SQUARE_BRACKET != lexer.NextTok())
  {
    ERROR_CHECK(!pCurrent, x_pchInvalidInput);
    CONSUME_TOK(TOK_COMMA);
    
    pCurrent->m_pNext = ParseToken(lexer);
    pCurrent = pCurrent->m_pNext;
    ERROR_CHECK(!pCurrent, "Too many nodes required for parsing");    
  }
  CONSUME_TOK(TOK_RIGHT_SQUARE_BRACKET);
  return pResult;
}

JToken * JSON::NewJToken(JTokenType_t type)
{
  JToken * pResult = NULL;
  
  if (m_pTokens->Enqueue())
  {
    pResult = &(m_pTokens->Tail());
    pResult->Reset();
    pResult->SetType(type);
  }
  
  return pResult;
}

JToken * JSON::NewJToken(JToken * parent, const String & s)
{
  JToken * pResult = NULL;
  
  if (m_pTokens->Enqueue())
  {
    pResult = &(m_pTokens->Tail());
    pResult->Reset();
    *pResult = s;
    if (parent)
    {
      parent->Add(pResult);
    }
  }
  
  return pResult;
}

JToken * JSON::NewJToken(JToken * parent, const String & key, const String & val)
{
  JToken * pResult = NewJToken(JTOK_OBJECT);   
  JToken * pCurrent = NewJToken(pResult, key);

  if (pCurrent)
  {
    NewJToken(pCurrent, val);
  }
  
  if (parent) {
    parent->Add(pResult);
  }
  return pResult;
}

// szJSON argument must be terminated with a NULL
JToken * JSON::Parse(const String & sJSON)
{
  Lexer   lexer(sJSON);
  
  Reset();
  lexer.GetNextTok();
  return ParseToken(lexer);
}

JToken * JToken::Add(JToken *pJToken)
{
  JToken * pLastToken = m_pChild;
  
  if (pLastToken)
  {
    while (pLastToken->m_pNext)
      pLastToken = pLastToken->m_pNext;
    
    if (pJToken && pJToken->Type() == JTOK_OBJECT)
      pJToken = pJToken->m_pChild;
    
    pLastToken->m_pNext = pJToken;
  }
  
  return this;
}

bool JToken::Stringify(Buffer & buffer)
{
  JToken * pCurrent;

  switch (Type())
  {
  case JTOK_NULL:
    {
      return (buffer += "null");
    }
  case JTOK_TRUE:
    {
      return (buffer += "true");
    }
  case JTOK_FALSE:
    {
      return (buffer += "false");
    }
  case JTOK_NUMBER:
    {
      return (buffer += Value());
    }
  case JTOK_STRING:
    {
      buffer += '\"';
      buffer += Value();
      return (buffer += '\"');
    }
  case JTOK_OBJECT:
    {
      buffer += '{';
      pCurrent = m_pChild;
      while (pCurrent)
      {
        buffer += '\"';
        buffer += pCurrent->Value();
        buffer += "\":";
        
        if (pCurrent->m_pChild)
          pCurrent->m_pChild->Stringify(buffer);
        pCurrent = pCurrent->m_pNext;
        if (pCurrent)
          buffer += ',';  
      }
      return (buffer += '}');
    }
  case JTOK_ARRAY:
    {
      pCurrent = m_pChild;
      buffer += '[';
      while (pCurrent)
      {
        pCurrent->Stringify(buffer);
        pCurrent = pCurrent->m_pNext;
        if (pCurrent)
          buffer += ',';  
      }
      return (buffer += ']');
    }
    default:
      return false;
  }
  return true;
}

void JToken::Debugify(uint8_t depth)
{
  JToken * pCurrent;
  for (int i = 0; i < depth; i++)
    printf("  ");
  switch (Type())
  {
  case JTOK_NULL:
    {
      printf("JTOK_NULL %.*s\n", m_string.Len(), m_string.Pch());
      return;
    }
  case JTOK_TRUE:
    {
      printf("JTOK_TRUE %.*s\n", m_string.Len(), m_string.Pch());
      return;
    }
  case JTOK_FALSE:
    {
      printf("JTOK_FALSE %.*s\n", m_string.Len(), m_string.Pch());
      return;
    }
  case JTOK_NUMBER:
    {
      printf("JTOK_NUMBER %.*s\n", m_string.Len(), m_string.Pch());
      return;
    }
  case JTOK_STRING:
    {
      printf("JTOK_STRING %.*s\n", m_string.Len(), m_string.Pch());
 //     buffer += '\"';
   //   buffer += Value();
      return;
    }
  case JTOK_OBJECT:
    {
      printf("JTOK_OBJECT %.*s\n", m_string.Len(), m_string.Pch());
      pCurrent = m_pChild;
      while (pCurrent)
      {
        for (int i = 0; i < depth; i++)
          printf("  ");
        printf("  %.*s\n", pCurrent->Value().Len(), pCurrent->Value().Pch());
        
    //    buffer += '\"';
      //  buffer += pCurrent->Value();
        //buffer += "\":";
        
        if (pCurrent->m_pChild)
          pCurrent->m_pChild->Debugify(depth + 1);
        pCurrent = pCurrent->m_pNext;
      }
      return ;
    }
  case JTOK_ARRAY:
    {
      printf("JTOK_ARRAY %.*s\n", m_string.Len(), m_string.Pch());
      pCurrent = m_pChild;
      while (pCurrent)
      {
        pCurrent->Debugify(depth + 1);
        pCurrent = pCurrent->m_pNext;
      }
      return ;
    }
    default:
      return;
  }
  return;
}

bool JSON::Stringify(Buffer & buffer)
{
  if (!m_pTokens->Empty())
    return m_pTokens->Head().Stringify(buffer); 
  return false;
}

uint8_t StrTokenLen(const String & s)
{
  uint8_t  i;
  for (i = 0; i < s.Len() && s[i] >= 32 && s[i] < 0x80; i++); 
  return i;
}
