extern "C" void free(void *p);

void f(int *q) { *q; }
void myFunc14(int *p) {
  free(p);
  f(p);
}

void freeFun(int **p) {
  char free_out;
  int *dest;

  if (*p) {
    free_out = 0;
    dest = *p;
  } else {
    free_out = 1;
  }

  if (free_out) {
    free(dest);
  }
}
int *myFunc14_2(int **p) {
  freeFun(p);
  return *p;
}

#define NULL ((void *)0)
void vim_free(void *x) {
  if (x != NULL) {
    free(x);
  }
}

struct MyS {
  char *p;
};

struct MySArr {
  MyS *ptr;
};

void myFunc14_3(MySArr *sArr) {
  MyS *myP;
  int todo = 10;
  for (myP = sArr->ptr; todo > 0; ++myP) {
    if (myP->p != nullptr) {
      vim_free(myP->p);
      todo--;
    }
  }
}

struct PP {
  int *pdata;
};

void freeStu(struct PP *stu) {
  if (stu) {
    free(stu->pdata);
    free(stu);
  }
}

struct PP *getStu();

int setStu(struct PP **ppStu) {
  if (!ppStu)
    return 0;

  *ppStu = getStu();
  return 1;
}

void myFunc14_4() {
  int i;
  struct PP *pStu = getStu();
  for (i = 0; i < 5; i++) {
    if (setStu(&pStu)) {
      freeStu(pStu);
    }
  }
}

void myFunc14_5() {
  int i;
  struct PP *pStu = getStu();
  for (i = 0; i < 5; i++) {
    if ((pStu = getStu()) != NULL)
      freeStu(pStu);
  }
}

extern "C" void free(void *ptr);

void myFree(int *ptr) { free(ptr); }

void myFunc14_6() {
  free(nullptr);
  free(nullptr);
}

void myFunc14_7() {
  myFree(nullptr);
  myFree(nullptr);
}

void alist_add(char *fname) { free(fname); }
void myFunc(char **files, int count) {
  for (int i = 0; i < count; i++) {
    alist_add(files[i]);
  }
}
