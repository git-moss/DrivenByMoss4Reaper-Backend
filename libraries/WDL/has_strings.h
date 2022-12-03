#ifndef _WDL_HASSTRINGS_H_
#define _WDL_HASSTRINGS_H_

#ifndef WDL_HASSTRINGS_EXPORT
#define WDL_HASSTRINGS_EXPORT
#endif

WDL_HASSTRINGS_EXPORT bool hasStrings_isNonWordChar(int c)
{
  // treat as whitespace when searching for " foo "
  switch (c)
  {
    case 0:
    case 1:
    case ' ':
    case '\t':
    case '.':
    case '/':
    case '\\':
      return true;

    default:
      return false;
  }
}

static int hasStrings_casecmp(const char *a, const char *b, unsigned int n)
{
  while (n)
  {
    char ca=*a, cb=*b;
    // ca may be any character (including A-Z), cb will never be A-Z or NUL
    WDL_ASSERT(cb != 0 && !(cb >= 'A' && cb <= 'Z'));
    cb -= ca;
    // if ca is A, and cb is a, cb will be 'a'-'A'
    if (cb && (cb != 'a'-'A' || ca < 'A' || ca > 'Z')) return ca ? 1 : -1;
    ++a;
    ++b;
    --n;
  }
  return 0;
}

WDL_HASSTRINGS_EXPORT bool WDL_hasStringsEx2(const char **name_list, int name_list_size,
    const LineParser *lp,
    int (*cmp_func)(const char *a, int apos, const char *b, int blen)  // if set, returns length of matched string (0 if no match)
    )
{
  if (!lp) return true;
  const int ntok = lp->getnumtokens();
  if (ntok<1) return true;

  char stack_[1024]; // &1=not bit, 0x10 = ignoring subscopes, &2= state when 0x10 set
  int stacktop = 0, stacktop_v;
#define TOP_OF_STACK stacktop_v
#define PUSH_STACK(x) do { if (stacktop < (int)sizeof(stack_) - 1) stack_[stacktop++] = stacktop_v&0xff; stacktop_v = (x); } while(0)
#define POP_STACK() (stacktop_v = stack_[--stacktop])
  TOP_OF_STACK = 0;

  char matched_local=-1; // -1 = first eval for scope, 0=did not pass scope, 1=OK, 2=ignore rest of scope
  for (int x = 0; x < ntok; x ++)
  {
    const char *n=lp->gettoken_str(x);
    
    if (n[0] == '(' && !n[1] && !lp->gettoken_quotingchar(x))
    {
      if (!(matched_local&1))
      {
        TOP_OF_STACK |= matched_local | 0x10;
        matched_local=2; // ignore subscope
      }
      else 
      {
        matched_local = -1; // new scope
      }

      PUSH_STACK(0);
    }
    else if (n[0] == ')' && !n[1] && stacktop && !lp->gettoken_quotingchar(x))
    {
      if (POP_STACK()&0x10)
      {
        // restore state
        matched_local = TOP_OF_STACK&2;
      }
      else
      {
        matched_local = (matched_local != 0 ? 1 : 0) ^ (TOP_OF_STACK&1);
      }
      TOP_OF_STACK = 0;
    }
    else if (n[0] == 'O' && n[1] == 'R' && !n[2] && matched_local != 2 && !lp->gettoken_quotingchar(x))
    {
      matched_local = (matched_local > 0) ? 2 : -1;
      TOP_OF_STACK = 0;
    }
    else if (matched_local&1) // matches 1, -1
    {
      int ln = (int)strlen(n);
      if (ln>0)
      {
        // ^foo -- string starts (or follows \1 separator with) foo
        // foo$ -- string ends with foo (or is immediately followed by \1 separator)
        // " foo ", "foo ", " foo" include end of string/start of string has whitespace
        int wc_left = 0; // 1=require \1 or start of string, 2=require space or \1 or start
        int wc_right = 0; // 1=require \1 or \0, 2 = require space or \1 or \0
        // perhaps wc_left/wc_right of 2 should also match non-alnum characters in addition to space?
        if (ln>1)
        {
          switch (*n) 
          {
            case ' ': 
              if (*++n != ' ') wc_left=2;
              // else { multiple characters of whitespace = literal whitespace search (two spaces requires a single space, etc) }

              ln--;
            break;
            case '^': 
              ln--; 
              n++; 
              wc_left=1; 
            break;
            // upper case being here implies it is almost certainly NOT/AND due to postprocessing in WDL_makeSearchFilter
            case 'N':
              if (WDL_NORMALLY(!strcmp(n,"NOT") && !lp->gettoken_quotingchar(x)))
              {
                TOP_OF_STACK^=1;
                continue;
              }
            break;
            case 'A':
              if (WDL_NORMALLY(!strcmp(n,"AND") && !lp->gettoken_quotingchar(x)))
              {
                // ignore unquoted uppercase AND
                continue;
              }
            break;
          }
        }
        if (ln>1)
        {
          switch (n[ln-1]) 
          {
            case ' ':               
              if (n[--ln - 1] != ' ') wc_right=2;
              // else { multiple characters of whitespace = literal whitespace search (two spaces requires a single space, etc) }
            break;
            case '$': 
              ln--; 
              wc_right++; 
            break;
          }
        }

        bool use_cmp_func = cmp_func != NULL && !(TOP_OF_STACK&1);

        if (!wc_left && !wc_right && *n)
        {
          switch (lp->gettoken_quotingchar(x))
          {
            case '\'':
            case '"':
              { // if a quoted string has no whitespace in it, treat as whole word search
                const char *p = n;
                while (*p && *p != ' ' && *p != '\t') p++;
                if (!*p)
                {
                  wc_left=wc_right=2;
                  use_cmp_func = false;
                }
              }
            break;
          }
        }

        bool matched = false;
        for (int i = 0; i < name_list_size; i ++)
        {
          const char *name = name_list[i];
          const char *t = name;

#define MATCH_RIGHT_CHECK_WORD(SZ) \
                (wc_right == 0 || ((const unsigned char*)(t))[SZ] < 2 || (wc_right > 1 && hasStrings_isNonWordChar((t)[SZ])))

#define MATCH_LEFT_SKIP_TO_WORD() do { \
                unsigned char lastchar = *(unsigned char*)t++; \
                if (lastchar < 2 || (wc_left>1 && hasStrings_isNonWordChar(lastchar))) break; \
              } while (t[0])

          if (use_cmp_func)
          {
            while (t[0])
            {
              const int lln = cmp_func(t,t-name,n,ln);
              if (lln && MATCH_RIGHT_CHECK_WORD(lln)) { matched = true; break; }
              if (wc_left > 0)
                MATCH_LEFT_SKIP_TO_WORD();
              else
                t++;
            }
          }
          else
          {
            if (wc_left>0)
            {
              for (;;)
              {
                int v = hasStrings_casecmp(t,n,ln);
                if (!v && MATCH_RIGHT_CHECK_WORD(ln)) { matched = true; break; }
                if (v<0) break;
                MATCH_LEFT_SKIP_TO_WORD();
              }
            }
            else
            {
              for (;;)
              {
                int v = hasStrings_casecmp(t,n,ln);
                if (!v && MATCH_RIGHT_CHECK_WORD(ln)) { matched = true; break; }
                if (v<0) break;
                t++;
              }
            }
          }
#undef MATCH_RIGHT_CHECK_WORD
#undef MATCH_LEFT_SKIP_TO_WORD
          if (matched) break;
        }

        matched_local = (matched?1:0) ^ (TOP_OF_STACK&1);
        TOP_OF_STACK=0;
      }
    }
  }
  while (stacktop > 0) 
  {
    if (POP_STACK() & 0x10) matched_local=TOP_OF_STACK&2;
    else matched_local = (matched_local > 0 ? 1 : 0) ^ (TOP_OF_STACK&1);
  }

  return matched_local!=0;
