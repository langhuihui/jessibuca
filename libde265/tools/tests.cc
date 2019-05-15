/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <iostream>
#include <string.h>


class Test
{
public:
  Test() { next=s_firstTest; s_firstTest=this; }
  virtual ~Test() { }

  virtual const char* getName() const { return "noname"; }
  virtual const char* getDescription() const { return "no description"; }
  virtual bool work(bool quiet=false) = 0;

  static void runTest(const char* name) {
    Test* t = s_firstTest;
    while (t) {
      if (strcmp(t->getName(), name)==0) {
        t->work();
        break;
      }
      t=t->next;
    }
  }

  static void runAllTests() {
    Test* t = s_firstTest;
    while (t) {
      printf("%s ... ",t->getName());
      fflush(stdout);
      if (t->work(true) == false) {
        printf("*** FAILED ***\n");
      }
      else {
        printf("passed\n");
      }

      t=t->next;
    }
  }

public:
  Test* next;
  static Test* s_firstTest;
};

Test* Test::s_firstTest = NULL;


class ListTests : public Test
{
public:
  const char* getName() const { return "list"; }
  const char* getDescription() const { return "list all available tests"; }
  bool work(bool quiet) {
    if (!quiet) {
      Test* t = s_firstTest;
      while (t) {
        printf("- %s: %s\n",t->getName(), t->getDescription());
        t=t->next;
      }
    }
    return true;
  }
} listtest;



int main(int argc,char** argv)
{
  if (argc>=2) {
    Test::runTest(argv[1]);
  }
  else {
    Test::runAllTests();
  }

  return 0;
}
