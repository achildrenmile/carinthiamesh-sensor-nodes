#pragma once
// Winziger Test-Rahmen: jeder Testfall laeuft in einem eigenen Prozess (saubere Globals).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

struct TestCase { const char* name; void (*fn)(); };
inline std::vector<TestCase>& registry() { static std::vector<TestCase> r; return r; }
struct Reg { Reg(const char* n, void (*f)()) { registry().push_back({n, f}); } };
inline int g_failed_checks = 0;

#define TEST(name) static void test_##name(); static Reg reg_##name(#name, test_##name); static void test_##name()
#define CHECK(cond) do { if (!(cond)) { printf("    FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failed_checks++; } } while (0)
#define CHECK_EQ(a, b) do { long long _a = (long long)(a), _b = (long long)(b); if (_a != _b) { printf("    FAIL %s:%d: %s == %s (%lld != %lld)\n", __FILE__, __LINE__, #a, #b, _a, _b); g_failed_checks++; } } while (0)

inline int run_all() {
  int failed = 0;
  for (auto& t : registry()) {
    fflush(stdout);
    pid_t p = fork();
    if (p == 0) { g_failed_checks = 0; t.fn(); fflush(stdout); _exit(g_failed_checks ? 1 : 0); }
    int st = 0; waitpid(p, &st, 0);
    bool ok = WIFEXITED(st) && WEXITSTATUS(st) == 0;
    printf("[%s] %s\n", ok ? " OK " : "FAIL", t.name);
    if (!ok) failed++;
  }
  printf("%d von %zu Tests fehlgeschlagen\n", failed, registry().size());
  return failed ? 1 : 0;
}
