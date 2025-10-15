
// void test() {
//   int a = CTL_HW;
//   f(a);
// }

#include "internal.h"
#include "uv.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <kvm.h>
#include <paths.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <uvm/uvm_extern.h>

#include <time.h>
#include <unistd.h>

uint64_t uv_get_total_memory(void) {
#if defined(HW_PHYSMEM64)
  uint64_t info;
  int which[] = {CTL_HW, HW_PHYSMEM64};
#else
  unsigned int info;
  int which[] = {1, HW_PHYSMEM};
#endif
  size_t size = sizeof(info);

  if (sysctl(which, 2, &info, &size, NULL, 0))
    return UV__ERR(errno);

  return (uint64_t)info;
}