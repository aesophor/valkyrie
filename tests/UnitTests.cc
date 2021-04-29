// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Deque.h>
#include <Functional.h>
#include <List.h>
#include <Memory.h>
#include <String.h>
#include <dev/Console.h>

namespace valkyrie::kernel {

class Base {
 public:
  Base() { printf("Base::Base()\n"); }
  virtual ~Base() { printf("Base::~Base()\n"); }

  virtual void func() { printf("Base::func()\n"); }
};

class Derived : public Base {
 public:
  Derived() : Base() { printf("Derived::Derived()\n"); }
  virtual ~Derived() { printf("Derived::~Derived()\n"); }

  virtual void func() override { printf("Derived::func()\n"); }
};


void __run_unit_tests() {
  /*
  {
    List<int> list;
    list.push_back(3);
    list.push_back(5);
    list.push_back(7);
    list.show();
    list.pop_front();
    list.show();
    list.pop_front();
    list.show();
    list.pop_front();
    list.show();
  }

  */

  {
    Deque<UniquePtr<Derived>> q;
    auto d = make_unique<Derived>();
    q.push_back(move(d));

    printf("[");
    for (size_t i = 0; i < q.size(); i++) {
      printf("0x%x ", q.at(i).get());
    }
    printf("]\n");

    d = move(q[0]);
    q.erase(0);

    printf("[");
    for (size_t i = 0; i < q.size(); i++) {
      printf("0x%x ", q.at(i).get());
    }
    printf("]\n");

  }


  Function<void ()> fout;
  {
    String s = "hi";

    Function<void ()> f = [s]() { printf("%s\n", s.c_str()); };
    f();

    fout = move(f);
  }
  fout();


  /*
  {
    printf("O M G\n");
    Deque<Derived> v1;
    Deque<Derived> v2;

    v1.push_back(Derived {});
    v1.push_back(Derived {});
    v1.push_back(Derived {});

    printf("v1 = [");
    for (int i = 0; i < (int) v1.size(); i++) {
      printf("%d ", v1[i]);
    }
    printf("]\n");

    v2 = move(v1);

    printf("v1 = [");
    for (int i = 0; i < (int) v1.size(); i++) {
      printf("%d ", v1[i]);
    }
    printf("]\n");

    printf("v2 = [");
    for (int i = 0; i < (int) v2.size(); i++) {
      printf("%d ", v2[i]);
    }
    printf("]\n");
  }


  {
    String s1 = "fuck";
    String s2 = "wow";
    s1 = s1 + s2 + s1 + s2 + s1 + "omg";
    s2 = s1;
    //s2 = move(s1);
    //String s2 = s1 + "omg";
    //String s2 = move(s1);
    //s2[1] = 'a';
    printf("s1 (0x%x) = %s\n", s1.c_str(), s1.c_str());
    printf("s2 (0x%x) = %s\n", s2.c_str(), s2.c_str());
  }
  */
}

}  // namespace valkyrie::kernel
