/************************************************
*                  Console.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   21.07.2017                  *
*************************************************/

#include <System/Console.h>

namespace Columbus
{

//////////////////////////////////////////////////////////////////////////////
  void C_Info(const char* fmt, ...)
  {
    va_list arglist;
    va_start(arglist, fmt);
    register int i;

    bool win = false;
    if (strcmp(C_GetSystem(), "Win32") == 0 || strcmp(C_GetSystem(), "Win64") == 0)
      win = true;

    if (!win)
      printf("\x1b[37;1m[Info]: \x1b[0m");
    else
      printf("[Info]: ");

    for(int i = 0; fmt[i]; i++)
    {
      if(fmt[i] == '%')
      {
        i++;
        switch(fmt[i])
        {
          case 'i':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'd':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'c':
          {
            char c = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", c);
            else
              printf("%c", c);
            break;
          }

          case 'p':
          {
            void* p = va_arg(arglist, void*);
            if (!win)
              printf("\x1b[37;1m%p\x1b[0m", p);
            else
              printf("%p", p);
            break;
          }

          case 'f':
          {
            float f = va_arg(arglist, double);
            if (!win)
              printf("\x1b[37;1m%f\x1b[0m", f);
            else
              printf("%f", f);
            break;
          }

          case 's':
          {
            char* s = va_arg(arglist, char*);
            if (!win)
              printf("\x1b[37;1m%s\x1b[0m", s);
            else
              printf("%s", s);
            break;
          }

          default:
          {
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
            else
              printf("%c", fmt[i]);
            break;
          }
        }
      } else
      {
        if (!win)
          printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
        else
          printf("%c", fmt[i]);
      }
    }
    printf("\n");
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Warning(const char* fmt, ...)
  {
    va_list arglist;
    va_start(arglist, fmt);
    register int i;

    bool win = false;
    if (strcmp(C_GetSystem(), "Win32") == 0 || strcmp(C_GetSystem(), "Win64") == 0)
      win = true;

    if (!win)
      printf("\x1b[33;1m[Warning]: \x1b[0m");
    else
      printf("[Warning]: ");

    for(int i = 0; fmt[i]; i++)
    {
      if(fmt[i] == '%')
      {
        i++;
        switch(fmt[i])
        {
          case 'i':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'd':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'c':
          {
            char c = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", c);
            else
              printf("%c", c);
            break;
          }

          case 'p':
          {
            void* p = va_arg(arglist, void*);
            if (!win)
              printf("\x1b[37;1m%p\x1b[0m", p);
            else
              printf("%p", p);
            break;
          }

          case 'f':
          {
            float f = va_arg(arglist, double);
            if (!win)
              printf("\x1b[37;1m%f\x1b[0m", f);
            else
              printf("%f", f);
            break;
          }

          case 's':
          {
            char* s = va_arg(arglist, char*);
            if (!win)
              printf("\x1b[37;1m%s\x1b[0m", s);
            else
              printf("%s", s);
            break;
          }

          default:
          {
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
            else
              printf("%c", fmt[i]);
            break;
          }
        }
      } else
      {
        if (!win)
          printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
        else
          printf("%c", fmt[i]);
      }
    }
    printf("\n");
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Error(const char* fmt, ...)
  {
    va_list arglist;
    va_start(arglist, fmt);
    register int i;

    bool win = false;
    if (strcmp(C_GetSystem(), "Win32") == 0 || strcmp(C_GetSystem(), "Win64") == 0)
      win = true;

    if (!win)
      printf("\x1b[31;1m[Error]: \x1b[0m");
    else
      printf("[Error]: ");

    for(int i = 0; fmt[i]; i++)
    {
      if(fmt[i] == '%')
      {
        i++;
        switch(fmt[i])
        {
          case 'i':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'd':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'c':
          {
            char c = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", c);
            else
              printf("%c", c);
            break;
          }

          case 'p':
          {
            void* p = va_arg(arglist, void*);
            if (!win)
              printf("\x1b[37;1m%p\x1b[0m", p);
            else
              printf("%p", p);
            break;
          }

          case 'f':
          {
            float f = va_arg(arglist, double);
            if (!win)
              printf("\x1b[37;1m%f\x1b[0m", f);
            else
              printf("%f", f);
            break;
          }

          case 's':
          {
            char* s = va_arg(arglist, char*);
            if (!win)
              printf("\x1b[37;1m%s\x1b[0m", s);
            else
              printf("%s", s);
            break;
          }

          default:
          {
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
            else
              printf("%c", fmt[i]);
            break;
          }
        }
      } else
      {
        if (!win)
          printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
        else
          printf("%c", fmt[i]);
      }
    }
    printf("\n");
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_FatalError(const char* fmt, ...)
  {
    va_list arglist;
    va_start(arglist, fmt);
    register int i;

    bool win = false;
    if (strcmp(C_GetSystem(), "Win32") == 0 || strcmp(C_GetSystem(), "Win64") == 0)
      win = true;

    if (!win)
      printf("\x1b[35;1m[Fatal Error]: \x1b[0m");
    else
      printf("[Fatal Error]: ");

    for(int i = 0; fmt[i]; i++)
    {
      if(fmt[i] == '%')
      {
        i++;
        switch(fmt[i])
        {
          case 'i':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'd':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'c':
          {
            char c = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", c);
            else
              printf("%c", c);
            break;
          }

          case 'p':
          {
            void* p = va_arg(arglist, void*);
            if (!win)
              printf("\x1b[37;1m%p\x1b[0m", p);
            else
              printf("%p", p);
            break;
          }

          case 'f':
          {
            float f = va_arg(arglist, double);
            if (!win)
              printf("\x1b[37;1m%f\x1b[0m", f);
            else
              printf("%f", f);
            break;
          }

          case 's':
          {
            char* s = va_arg(arglist, char*);
            if (!win)
              printf("\x1b[37;1m%s\x1b[0m", s);
            else
              printf("%s", s);
            break;
          }

          default:
          {
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
            else
              printf("%c", fmt[i]);
            break;
          }
        }
      } else
      {
        if (!win)
          printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
        else
          printf("%c", fmt[i]);
      }
    }
    printf("\n");
    exit(1);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Success(const char* fmt, ...)
  {
    va_list arglist;
    va_start(arglist, fmt);
    register int i;

    bool win = false;
    if (strcmp(C_GetSystem(), "Win32") == 0 || strcmp(C_GetSystem(), "Win64") == 0)
      win = true;

    if (!win)
      printf("\x1b[32;1m[Success]: \x1b[0m");
    else
      printf("[Success]: ");

    for(int i = 0; fmt[i]; i++)
    {
      if(fmt[i] == '%')
      {
        i++;
        switch(fmt[i])
        {
          case 'i':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'd':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'c':
          {
            char c = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", c);
            else
              printf("%c", c);
            break;
          }

          case 'p':
          {
            void* p = va_arg(arglist, void*);
            if (!win)
              printf("\x1b[37;1m%p\x1b[0m", p);
            else
              printf("%p", p);
            break;
          }

          case 'f':
          {
            float f = va_arg(arglist, double);
            if (!win)
              printf("\x1b[37;1m%f\x1b[0m", f);
            else
              printf("%f", f);
            break;
          }

          case 's':
          {
            char* s = va_arg(arglist, char*);
            if (!win)
              printf("\x1b[37;1m%s\x1b[0m", s);
            else
              printf("%s", s);
            break;
          }

          default:
          {
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
            else
              printf("%c", fmt[i]);
            break;
          }
        }
      } else
      {
        if (!win)
          printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
        else
          printf("%c", fmt[i]);
      }
    }
    printf("\n");
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Initialization(const char* fmt, ...)
  {
    va_list arglist;
    va_start(arglist, fmt);
    register int i;

    bool win = false;
    if (strcmp(C_GetSystem(), "Win32") == 0 || strcmp(C_GetSystem(), "Win64") == 0)
      win = true;

    if (!win)
      printf("\x1b[36;1m[Initialization]: \x1b[0m");
    else
      printf("[Initialization]: ");

    for(int i = 0; fmt[i]; i++)
    {
      if(fmt[i] == '%')
      {
        i++;
        switch(fmt[i])
        {
          case 'i':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'd':
          {
            int i = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%i\x1b[0m", i);
            else
              printf("%i", i);
            break;
          }

          case 'c':
          {
            char c = va_arg(arglist, int);
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", c);
            else
              printf("%c", c);
            break;
          }

          case 'p':
          {
            void* p = va_arg(arglist, void*);
            if (!win)
              printf("\x1b[37;1m%p\x1b[0m", p);
            else
              printf("%p", p);
          break;
          }

          case 'f':
          {
            float f = va_arg(arglist, double);
            if (!win)
              printf("\x1b[37;1m%f\x1b[0m", f);
            else
              printf("%f", f);
            break;
          }

          case 's':
          {
            char* s = va_arg(arglist, char*);
            if (!win)
              printf("\x1b[37;1m%s\x1b[0m", s);
            else
              printf("%s", s);
            break;
          }

          default:
          {
            if (!win)
              printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
            else
              printf("%c", fmt[i]);
            break;
          }
        }
      } else
      {
        if (!win)
          printf("\x1b[37;1m%c\x1b[0m", fmt[i]);
        else
          printf("%c", fmt[i]);
      }
    }
    printf("\n");
  }

}