#undef TOP_OF_STACK
#undef POP_STACK
#undef PUSH_STACK
}

WDL_HASSTRINGS_EXPORT bool WDL_hasStringsEx(const char *name, const LineParser *lp,
     int (*cmp_func)(const char *a, int apos, const char *b, int blen)  // if set, returns length of matched string (0 if no match)
    )
{
  return WDL_hasStringsEx2(&name,1,lp,cmp_func);
}

WDL_HASSTRINGS_EXPORT bool WDL_hasStrings(const char *name, const LineParser *lp)
{
  return WDL_hasStringsEx(name,lp,NULL);
}

WDL_HASSTRINGS_EXPORT bool WDL_makeSearchFilter(const char *flt, LineParser *lp)
{
  if (WDL_NOT_NORMALLY(!lp)) return false;

  if (WDL_NOT_NORMALLY(!flt)) flt="";

#ifdef WDL_LINEPARSER_HAS_LINEPARSERINT
  if (lp->parse_ex(flt,true,false,true)) // allow unterminated quotes
#else
  if (lp->parse_ex(flt,true,false))
#endif
  {
    if (*flt) lp->set_one_token(flt); // failed parsing search string, search as a single token
  }
  for (int x = 0; x < lp->getnumtokens(); x ++)
  {
    char *p = (char *)lp->gettoken_str(x);
    if (lp->gettoken_quotingchar(x) || (strcmp(p,"NOT") && strcmp(p,"AND") && strcmp(p,"OR")))
    {
      while (*p)
      {
        if (*p >= 'A' && *p <= 'Z') *p+='a'-'A';
        p++;
      }
    }
  }

  return lp->getnumtokens()>0;
}

#endif
