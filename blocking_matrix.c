/*
 * blocking_matrix.c
 * Prática de Matrix Blocking - Demonstra overhead com diferentes tamanhos de bloco
 *
 * Compile:
 *   gcc -O3 -march=native -o blocking_matrix blocking_matrix.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

#define A(i,j) A[(j)*ldA + (i)]
#define B(i,j) B[(j)*ldB + (i)]
#define C(i,j) C[(j)*ldC + (i)]

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void init_matrix(double *M, int rows, int cols, int ld) {
    for (int j = 0; j < cols; j++)
        for (int i = 0; i < rows; i++)
            M[j*ld + i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
}

void zero_matrix(double *M, int rows, int cols, int ld) {
    for (int j = 0; j < cols; j++)
        for (int i = 0; i < rows; i++)
            M[j*ld + i] = 0.0;
}

// Versão simples (não bloqueada)
void Gemm_simple(int m, int n, int k, double *A, int ldA,
                 double *B, int ldB, double *C, int ldC)
{
    for (int p = 0; p < k; p++)
        for (int j = 0; j < n; j++)
            for (int i = 0; i < m; i++)
                C(i,j) += A(i,p) * B(p,j);
}

// Versão bloqueada
void MyGemm_blocked(int m, int n, int k, double *A, int ldA,
                    double *B, int ldB, double *C, int ldC,
                    int MB, int NB, int KB)
{
    for (int j = 0; j < n; j += NB) {
        int jb = min(n - j, NB);
        for (int i = 0; i < m; i += MB) {
            int ib = min(m - i, MB);
            for (int p = 0; p < k; p += KB) {
                int pb = min(k - p, KB);

                for (int pp = 0; pp < pb; pp++)
                    for (int jj = 0; jj < jb; jj++)
                        for (int ii = 0; ii < ib; ii++)
                            C(i+ii, j+jj) += A(i+ii, p+pp) * B(p+pp, j+jj);
            }
        }
    }
}

void run_test(const char *name, int m, int n, int k,
              double *A, int ldA, double *B, int ldB, double *C, int ldC,
              int MB, int NB, int KB, int reps)
{
    double total = 0.0;
    for (int r = 0; r < reps; r++) {
        zero_matrix(C, m, n, ldC);
        double t0 = get_time();

        if (MB == 0)
            Gemm_simple(m, n, k, A, ldA, B, ldB, C, ldC);
        else
            MyGemm_blocked(m, n, k, A, ldA, B, ldB, C, ldC, MB, NB, KB);

        total += get_time() - t0;
    }
    double avg = total / reps;
    double gflops = (2.0 * m * n * k) / (avg * 1e9);
    printf("%-28s | %4d | %4d | %4d | %10.6f | %8.2f\n",
           name, MB, NB, KB, avg, gflops);
}

int main() {
    srand(42);
    int m = 512, n = 512, k = 512;
    int ldA = m, ldB = k, ldC = m;

    printf("=== Prática de Matrix Blocking ===\n");
    printf("Matriz: %d x %d x %d\n\n", m, n, k);

    double *A = malloc(ldA * k * sizeof(double));
    double *B = malloc(ldB * n * sizeof(double));
    double *C = malloc(ldC * n * sizeof(double));

    if (!A || !B || !C) {
        printf("Erro de memória!\n");
        return 1;
    }

    init_matrix(A, m, k, ldA);
    init_matrix(B, k, n, ldB);

    int reps = 2;

    printf("%-28s | %4s | %4s | %4s | %10s | %8s\n",
           "Versão", "MB", "NB", "KB", "Tempo(s)", "GFLOPS");
    printf("------------------------------------------------------------------------\n");

    run_test("Nao bloqueada (PJI)", m, n, k, A, ldA, B, ldB, C, ldC, 0,0,0, reps);

    printf("------------------------------------------------------------------------\n");

    // Blocos muito pequenos (ruim)
    run_test("Bloqueado (MUITO PEQUENO)", m, n, k, A, ldA, B, ldB, C, ldC, 8,8,8, reps);
    run_test("Bloqueado (MUITO PEQUENO)", m, n, k, A, ldA, B, ldB, C, ldC, 16,16,16, reps);

    printf("------------------------------------------------------------------------\n");

    // Blocos bons
    run_test("Bloqueado (BOM)", m, n, k, A, ldA, B, ldB, C, ldC, 32,32,32, reps);
    run_test("Bloqueado (BOM)", m, n, k, A, ldA, B, ldB, C, ldC, 64,64,64, reps);
    run_test("Bloqueado (BOM)", m, n, k, A, ldA, B, ldB, C, ldC, 96,96,96, reps);

    printf("------------------------------------------------------------------------\n");

    // Bloco muito grande
    run_test("Bloqueado (MUITO GRANDE)", m, n, k, A, ldA, B, ldB, C, ldC, 256,256,256, reps);

    free(A);
    free(B);
    free(C);

    printf("\nDica: Tamanhos médios (32~96) costumam ser os melhores.\n");
    return 0;
}