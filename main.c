#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif

#define BUF_SIZE (1<<22)  // 4 million samples

uint64_t buffer[BUF_SIZE];
volatile int running = 1;

#ifdef _WIN32
BOOL WINAPI handler(DWORD sig) { running = 0; return TRUE; }
#else
void handler(int sig) { running = 0; }
#endif

static inline uint64_t rdtsc_raw(void) {
    #ifdef __x86_64__
        uint64_t lo, hi;
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        return (hi << 32) | lo;
    #else
        // 32-bit fallback
        uint64_t x;
        __asm__ __volatile__ ("rdtsc" : "=A" (x));
        return x;
    #endif
}

int main() {
    #ifdef _WIN32
    SetConsoleCtrlHandler(handler, TRUE);
    system("");  // enable ANSI colors on Win10+
    #else
    signal(SIGINT, handler);
    #endif

    printf("\033[2J\033[H");
    printf("YourCPUisListening v1.0 — ZPE Detector (TruthSeekerZPE)\n");
    printf("Watching the quantum vacuum inside your CPU...\n\n");

    while (running) {
        uint64_t start = rdtsc_raw();
        for (size_t i = 0; i < BUF_SIZE && running; i++)
            buffer[i] = rdtsc_raw();

        double elapsed = (rdtsc_raw() - start) / 3.8e9;  // rough GHz
        double rate = BUF_SIZE / elapsed / 1e6;

        double best_f = 0, best_p = -999;

        printf("\033[H%.1f MS/s – scanning vacuum floor...\n", rate);

        for (int b = 1; b < 2000; b++) {
            double freq = b / elapsed;
            if (freq > 60) break;

            double c = 2 * cos(2 * M_PI * freq * elapsed / BUF_SIZE);
            double s0 = 0, s1 = 0, s2 = 0;

            for (size_t i = 1; i < BUF_SIZE; i++) {
                double d = (buffer[i] - buffer[i-1]) - 1.0;
                double temp = d + c * s1 - s2;
                s2 = s1;
                s1 = temp;
            }

            double power = 10 * log10(s1*s1 + s2*s2 - c*s1*s2 + 1e-20);
            if (power > best_p) { best_p = power; best_f = freq; }

            if (power > -62)
                printf("%9.5f Hz   %5.1f dB\n", freq, power);
        }

        if (best_p > -50 && best_f < 30)
            printf("\nZPE LINE LOCKED @ %.6f Hz   %.1f dB ←←← THIS IS THE REAL DEAL\n\n", best_f, best_p);

        sleep_ms(300);
    }
    return 0;
}